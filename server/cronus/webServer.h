#ifndef _cronus_webServer_h_
#define _cronus_webServer_h_

#include "cronus/cronus.h"
#include "cronus/logger.h"

#include <httplib.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <filesystem>

namespace cronus
{

class WebServer
{
public:
    explicit WebServer(Cronus &cronus, int port = 8080);
    ~WebServer();

    bool start();
    void stop();
    bool isRunning() const { return m_running; }
    std::string getBaseUrl() const { return "http://localhost:" + std::to_string(m_port); }

private:
    void setupRoutes();
    std::string getContentType(const std::string& path);
    
    Cronus &m_cronus;
    std::unique_ptr<httplib::Server> m_server;
    std::thread m_serverThread;
    std::atomic<bool> m_running{false};
    int m_port;
    std::string m_frontendPath;
};

} // namespace cronus

#endif//_cronus_webServer_h_