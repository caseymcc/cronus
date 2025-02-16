#include "cronus/cronus.h"

#include "cronus/config.h"

namespace cronus
{

Cronus::Cronus() :
    m_currentPath(std::filesystem::current_path())
{
}

Cronus::~Cronus()=default;

void Cronus::run()
{
    m_running = true;
    m_workerThread = std::thread(&Cronus::workerLoop, this);
}

void Cronus::stop()
{
    m_running = false;
    m_condition.notify_one();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

std::future<int> Cronus::processInput(const std::string &input)
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

void Cronus::log(const std::string &message) const
{
    logInfo(message);
}

void Cronus::handleResponse(const std::string &provider, const std::string &response) const
{
    if(m_responseCallback)
    {
        m_responseCallback(provider, response);
    }
}

void Cronus::handleError(const std::string &error) const
{
    logError(error);
}

void Cronus::workerLoop()
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

int Cronus::processCompletion(const std::string &input)
{
    const auto &config=Config::instance();
    log("Processing completion request...");

    // Get model config
    auto modelConfig = config.getModelConfig(config.getModel());
    
    if (!modelConfig) {
        handleError("Model configuration not found for: " + config.getModel());
        return 1;
    }

    hermes::CompletionRequest request{
        .model=modelConfig->model,
        .messages={
            {"user", input}
        }
    };

    hermes::ErrorCode result;
    
    if (modelConfig->streaming) {
        result = hermes::streamingCompletion(request, 
            [this](const std::string& content) {
                handleResponse("streaming", content);
            });
    } else {
        hermes::CompletionResponse response;
        result = hermes::completion(request, response);
        if (result == hermes::ErrorCode::Success) {
            handleResponse(response.provider, response.text);
        }
    }

    if (result != hermes::ErrorCode::Success) {
        handleError(request.model + " completion failed with error code: " +
            std::to_string(static_cast<int>(result)));
        return 1;
    }

    log("Completion processed successfully");
    return 0;
}

std::vector<std::pair<bool, std::string>> Cronus::getCurrentDirectoryContents() const
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
