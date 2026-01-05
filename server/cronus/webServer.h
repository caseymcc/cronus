#ifndef _cronus_webServer_h_
#define _cronus_webServer_h_

#include "cronus/cronus.h"
#include "cronus/logger.h"

#include <crow.h>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace cronus
{

using json = nlohmann::json;

class WebServer
{
public:
    explicit WebServer(Cronus &cronus, int port = 9000);
    ~WebServer();

    bool start();
    void stop();
    bool isRunning() const { return m_running; }
    std::string getBaseUrl() const { return "http://localhost:" + std::to_string(m_port); }
    
    // Broadcast notifications to all connected WebSocket clients
    void broadcastNotification(const std::string& method, const json& params);
    void broadcastMessage(const std::string& role, const std::string& content);
    void broadcastLog(const std::string& level, const std::string& message);
    void broadcastDirectoryUpdate();

private:
    void setupRoutes();
    std::string getContentType(const std::string& path);
    
    // JSON-RPC handlers
    using JsonRpcHandler = std::function<json(const json& params)>; 
    void registerJsonRpcMethod(const std::string& method, JsonRpcHandler handler);
    json handleJsonRpcRequest(const std::string& message);
    
    // Method implementations
    json handleInput(const json& params);
    json handleGetSourceMap(const json& params);
    json handleGetFile(const json& params);
    json handleGetDirectory(const json& params);
    json handleGetLogs(const json& params);
    
    Cronus &m_cronus;
    crow::SimpleApp m_app;
    std::thread m_serverThread;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_routesSetup{false};
    int m_port;
    std::string m_frontendPath;
    
    // WebSocket connection management
    std::mutex m_wsMutex;
    std::unordered_set<crow::websocket::connection*> m_wsConnections;
    
    // JSON-RPC method registry
    std::unordered_map<std::string, JsonRpcHandler> m_jsonRpcMethods;
    
    // Log history
    struct LogEntry {
        std::string timestamp;
        std::string level;
        std::string message;
    };
    std::mutex m_logsMutex;
    std::vector<LogEntry> m_logHistory;
    size_t m_maxLogHistory = 1000;
};

} // namespace cronus

#endif//_cronus_webServer_h_