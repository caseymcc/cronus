#include "cronus/webServer.h"

#include "cronus/logger.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs=std::filesystem;
using json=nlohmann::json;

namespace cronus
{

// Helper function to check if a string starts with a prefix (replacement for C++20 starts_with)
bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}

WebServer::WebServer(Cronus &cronus, int port)
    : m_cronus(cronus),
    m_server(std::make_unique<httplib::Server>()),
    m_port(port)
{
    // Determine the path to the frontend files
    fs::path executablePath=fs::canonical("/proc/self/exe").parent_path();

    // Try different possible locations for the frontend build
    std::vector<fs::path> possiblePaths={
        executablePath/"web/build",
        executablePath/"../web/build",
        executablePath/"../../web/build",
        fs::path("/home/caseymcc/projects/cronus/web/build"),
    };

    for(const auto &path:possiblePaths)
    {
        if(fs::exists(path)&&fs::exists(path/"index.html"))
        {
            m_frontendPath=path.string();
            break;
        }
    }

    if(m_frontendPath.empty())
    {
        logError("Frontend path not found. Web UI may not work correctly.");
        // Fallback to the executable directory
        m_frontendPath=executablePath.string()+"/web/build";
        fs::create_directories(m_frontendPath);
    }

    logInfo("Using frontend path: "+m_frontendPath);

    setupRoutes();
}

WebServer::~WebServer()
{
    stop();
}

bool WebServer::start()
{
    if(m_running)
    {
        return true;
    }

    m_running=true;

    // Start the server in a separate thread
    m_serverThread=std::thread([this]()
        {
            logInfo("Starting web server on port "+std::to_string(m_port));
            m_server->listen("0.0.0.0", m_port);
        });

    return true;
}

void WebServer::stop()
{
    if(!m_running)
    {
        return;
    }

    m_running=false;

    // Stop the server
    m_server->stop();

    // Wait for the server thread to finish
    if(m_serverThread.joinable())
    {
        m_serverThread.join();
    }

    logInfo("Web server stopped");
}

void WebServer::setupRoutes()
{
    // Enable CORS for all routes
    m_server->set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
        });

    m_server->Options("/(.*)", [](const httplib::Request &, httplib::Response &res)
        {
            res.status=204; // No content for OPTIONS requests
        });

    // Serve the React app index.html for the root path
    m_server->Get("/", [this](const httplib::Request &, httplib::Response &res)
        {
            std::string indexPath=m_frontendPath+"/index.html";
            if(fs::exists(indexPath))
            {
                std::ifstream file(indexPath, std::ios::binary);
                std::stringstream buffer;
                buffer<<file.rdbuf();
                res.set_content(buffer.str(), "text/html");
            }
            else
            {
                // If frontend is not built yet, serve a basic HTML that redirects to API docs
                std::string basicHtml=R"(
                <!DOCTYPE html>
                <html>
                <head>
                    <title>Cronus Web UI</title>
                    <style>
                        body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; }
                        .card { border: 1px solid #ddd; border-radius: 8px; padding: 20px; margin: 20px 0; }
                    </style>
                </head>
                <body>
                    <h1>Cronus Web UI</h1>
                    <div class="card">
                        <h2>Frontend not built</h2>
                        <p>The React frontend has not been built yet. Please build the frontend and restart the server.</p>
                        <p>Build instructions:</p>
                        <pre>
                cd frontend
                npm install
                npm run build
                        </pre>
                    </div>
                    <div class="card">
                        <h2>API is available</h2>
                        <p>The REST API is available at:</p>
                        <ul>
                            <li><a href="/api/info">/api/info</a> - Get API information</li>
                        </ul>
                    </div>
                </body>
                </html>
            )";
                res.set_content(basicHtml, "text/html");
            }
        });

    // Serve static files from the frontend build directory
    m_server->Get("/static/(.*)", [this](const httplib::Request &req, httplib::Response &res)
        {
            std::string path=m_frontendPath+"/static/"+req.matches[1].str();
            if(fs::exists(path))
            {
                std::ifstream file(path, std::ios::binary);
                std::stringstream buffer;
                buffer<<file.rdbuf();
                res.set_content(buffer.str(), getContentType(path));
            }
            else
            {
                res.status=404;
            }
        });

    // Serve asset-manifest.json, robots.txt, favicon.ico, etc.
    m_server->Get("/(.*)", [this](const httplib::Request &req, httplib::Response &res)
        {
            std::string requestPath=req.matches[1].str();
            if(requestPath.empty())
            {
                requestPath="index.html";
            }

            std::string fullPath=m_frontendPath+"/"+requestPath;

            if(fs::exists(fullPath))
            {
                std::ifstream file(fullPath, std::ios::binary);
                std::stringstream buffer;
                buffer<<file.rdbuf();
                res.set_content(buffer.str(), getContentType(fullPath));
            }
            else
            {
                // For SPA routing, serve index.html for all non-file routes
                // unless it's an API route
                if(!startsWith(requestPath, "api/"))
                {
                    std::string indexPath=m_frontendPath+"/index.html";
                    if(fs::exists(indexPath))
                    {
                        std::ifstream file(indexPath, std::ios::binary);
                        std::stringstream buffer;
                        buffer<<file.rdbuf();
                        res.set_content(buffer.str(), "text/html");
                    }
                    else
                    {
                        res.status=404;
                    }
                }
                else
                {
                    res.status=404;
                }
            }
        });

    // Add info endpoint
    m_server->Get("/api/info", [this](const httplib::Request &, httplib::Response &res)
        {
            json info={
                {"name", "Cronus Web UI"},
                {"version", "1.0.0"},
                {"apiBaseUrl", getBaseUrl()}
            };
            res.set_content(info.dump(), "application/json");
        });
}

std::string WebServer::getContentType(const std::string &path)
{
    std::string ext=fs::path(path).extension().string();

    if(ext==".html"||ext==".htm") return "text/html";
    if(ext==".css") return "text/css";
    if(ext==".js") return "application/javascript";
    if(ext==".json") return "application/json";
    if(ext==".png") return "image/png";
    if(ext==".jpg"||ext==".jpeg") return "image/jpeg";
    if(ext==".svg") return "image/svg+xml";
    if(ext==".ico") return "image/x-icon";
    if(ext==".txt") return "text/plain";
    if(ext==".woff") return "font/woff";
    if(ext==".woff2") return "font/woff2";

    return "application/octet-stream";
}

} // namespace cronus