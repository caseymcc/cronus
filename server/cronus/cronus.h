#ifndef _cronus_client_logic_h_
#define _cronus_client_logic_h_

#include "cronus/logger.h"
#include "cronus/sourceMap.h"
#include "cronus/inputParser.h"
#include "cronus/agents/coder.h"
#include "cronus/commandHandler.h"
#include "cronus/model.h"

#include "loreforge/loreforge.h"

#include <filesystem>
#include <future>
#include <queue>
#include <thread>
#include <functional>
#include <atomic>

namespace cronus
{

// Forward declaration
class WebServer;

struct Task
{
    enum class Type
    {
        Input,
        DirectoryContents
    };

    Type type;
    size_t id;
    std::string input;
    std::string provider;
};

class Cronus
{
public:
    using ResponseCallback=std::function<void(const std::string &, const std::string &)>;

    explicit Cronus();
    ~Cronus();

    void run(const std::string& resourcePath = "");
    void stop();
    void waitForComplete();
    
    std::future<int> processInput(const std::string &input);
    
    // Helper methods for input processing
    std::vector<std::string> extractFileReferences(const std::string &input) const;
    std::vector<std::string> extractTagReferences(const std::string &input) const;
    
    // Callback setters
    void setLogCallback(Logger::LogCallback callback) { Logger::instance().setCallback(callback); }
    void setResponseCallback(ResponseCallback callback) { m_responseCallback=callback; }

    // API related methods
    void startWebServer(int port = 9000);
    void stopWebServer();
    bool isWebServerRunning() const;
    std::string getWebServerBaseUrl() const;
    
    // For API access
    const std::shared_ptr<SourceMap>& getSourceMap() const { return m_sourceMap; }
    const std::string& getWorkingDirectory() const { return m_workingDir; }

    // Worker thread
    void workerLoop();

    int handleInput(const std::string &input);

    // Helper method to update SourceMap and notify clients
    void updateSourceMap();

private:
    std::vector<std::string> buildMessage(const std::string &input);

    void log(const std::string &message) const;
    void handleResponse(const std::string &provider, const std::string &response) const;
    void handleError(const std::string &error) const;

    std::filesystem::path m_currentPath;
    std::string m_resourcePath;
    std::string m_workingDir;

    // Callbacks
    ResponseCallback m_responseCallback;

    //processing thread
    std::queue<Task> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_workerThread;

    std::atomic<bool> m_running{ false };
    std::atomic<bool> m_workerRunning{ false };

    //Used only in the worker thread
    std::shared_ptr<SourceMap> m_sourceMap;
    std::shared_ptr<InputParser> m_inputParser;
    std::shared_ptr<Model> m_model;
    std::shared_ptr<agents::Coder> m_coder;
    std::shared_ptr<CommandHandler> m_commandHandler;
    
    // Web Server
    std::unique_ptr<WebServer> m_webServer;
    
    // List of files explicitly added by the user
    std::vector<std::string> m_addedFiles;
};

} // namespace cronus

#endif//_cronus_client_logic_h_
