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

private:
    void setupRoutes();
    
    // API endpoints
    void handleProcessInput(const httplib::Request& req, httplib::Response& res);
    void handleGetDirectoryContents(const httplib::Request& req, httplib::Response& res);
    
    Cronus& m_cronus;
    int m_port;
    std::unique_ptr<httplib::Server> m_server;
    std::thread m_serverThread;
    std::atomic<bool> m_running{false};
};

} // namespace cronus

#endif//_cronus_rest_api_h_
