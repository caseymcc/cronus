#include "cronus/restApi.h"

#include "cronus/logger.h"

#include <nlohmann/json.hpp>
#include <sstream>
#include <chrono>
#include <random>
#include <iomanip>
#include <fstream>

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
    
    // Clean up all SSE clients
    {
        std::lock_guard<std::mutex> lock(m_sseClientsMutex);
        for (auto& client : m_sseClients) {
            client->connected = false;
        }
        m_sseClients.clear();
    }
    
    m_server->stop();
    
    if (m_serverThread.joinable()) {
        m_serverThread.join();
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
    
    // Get file content endpoint
    m_server->Get("/api/file", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetFileContent(req, res);
    });
    
    // Health check endpoint
    m_server->Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "ok"},
            {"message", "Cronus API is running"}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // SSE endpoint for real-time updates
    m_server->Get("/api/events", [this](const httplib::Request& req, httplib::Response& res) {
        handleSSEConnection(req, res);
    });
}

// Generate a unique ID for SSE clients
std::string generateClientId() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y%m%d%H%M%S");
    
    // Add a random component
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10000, 99999);
    ss << "-" << dis(gen);
    
    return ss.str();
}

void RestApi::handleSSEConnection(const httplib::Request& req, httplib::Response& res) 
{
    // Set headers for SSE
    res.set_header("Content-Type", "text/event-stream");
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Connection", "keep-alive");
    res.set_header("X-Accel-Buffering", "no"); // For Nginx
    
    auto client = std::make_shared<SSEClient>();
    client->id = generateClientId();
    client->connected = true;
    
    {
        std::lock_guard<std::mutex> lock(m_sseClientsMutex);
        m_sseClients.push_back(client);
    }
    
    logInfo("New SSE connection established with ID: " + client->id);
    
    // Send initial keepalive comment
    res.set_content(": keepalive\n\n", "text/event-stream");
    
    // Send initial directory structure
    json initialMessage = {
        {"type", "directory_init"},
        {"message", "Sending initial directory structure"}
    };
    
    std::stringstream ss;
    ss << "data: " << initialMessage.dump() << "\n\n";
    res.set_content(ss.str(), "text/event-stream");
    
    // Setup client send function
    client->send = [&](const std::string& message) -> bool {
        try {
            std::stringstream ss;
            ss << "data: " << message << "\n\n";
            res.set_content(ss.str(), "text/event-stream");
            return true;
        } catch (...) {
            return false;
        }
    };
    
    // Trigger a directory update
    broadcastDirectoryUpdate();
    
    // Keep the connection open with periodic keepalive messages
    auto start_time = std::chrono::steady_clock::now();
    while (client->connected && 
           m_running && 
           std::chrono::steady_clock::now() - start_time < std::chrono::seconds(600)) {  // 10-minute timeout
        
        try {
            // Send a keepalive comment every 15 seconds
            res.set_content(": keepalive\n\n", "text/event-stream");
            std::this_thread::sleep_for(std::chrono::seconds(15));
        } catch (...) {
            // Connection broken
            break;
        }
    }
    
    // Remove the client from our list
    {
        std::lock_guard<std::mutex> lock(m_sseClientsMutex);
        auto it = std::find_if(m_sseClients.begin(), m_sseClients.end(),
            [&client](const std::shared_ptr<SSEClient>& c) { return c->id == client->id; });
        
        if (it != m_sseClients.end()) {
            (*it)->connected = false;
            m_sseClients.erase(it);
        }
    }
    
    logInfo("SSE connection closed for client: " + client->id);
}

void RestApi::broadcastMessage(const std::string& message) 
{
    std::lock_guard<std::mutex> lock(m_sseClientsMutex);
    
    // Make a copy of the client list to avoid issues if clients are disconnected during iteration
    auto clients = m_sseClients;
    
    for (auto& client : clients) {
        if (client->connected) {
            try {
                bool success = false;
                
                if (client->send) {
                    success = client->send(message);
                }
                
                if (!success) {
                    // If send fails, mark the client as disconnected
                    client->connected = false;
                }
            } catch (const std::exception& e) {
                logError("Error broadcasting message to client " + client->id + ": " + e.what());
                client->connected = false;
            }
        }
    }
    
    // Clean up disconnected clients
    m_sseClients.erase(
        std::remove_if(m_sseClients.begin(), m_sseClients.end(),
            [](const std::shared_ptr<SSEClient>& client) { return !client->connected; }),
        m_sseClients.end());
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

void RestApi::handleGetFileContent(const httplib::Request& req, httplib::Response& res)
{
    try {
        if (!req.has_param("path")) {
            res.status = 400;
            json errorResponse = {
                {"error", "Missing 'path' parameter in request"}
            };
            res.set_content(errorResponse.dump(), "application/json");
            return;
        }

        std::string filePath = req.get_param_value("path");
        logInfo("API requested file content for path: " + filePath);

        // Check if file exists in the SourceMap for metadata
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
        auto it = fileCache.find(filePath);
        if (it == fileCache.end()) {
            res.status = 404;
            json errorResponse = {
                {"error", "File not found in SourceMap"}
            };
            res.set_content(errorResponse.dump(), "application/json");
            return;
        }

        // Load the file content directly from the file system
        std::ifstream file(filePath);
        if (!file.is_open()) {
            res.status = 404;
            json errorResponse = {
                {"error", "Failed to open file: " + filePath}
            };
            res.set_content(errorResponse.dump(), "application/json");
            return;
        }

        // Read the file content
        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        // Remove the last newline if it exists
        if (!content.empty() && content.back() == '\n') {
            content.pop_back();
        }

        const auto& fileTags = it->second;
        json response = {
            {"path", filePath},
            {"content", content},
            {"tags", json::array()}
        };

        for (const auto& tag : fileTags.m_tags) {
            response["tags"].push_back({
                {"name", tag.name},
                {"type", static_cast<int>(tag.type)},
                {"start", tag.start},
                {"end", tag.end}
            });
        }

        res.set_content(response.dump(), "application/json");

    } catch (const std::exception& e) {
        res.status = 500;
        json errorResponse = {
            {"error", std::string("Error getting file content: ") + e.what()}
        };
        res.set_content(errorResponse.dump(), "application/json");
    }
}

} // namespace cronus
