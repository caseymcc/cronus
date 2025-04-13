#include "cronus/restApi.h"

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
        m_server->listen("0.0.0.0", m_port);
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
    
    // Close and clean up all WebSocket connections
    {
        std::lock_guard<std::mutex> lock(m_webSocketMutex);
        for (auto* ws : m_webSocketConnections) {
            // Note: httplib manages WebSocket deletion, we just need to clear our set
        }
        m_webSocketConnections.clear();
    }
    
    logInfo("REST API server stopped");
}

void RestApi::setupRoutes()
{
    // Enable CORS for all routes
    m_server->set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
    });
    
    m_server->Options("/(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204; // No content for OPTIONS requests
    });
    
    // Process input endpoint
    m_server->Post("/api/input", [this](const httplib::Request& req, httplib::Response& res) {
        handleProcessInput(req, res);
    });
    
    // Get directory contents endpoint
    m_server->Get("/api/directory", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetDirectoryContents(req, res);
    });
    
    // Get source map data endpoint
    m_server->Get("/api/sourcemap", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetSourceMap(req, res);
    });
    
    // Health check endpoint
    m_server->Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "ok"},
            {"message", "Cronus API is running"}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // Add WebSocket handler
    addWebSocketHandler();
}

void RestApi::addWebSocketHandler() 
{
    // WebSocket endpoint for real-time updates
    m_server->Get("/ws", [this](const httplib::Request& req, httplib::Response& res) {
        handleWebSocketConnection(req, res);
    });
}

void RestApi::handleWebSocketConnection(const httplib::Request& req, httplib::Response& res) 
{
    auto ws = res.upgrade(req);
    if (ws) {
        logInfo("New WebSocket connection established");
        
        // Store the WebSocket connection
        {
            std::lock_guard<std::mutex> lock(m_webSocketMutex);
            m_webSocketConnections.insert(ws.get());
        }
        
        // Send initial directory structure
        json initialMessage = {
            {"type", "directory_init"},
            {"message", "Sending initial directory structure"}
        };
        ws->send(initialMessage.dump());
        
        // Configure WebSocket connection callbacks
        ws->set_close_callback([this, ws=ws.get()](const httplib::WebSocket&, int, const std::string&) {
            logInfo("WebSocket connection closed");
            std::lock_guard<std::mutex> lock(m_webSocketMutex);
            m_webSocketConnections.erase(ws);
        });
        
        ws->set_error_callback([this, ws=ws.get()](const httplib::WebSocket&, httplib::Error) {
            logError("WebSocket error occurred");
            std::lock_guard<std::mutex> lock(m_webSocketMutex);
            m_webSocketConnections.erase(ws);
        });
        
        // Handle incoming messages from client
        ws->set_message_callback([this](const httplib::WebSocket&, const httplib::WebSocket::Message& msg) {
            if (msg.is_text()) {
                try {
                    auto data = json::parse(msg.str());
                    if (data.contains("type") && data["type"] == "request_update") {
                        broadcastDirectoryUpdate();
                    }
                } catch (const std::exception& e) {
                    logError("Failed to parse WebSocket message: " + std::string(e.what()));
                }
            }
        });
    } else {
        logError("WebSocket upgrade failed");
    }
}

void RestApi::broadcastMessage(const std::string& message) 
{
    std::lock_guard<std::mutex> lock(m_webSocketMutex);
    
    // Copy the set to avoid issues if connections are closed during iteration
    auto connections = m_webSocketConnections;
    
    for (auto* ws : connections) {
        // Check if connection is still valid
        if (m_webSocketConnections.find(ws) != m_webSocketConnections.end()) {
            ws->send(message);
        }
    }
}

void RestApi::broadcastDirectoryUpdate() 
{
    try {
        const auto& sourceMap = m_cronus.getSourceMap();
        if (!sourceMap) {
            logError("Cannot broadcast directory update: SourceMap is not initialized");
            return;
        }
        
        // Get directory structure from the SourceMap for the update
        const auto& fileCache = sourceMap->getFileCache();
        
        // Build a hierarchical structure
        json fileTree = {};
        
        for (const auto& [path, fileTags] : fileCache) {
            std::string relativePath = fileTags.m_relativeFileName;
            
            // Skip files outside the working directory
            if (relativePath.empty() || relativePath[0] == '.') {
                continue;
            }
            
            // Convert file path to hierarchical structure
            std::vector<std::string> pathParts;
            std::string currentPart;
            std::istringstream pathStream(relativePath);
            
            // Split path by directory separator
            while (std::getline(pathStream, currentPart, '/')) {
                if (!currentPart.empty()) {
                    pathParts.push_back(currentPart);
                }
            }
            
            // Construct the tree structure
            json* currentLevel = &fileTree;
            for (size_t i = 0; i < pathParts.size(); ++i) {
                const std::string& part = pathParts[i];
                bool isLastPart = (i == pathParts.size() - 1);
                
                // If this part doesn't exist in the current level, add it
                if (!currentLevel->contains(part)) {
                    if (isLastPart) {
                        // It's a file
                        (*currentLevel)[part] = {
                            {"type", "file"},
                            {"path", path},
                            {"tags", json::array()}
                        };
                        
                        // Add file tags if available
                        for (const auto& tag : fileTags.m_tags) {
                            (*currentLevel)[part]["tags"].push_back({
                                {"name", tag.name},
                                {"type", static_cast<int>(tag.type)},
                                {"start", tag.start},
                                {"end", tag.end}
                            });
                        }
                    } else {
                        // It's a directory
                        (*currentLevel)[part] = {
                            {"type", "directory"},
                            {"children", json::object()}
                        };
                    }
                }
                
                // Move to the next level if not at the end
                if (!isLastPart) {
                    currentLevel = &(*currentLevel)[part]["children"];
                }
            }
        }
        
        // Create the message
        json updateMessage = {
            {"type", "directory_update"},
            {"timestamp", std::time(nullptr)},
            {"fileTree", fileTree}
        };
        
        // Broadcast the message to all connected clients
        broadcastMessage(updateMessage.dump());
        
    } catch (const std::exception& e) {
        logError("Error broadcasting directory update: " + std::string(e.what()));
    }
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
        const auto& sourceMap = m_cronus.getSourceMap();
        if (!sourceMap) {
            res.status = 404;
            json errorResponse = {
                {"error", "SourceMap is not initialized yet"}
            };
            res.set_content(errorResponse.dump(), "application/json");
            return;
        }
        
        // Get directory structure from the SourceMap
        const auto& dirStructure = sourceMap->getDirectoryStructure();
        
        // Prepare the response as a flat list for legacy compatibility
        json dirContents = json::array();
        
        // Add all entries from the directory structure
        for (const auto& [path, entry] : dirStructure) {
            if (entry.isDirectory) {
                dirContents.push_back({
                    {"isDirectory", true},
                    {"name", entry.name},
                    {"path", path},
                    {"relativePath", std::filesystem::relative(path, m_cronus.getWorkingDirectory()).string()}
                });
            }
        }
        
        // Add file entries
        const auto& fileCache = sourceMap->getFileCache();
        for (const auto& [path, fileTags] : fileCache) {
            dirContents.push_back({
                {"isDirectory", false},
                {"name", std::filesystem::path(path).filename().string()},
                {"path", path},
                {"relativePath", fileTags.m_relativeFileName}
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

void RestApi::handleGetSourceMap(const httplib::Request&, httplib::Response& res)
{
    try {
        const auto& sourceMap = m_cronus.getSourceMap();
        if (!sourceMap) {
            res.status = 404;
            json errorResponse = {
                {"error", "SourceMap is not initialized yet"}
            };
            res.set_content(errorResponse.dump(), "application/json");
            return;
        }

        const auto& fileCache = sourceMap->getFileCache();
        
        // Build a hierarchical structure
        json fileTree = {};
        
        for (const auto& [path, fileTags] : fileCache) {
            std::string relativePath = fileTags.m_relativeFileName;
            
            // Skip files outside the working directory
            if (relativePath.empty() || relativePath[0] == '.') {
                continue;
            }
            
            // Convert file path to hierarchical structure
            std::vector<std::string> pathParts;
            std::string currentPart;
            std::istringstream pathStream(relativePath);
            
            // Split path by directory separator
            while (std::getline(pathStream, currentPart, '/')) {
                if (!currentPart.empty()) {
                    pathParts.push_back(currentPart);
                }
            }
            
            // Construct the tree structure
            json* currentLevel = &fileTree;
            for (size_t i = 0; i < pathParts.size(); ++i) {
                const std::string& part = pathParts[i];
                bool isLastPart = (i == pathParts.size() - 1);
                
                // If this part doesn't exist in the current level, add it
                if (!currentLevel->contains(part)) {
                    if (isLastPart) {
                        // It's a file
                        (*currentLevel)[part] = {
                            {"type", "file"},
                            {"path", path},
                            {"tags", json::array()}
                        };
                        
                        // Add file tags if available
                        for (const auto& tag : fileTags.m_tags) {
                            (*currentLevel)[part]["tags"].push_back({
                                {"name", tag.name},
                                {"type", static_cast<int>(tag.type)},
                                {"start", tag.start},
                                {"end", tag.end}
                            });
                        }
                    } else {
                        // It's a directory
                        (*currentLevel)[part] = {
                            {"type", "directory"},
                            {"children", json::object()}
                        };
                    }
                }
                
                // Move to the next level if not at the end
                if (!isLastPart) {
                    currentLevel = &(*currentLevel)[part]["children"];
                }
            }
        }
        
        // Prepare the response
        json response = {
            {"fileTree", fileTree}
        };
        
        res.set_content(response.dump(), "application/json");
        
    } catch (const std::exception& e) {
        res.status = 500;
        json errorResponse = {
            {"error", std::string("Error getting sourcemap data: ") + e.what()}
        };
        res.set_content(errorResponse.dump(), "application/json");
    }
}

} // namespace cronus
