#ifndef _cronus_rest_api_h_
#define _cronus_rest_api_h_

#include "cronus/cronus.h"

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>

namespace cronus
{

class RestApi
{
public:
    explicit RestApi(Cronus& cronus, int port = 8080);
    ~RestApi();

    void start();
    void stop();
    
    bool isRunning() const { return m_running; }
    int getPort() const { return m_port; }
    std::string getBaseUrl() const { return "http://localhost:" + std::to_string(m_port); }

    // Event notification methods
    void broadcastMessage(const std::string& message);
    void broadcastDirectoryUpdate();
    void broadcastLog(const std::string& level, const std::string& message);

private:
    void setupRoutes();
    void setupLoggerCallback();
    
    // API endpoints
    void handleProcessInput(const httplib::Request& req, httplib::Response& res);
    void handleGetDirectoryContents(const httplib::Request& req, httplib::Response& res);
    void handleGetSourceMap(const httplib::Request& req, httplib::Response& res);
    void handleGetFileContent(const httplib::Request& req, httplib::Response& res);
    void handleGetLogs(const httplib::Request& req, httplib::Response& res); // Get historical logs
    
    // SSE (Server-Sent Events) management
    void handleSSEConnection(const httplib::Request& req, httplib::Response& res);
    
    // SSE client connections
    struct SSEClient {
        std::string id;
        std::function<bool(const std::string&)> send;
        bool connected;
    };
    
    std::mutex m_sseClientsMutex;
    std::vector<std::shared_ptr<SSEClient>> m_sseClients;
    
    // Log history
    struct LogEntry {
        std::string timestamp;
        std::string level;
        std::string message;
    };
    std::mutex m_logsMutex;
    std::vector<LogEntry> m_logHistory;
    size_t m_maxLogHistory = 1000; // Keep last 1000 log entries
    
    Cronus& m_cronus;
    int m_port;
    std::unique_ptr<httplib::Server> m_server;
    std::thread m_serverThread;
    std::atomic<bool> m_running{false};
};

} // namespace cronus

#endif//_cronus_rest_api_h_
