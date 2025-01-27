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
}

void ClientLogic::stop()
{
}

std::future<int> ClientLogic::processInput(const std::string &input)
{
}

void ClientLogic::log(const std::string &message) const
{
    if(m_logCallback)
    {
        m_logCallback(message);
    }
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
    if(m_errorCallback)
    {
        m_errorCallback(error);
    }
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
            updateDirectoryTree();
            break;
        }
    }
}

int ClientLogic::processCompletion(const std::string &input)
{
    const auto &config=Config::instance();
    log("Processing completion request...");

    llm_hermes::CompletionRequest request{
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

    llm_hermes::CompletionResponse response;
    llm_hermes::ErrorCode result=llm_hermes::completion(request, response);

    if(result!=llm_hermes::ErrorCode::Success)
    {
        handleError(request.model+" completion failed with error code: "+
            std::to_string(static_cast<int>(result)));
        return 1;
    }

    handleResponse(response.provider, response.text);
    log("Completion processed successfully");
    return 0;
}

void ClientLogic::updateDirectoryTree()
{
    auto contents=getCurrentDirectoryContents();
    m_ui.updateDirectoryTree(contents);
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
