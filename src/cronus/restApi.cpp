#include "cronus/rest_api.h"
#include "cronus/logger.h"

#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace cronus
{

RestApi::RestApi(Cronus& cronus, int port) :
    m_cronus(cronus),
    m_port(port),
    m_server(std::make_unique<httplib::Server>())
{
}

RestApi::~RestApi()
{
    stop();
}

void RestApi::start()
{
    if (m_running) {
        return;
    }
    
    setupRoutes();
    
    m_running = true;
    m_serverThread = std::thread([this]() {
        logInfo("Starting REST API server on port " + std::to_string(m_port));
        m_server->listen("localhost", m_port);
    });
}

void RestApi::stop()
{
    if (!m_running) {
        return;
    }
    
    m_running = false;
    m_server->stop();
    
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }
    
    logInfo("REST API server stopped");
}

void RestApi::setupRoutes()
{
    // Process input endpoint
    m_server->Post("/api/input", [this](const httplib::Request& req, httplib::Response& res) {
        handleProcessInput(req, res);
    });
    
    // Get directory contents endpoint
    m_server->Get("/api/directory", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetDirectoryContents(req, res);
    });
    
    // Health check endpoint
    m_server->Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "ok"},
            {"message", "Cronus API is running"}
        };
        res.set_content(response.dump(), "application/json");
    });
}

void RestApi::handleProcessInput(const httplib::Request& req, httplib::Response& res)
{
    try {
        auto requestJson = json::parse(req.body);
        
        if (!requestJson.contains("input")) {
            res.status = 400;
            json errorResponse = {
                {"error", "Missing 'input' field in request"}
            };
            res.set_content(errorResponse.dump(), "application/json");
            return;
        }
        
        std::string input = requestJson["input"];
        logInfo("API received input: " + input);
        
        // Process the input asynchronously
        auto future = m_cronus.processInput(input);
        
        // Return immediately with a success response
        json response = {
            {"status", "processing"},
            {"message", "Input is being processed"}
        };
        res.set_content(response.dump(), "application/json");
        
    } catch (const std::exception& e) {
        res.status = 500;
        json errorResponse = {
            {"error", std::string("Error processing input: ") + e.what()}
        };
        res.set_content(errorResponse.dump(), "application/json");
    }
}

void RestApi::handleGetDirectoryContents(const httplib::Request&, httplib::Response& res)
{
    try {
        auto contents = m_cronus.getCurrentDirectoryContents();
        
        json dirContents = json::array();
        for (const auto& [isDir, name] : contents) {
            dirContents.push_back({
                {"isDirectory", isDir},
                {"name", name}
            });
        }
        
        json response = {
            {"contents", dirContents}
        };
        
        res.set_content(response.dump(), "application/json");
        
    } catch (const std::exception& e) {
        res.status = 500;
        json errorResponse = {
            {"error", std::string("Error getting directory contents: ") + e.what()}
        };
        res.set_content(errorResponse.dump(), "application/json");
    }
}

} // namespace cronus
