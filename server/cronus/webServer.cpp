#include "cronus/webServer.h"
#include "cronus/logger.h"
#include "cronus/sourceMap.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace fs=std::filesystem;
using json=nlohmann::json;
using cronus::DirectoryEntry;

namespace cronus
{

// Helper function to check if a string starts with a prefix (replacement for C++20 starts_with)
bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}

// Helper function to get current timestamp
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%S");
    return ss.str();
}

// Helper function to convert directory structure to JSON
json buildFileTreeJson(const std::unordered_map<std::string, DirectoryEntry>& dirStructure) {
    json result = json::array();
    
    // Find root entries (those with no parent)
    for (const auto& [path, entry] : dirStructure) {
        // Check if this is a root-level entry (path has no parent in structure)
        bool isRoot = true;
        for (const auto& [otherPath, otherEntry] : dirStructure) {
            if (otherEntry.isDirectory) {
                for (const auto& childPath : otherEntry.children) {
                    if (childPath == path) {
                        isRoot = false;
                        break;
                    }
                }
            }
            if (!isRoot) break;
        }
        
        if (isRoot) {
            json node = {
                {"name", entry.name},
                {"path", entry.path},
                {"isDirectory", entry.isDirectory}
            };
            
            if (entry.isDirectory && !entry.children.empty()) {
                json children = json::array();
                for (const auto& childPath : entry.children) {
                    auto it = dirStructure.find(childPath);
                    if (it != dirStructure.end()) {
                        children.push_back({
                            {"name", it->second.name},
                            {"path", it->second.path},
                            {"isDirectory", it->second.isDirectory}
                        });
                    }
                }
                node["children"] = children;
            }
            
            result.push_back(node);
        }
    }
    
    return result;
}

WebServer::WebServer(Cronus &cronus, int port)
    : m_cronus(cronus),
    m_port(port)
{
    // Determine the path to the frontend files
    fs::path executablePath=fs::canonical("/proc/self/exe").parent_path();

    // Try different possible locations for the frontend build
    std::vector<fs::path> possiblePaths={
        executablePath/"web/build",
        executablePath/"../web/build",
        executablePath/"../../web/build",
        executablePath/"../../../clients/web/build",
        fs::path("/home/caseymcc/projects/cronus/clients/web/build"),
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
        Logger::instance().error("Frontend path not found. Web UI may not work correctly.");
        // Fallback to the executable directory
        m_frontendPath=executablePath.string()+"/web/build";
        fs::create_directories(m_frontendPath);
    }

    Logger::instance().info("Using frontend path: "+m_frontendPath);

    // Register JSON-RPC methods
    registerJsonRpcMethod("input", [this](const json& params) { return handleInput(params); });
    registerJsonRpcMethod("getSourceMap", [this](const json& params) { return handleGetSourceMap(params); });
    registerJsonRpcMethod("getFile", [this](const json& params) { return handleGetFile(params); });
    registerJsonRpcMethod("getDirectory", [this](const json& params) { return handleGetDirectory(params); });
    registerJsonRpcMethod("getLogs", [this](const json& params) { return handleGetLogs(params); });

    // Setup routes once in constructor
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
            Logger::instance().info("Starting web server on port "+std::to_string(m_port));
            m_app.port(m_port).multithreaded().run();
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
    m_app.stop();

    // Wait for the server thread to finish
    if(m_serverThread.joinable())
    {
        m_serverThread.join();
    }

    Logger::instance().info("Web server stopped");
}

void WebServer::registerJsonRpcMethod(const std::string& method, JsonRpcHandler handler)
{
    m_jsonRpcMethods[method] = handler;
}

json WebServer::handleJsonRpcRequest(const std::string& message)
{
    try {
        json request = json::parse(message);
        
        // Validate JSON-RPC request
        if (!request.contains("jsonrpc") || request["jsonrpc"] != "2.0") {
            return {
                {"jsonrpc", "2.0"},
                {"error", {
                    {"code", -32600},
                    {"message", "Invalid Request: missing or invalid jsonrpc field"}
                }},
                {"id", nullptr}
            };
        }
        
        if (!request.contains("method")) {
            return {
                {"jsonrpc", "2.0"},
                {"error", {
                    {"code", -32600},
                    {"message", "Invalid Request: missing method field"}
                }},
                {"id", request.value("id", nullptr)}
            };
        }
        
        std::string method = request["method"];
        json params = request.value("params", json::object());
        auto id = request.value("id", nullptr);
        
        // Find and execute the method handler
        auto it = m_jsonRpcMethods.find(method);
        if (it == m_jsonRpcMethods.end()) {
            return {
                {"jsonrpc", "2.0"},
                {"error", {
                    {"code", -32601},
                    {"message", "Method not found: " + method}
                }},
                {"id", id}
            };
        }
        
        // Call the handler
        try {
            json result = it->second(params);
            return {
                {"jsonrpc", "2.0"},
                {"result", result},
                {"id", id}
            };
        } catch (const std::exception& e) {
            return {
                {"jsonrpc", "2.0"},
                {"error", {
                    {"code", -32603},
                    {"message", std::string("Internal error: ") + e.what()}
                }},
                {"id", id}
            };
        }
        
    } catch (const json::parse_error& e) {
        return {
            {"jsonrpc", "2.0"},
            {"error", {
                {"code", -32700},
                {"message", std::string("Parse error: ") + e.what()}
            }},
            {"id", nullptr}
        };
    }
}

void WebServer::broadcastNotification(const std::string& method, const json& params)
{
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params}
    };
    
    std::string message = notification.dump();
    
    std::lock_guard<std::mutex> lock(m_wsMutex);
    for (auto* conn : m_wsConnections) {
        conn->send_text(message);
    }
}

void WebServer::broadcastMessage(const std::string& role, const std::string& content)
{
    json params = {
        {"role", role},
        {"content", content},
        {"timestamp", getCurrentTimestamp()}
    };
    broadcastNotification("message", params);
}

void WebServer::broadcastLog(const std::string& level, const std::string& message)
{
    // Add to log history
    {
        std::lock_guard<std::mutex> lock(m_logsMutex);
        m_logHistory.push_back({getCurrentTimestamp(), level, message});
        if (m_logHistory.size() > m_maxLogHistory) {
            m_logHistory.erase(m_logHistory.begin());
        }
    }
    
    // Broadcast to clients
    json params = {
        {"level", level},
        {"message", message},
        {"timestamp", getCurrentTimestamp()}
    };
    broadcastNotification("log", params);
}

void WebServer::broadcastDirectoryUpdate()
{
    auto sourceMap = m_cronus.getSourceMap();
    if (sourceMap) {
        json fileTree = buildFileTreeJson(sourceMap->getDirectoryStructure());
        json params = {
            {"fileTree", fileTree}
        };
        broadcastNotification("directoryUpdate", params);
    }
}

// JSON-RPC Method Implementations

json WebServer::handleInput(const json& params)
{
    if (!params.contains("text")) {
        throw std::runtime_error("Missing 'text' parameter");
    }
    
    std::string input = params["text"];
    m_cronus.handleInput(input);
    
    return {
        {"success", true},
        {"message", "Input received and processing"}
    };
}

json WebServer::handleGetSourceMap(const json& params)
{
    auto sourceMap = m_cronus.getSourceMap();
    if (sourceMap) {
        return {
            {"fileTree", buildFileTreeJson(sourceMap->getDirectoryStructure())}
        };
    }
    
    return {
        {"fileTree", nullptr}
    };
}

json WebServer::handleGetFile(const json& params)
{
    if (!params.contains("path")) {
        throw std::runtime_error("Missing 'path' parameter");
    }
    
    std::string path = params["path"];
    
    // Read file content
    if (!fs::exists(path)) {
        throw std::runtime_error("File not found: " + path);
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    // Determine language from extension
    std::string ext = fs::path(path).extension().string();
    std::string language = "text";
    if (ext == ".cpp" || ext == ".cc" || ext == ".cxx") language = "cpp";
    else if (ext == ".h" || ext == ".hpp") language = "cpp";
    else if (ext == ".py") language = "python";
    else if (ext == ".js") language = "javascript";
    else if (ext == ".ts") language = "typescript";
    else if (ext == ".json") language = "json";
    else if (ext == ".md") language = "markdown";
    
    return {
        {"path", path},
        {"content", buffer.str()},
        {"language", language}
    };
}

json WebServer::handleGetDirectory(const json& params)
{
    if (!params.contains("path")) {
        throw std::runtime_error("Missing 'path' parameter");
    }
    
    std::string path = params["path"];
    
    if (!fs::exists(path) || !fs::is_directory(path)) {
        throw std::runtime_error("Directory not found: " + path);
    }
    
    json entries = json::array();
    for (const auto& entry : fs::directory_iterator(path)) {
        entries.push_back({
            {"name", entry.path().filename().string()},
            {"type", entry.is_directory() ? "directory" : "file"}
        });
    }
    
    return {
        {"path", path},
        {"entries", entries}
    };
}

json WebServer::handleGetLogs(const json& params)
{
    int limit = params.value("limit", 100);
    
    std::lock_guard<std::mutex> lock(m_logsMutex);
    
    json logs = json::array();
    size_t start = m_logHistory.size() > (size_t)limit ? m_logHistory.size() - limit : 0;
    
    for (size_t i = start; i < m_logHistory.size(); i++) {
        logs.push_back({
            {"timestamp", m_logHistory[i].timestamp},
            {"level", m_logHistory[i].level},
            {"message", m_logHistory[i].message}
        });
    }
    
    return {
        {"logs", logs}
    };
}

void WebServer::setupRoutes()
{
    // WebSocket endpoint for JSON-RPC communication
    CROW_WEBSOCKET_ROUTE(m_app, "/ws")
        .onopen([this](crow::websocket::connection& conn) {
            std::lock_guard<std::mutex> lock(m_wsMutex);
            m_wsConnections.insert(&conn);
            
            Logger::instance().info("WebSocket client connected");
            
            // Send initial directory update to new client
            auto sourceMap = m_cronus.getSourceMap();
            if (sourceMap) {
                json notification = {
                    {"jsonrpc", "2.0"},
                    {"method", "directoryUpdate"},
                    {"params", {
                        {"tree", buildFileTreeJson(sourceMap->getDirectoryStructure())}
                    }}
                };
                conn.send_text(notification.dump());
            }
        })
        .onclose([this](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
            std::lock_guard<std::mutex> lock(m_wsMutex);
            m_wsConnections.erase(&conn);
            Logger::instance().info("WebSocket client disconnected: " + reason + " (code: " + std::to_string(code) + ")");
        })
        .onmessage([this](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
            if (is_binary) {
                json error = {
                    {"jsonrpc", "2.0"},
                    {"error", {
                        {"code", -32600},
                        {"message", "Binary messages not supported"}
                    }},
                    {"id", nullptr}
                };
                conn.send_text(error.dump());
                return;
            }
            
            json response = handleJsonRpcRequest(data);
            conn.send_text(response.dump());
        });

    // Serve the React app index.html for the root path
    CROW_ROUTE(m_app, "/")
    ([this]() {
        std::string indexPath = m_frontendPath + "/index.html";
        if (fs::exists(indexPath)) {
            std::ifstream file(indexPath, std::ios::binary);
            std::stringstream buffer;
            buffer << file.rdbuf();
            
            auto response = crow::response(buffer.str());
            response.set_header("Content-Type", "text/html");
            response.set_header("Access-Control-Allow-Origin", "*");
            return response;
        } else {
            // If frontend is not built yet, serve a basic HTML page
            std::string basicHtml = R"(
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
                cd clients/web
                npm install
                npm run build
                        </pre>
                    </div>
                    <div class="card">
                        <h2>WebSocket API is available</h2>
                        <p>The WebSocket API is available at:</p>
                        <ul>
                            <li>ws://localhost:)" + std::to_string(m_port) + R"(/ws - WebSocket endpoint</li>
                            <li><a href="/api/info">/api/info</a> - API information</li>
                            <li><a href="/api/health">/api/health</a> - Health check</li>
                        </ul>
                    </div>
                </body>
                </html>
            )";
            
            auto response = crow::response(basicHtml);
            response.set_header("Content-Type", "text/html");
            response.set_header("Access-Control-Allow-Origin", "*");
            return response;
        }
    });

    // Serve static files from the frontend build directory
    CROW_ROUTE(m_app, "/static/<path>")
    ([this](const std::string& path) {
        std::string fullPath = m_frontendPath + "/static/" + path;
        if (fs::exists(fullPath)) {
            std::ifstream file(fullPath, std::ios::binary);
            std::stringstream buffer;
            buffer << file.rdbuf();
            
            auto response = crow::response(buffer.str());
            response.set_header("Content-Type", getContentType(fullPath));
            response.set_header("Access-Control-Allow-Origin", "*");
            return response;
        } else {
            auto response = crow::response(404);
            response.set_header("Access-Control-Allow-Origin", "*");
            return response;
        }
    });

    // API info endpoint
    CROW_ROUTE(m_app, "/api/info")
    ([this]() {
        json info = {
            {"name", "Cronus Web UI"},
            {"version", "1.0.0"},
            {"websocket", "ws://localhost:" + std::to_string(m_port) + "/ws"},
            {"protocol", "JSON-RPC 2.0"}
        };
        
        auto response = crow::response(info.dump());
        response.set_header("Content-Type", "application/json");
        response.set_header("Access-Control-Allow-Origin", "*");
        return response;
    });

    // Health check endpoint
    CROW_ROUTE(m_app, "/api/health")
    ([]() {
        json healthInfo = {
            {"status", "ok"},
            {"message", "Cronus Web Server is running"}
        };
        
        auto response = crow::response(healthInfo.dump());
        response.set_header("Content-Type", "application/json");
        response.set_header("Access-Control-Allow-Origin", "*");
        return response;
    });

    // Catch-all route for SPA routing - serve index.html
    CROW_ROUTE(m_app, "/<path>")
    ([this](const std::string& path) {
        // Don't catch API routes or WebSocket
        if (path.substr(0, 4) == "api/" || path == "ws") {
            auto response = crow::response(404);
            response.set_header("Access-Control-Allow-Origin", "*");
            return response;
        }
        
        std::string fullPath = m_frontendPath + "/" + path;
        
        if (fs::exists(fullPath)) {
            std::ifstream file(fullPath, std::ios::binary);
            std::stringstream buffer;
            buffer << file.rdbuf();
            
            auto response = crow::response(buffer.str());
            response.set_header("Content-Type", getContentType(fullPath));
            response.set_header("Access-Control-Allow-Origin", "*");
            return response;
        } else {
            // For SPA routing, serve index.html for non-existent paths
            std::string indexPath = m_frontendPath + "/index.html";
            if (fs::exists(indexPath)) {
                std::ifstream file(indexPath, std::ios::binary);
                std::stringstream buffer;
                buffer << file.rdbuf();
                
                auto response = crow::response(buffer.str());
                response.set_header("Content-Type", "text/html");
                response.set_header("Access-Control-Allow-Origin", "*");
                return response;
            } else {
                auto response = crow::response(404);
                response.set_header("Access-Control-Allow-Origin", "*");
                return response;
            }
        }
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