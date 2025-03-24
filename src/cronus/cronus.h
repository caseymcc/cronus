#ifndef _cronus_client_logic_h_
#define _cronus_client_logic_h_

#include "cronus/logger.h"
#include "cronus/sourceMap.h"
#include "cronus/inputParser.h"
#include "cronus/agents/coder.h"
#include "cronus/commandHandler.h"

#include "hermes/hermes.h"

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

class Cronus
{
public:
    using ResponseCallback=std::function<void(const std::string &, const std::string &)>;

    explicit Cronus();
    ~Cronus();

    void run();
    void stop();
    std::future<int> processInput(const std::string &input);
    
    // Helper methods for input processing
    std::vector<std::string> extractFileReferences(const std::string &input) const;
    std::vector<std::string> extractTagReferences(const std::string &input) const;
    std::vector<std::pair<bool, std::string>> getCurrentDirectoryContents() const;

    // Callback setters
    void setLogCallback(Logger::LogCallback callback) { Logger::instance().setCallback(callback); }
    void setResponseCallback(ResponseCallback callback) { m_responseCallback=callback; }

    // Worker thread
    void workerLoop();

    int processCompletion(const std::string &input);
private:
    std::vector<std::string> buildMessage(const std::string &input);

    void log(const std::string &message) const;
    void handleResponse(const std::string &provider, const std::string &response) const;
    void handleError(const std::string &error) const;

    std::filesystem::path m_currentPath;

    // Callbacks
    ResponseCallback m_responseCallback;

    //processing thread
    std::queue<Task> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_workerThread;
    bool m_running{ false };
    
    //Used only in the worker thread
    std::shared_ptr<SourceMap> m_sourceMap;
    std::shared_ptr<InputParser> m_inputParser;
    std::shared_ptr<agents::Coder> m_coder;
    std::shared_ptr<CommandHandler> m_commandHandler;
};

} // namespace cronus

#endif//_cronus_client_logic_h_
