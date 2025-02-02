#ifndef _cronus_client_logic_h_
#define _cronus_client_logic_h_

#include "cronus/terminal_ui.h"

#include "llm_hermes/hermes.h"

#include <filesystem>
#include <future>
#include <queue>
#include <thread>

namespace cronus
{

struct Task
{
    enum class Type
    {
        Completion,
        DirectoryContents
    };

    Type type;
    size_t id;
    std::string input;
    std::string provider;
};

class ClientLogic
{
public:
    using LogCallback=std::function<void(const std::string &)>;
    using ResponseCallback=std::function<void(const std::string &, const std::string &)>;
    using ErrorCallback=std::function<void(const std::string &)>;

    explicit ClientLogic();
    ~ClientLogic();

    void run();
    void stop();
    std::future<int> processInput(const std::string &input);
    std::vector<std::pair<bool, std::string>> getCurrentDirectoryContents() const;

    // Callback setters
    void setLogCallback(LogCallback callback) { m_logCallback=callback; }
    void setResponseCallback(ResponseCallback callback) { m_responseCallback=callback; }
    void setErrorCallback(ErrorCallback callback) { m_errorCallback=callback; }

    // Worker thread
    void workerLoop();

    int processCompletion(const std::string &input);
private:
    void log(const std::string &message) const;
    void handleResponse(const std::string &provider, const std::string &response) const;
    void handleError(const std::string &error) const;

    std::filesystem::path m_currentPath;

    // Callbacks
    LogCallback m_logCallback;
    ResponseCallback m_responseCallback;
    ErrorCallback m_errorCallback;

    //processing thread
    std::queue<Task> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_worker;
    bool m_running;
};

} // namespace cronus

#endif//_cronus_client_logic_h_
