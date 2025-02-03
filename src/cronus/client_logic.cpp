#include "cronus/client_logic.h"

#include "cronus/config.h"

namespace cronus
{

ClientLogic::ClientLogic() :
    m_currentPath(std::filesystem::current_path())
{
}

ClientLogic::~ClientLogic()=default;

void ClientLogic::run()
{
    m_running = true;
    m_workerThread = std::thread(&ClientLogic::workerLoop, this);
}

void ClientLogic::stop()
{
    m_running = false;
    m_condition.notify_one();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

std::future<int> ClientLogic::processInput(const std::string &input)
{
    Task task{Task::Type::Completion, 0, input, Config::instance().getProvider()};
    std::promise<int> promise;
    auto future = promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push(task);
    }
    m_condition.notify_one();
    
    return future;
}

void ClientLogic::log(const std::string &message) const
{
    logInfo(message);
}

void ClientLogic::handleResponse(const std::string &provider, const std::string &response) const
{
    if(m_responseCallback)
    {
        m_responseCallback(provider, response);
    }
}

void ClientLogic::handleError(const std::string &error) const
{
    logError(error);
}

void ClientLogic::workerLoop()
{
    while(true)
    {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this]
                {
                    return !m_tasks.empty()||!m_running;
                });

            if(!m_running&&m_tasks.empty())
            {
                return;
            }

            task=std::move(m_tasks.front());
            m_tasks.pop();
        }
        switch(task.type)
        {
        case Task::Type::Completion:
            processCompletion(task.input);
            break;
        case Task::Type::DirectoryContents:
            break;
        }
    }
}

int ClientLogic::processCompletion(const std::string &input)
{
    const auto &config=Config::instance();
    log("Processing completion request...");

    hermes::CompletionRequest request{
        .model=config.getModel(),
        .messages={
            {"user", input}
        }
    };

    // Add API key if configured
    auto apiKey=config.getApiKey(config.getProvider());
    if(apiKey)
    {
        request.api_key=*apiKey;
    }
    else
    {
        handleError("API key not found for provider: "+config.getProvider());
        return 1;
    }

    hermes::CompletionResponse response;
    hermes::ErrorCode result=hermes::completion(request, response);

    if(result!=hermes::ErrorCode::Success)
    {
        handleError(request.model+" completion failed with error code: "+
            std::to_string(static_cast<int>(result)));
        return 1;
    }

    handleResponse(response.provider, response.text);
    log("Completion processed successfully");
    return 0;
}

std::vector<std::pair<bool, std::string>> ClientLogic::getCurrentDirectoryContents() const
{
    std::vector<std::pair<bool, std::string>> contents;
    for(const auto &entry:std::filesystem::directory_iterator(m_currentPath))
    {
        contents.emplace_back(
            entry.is_directory(),
            entry.path().filename().string()
        );
    }
    return contents;
}

} // namespace cronus
