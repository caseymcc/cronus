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
    m_running=true;
    m_workerThread=std::thread(&Cronus::workerLoop, this);
}

void Cronus::stop()
{
    m_running=false;
    m_condition.notify_one();
    if(m_workerThread.joinable())
    {
        m_workerThread.join();
    }
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
    std::string workingDir=m_currentPath.string();

    m_sourceMap=std::make_shared<SourceMap>(workingDir);
    m_inputParser=std::make_shared<InputParser>(m_sourceMap, m_currentPath);
    m_coder=std::make_shared<agents::Coder>(m_sourceMap);
    m_commandHandler=std::make_shared<CommandHandler>(m_sourceMap);

    // Try to load the source map cache
    logInfo("Loading source map cache...");
    m_sourceMap->loadTagsCache();

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

std::future<int> Cronus::processInput(const std::string &input)
{
    Task task{ Task::Type::Completion, 0, input, Config::instance().getProvider() };
    std::promise<int> promise;
    auto future=promise.get_future();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push(task);
    }
    m_condition.notify_one();

    return future;
}

std::vector<std::string> Cronus::buildMessage(const std::string &input)
{
    std::vector<std::string> messages;

    // Parse input for file and tag references
    std::vector<std::string> fileRefs=m_inputParser->extractFileReferences(input);
    std::vector<std::string> tagRefs=m_inputParser->extractTagReferences(input);

    // Log what we found
    if(!fileRefs.empty())
    {
        std::string fileRefsStr="Found file references: ";
        for(const auto &file:fileRefs)
        {
            fileRefsStr+=file+", ";
        }
        log(fileRefsStr);
    }

    if(!tagRefs.empty())
    {
        std::string tagRefsStr="Found tag references: ";
        for(const auto &tag:tagRefs)
        {
            tagRefsStr+=tag+", ";
        }
        log(tagRefsStr);
    }

    return messages;
}

int Cronus::processCompletion(const std::string &input)
{
    const auto &config=Config::instance();
    log("Processing completion request...");

    // Check if the input is a command
    if (m_commandHandler && m_commandHandler->isCommand(input)) {
        log("Processing command: " + input);
        std::string response = m_commandHandler->processCommand(input);
        handleResponse("Command", response);
        log("Command processed successfully");
        return 0;
    }

    // Extract file references for context
    std::vector<std::string> fileRefs = m_inputParser->extractFileReferences(input);
    
    // Check if the input appears to be a code generation request
    bool isCodeRequest = input.find("generate") != std::string::npos || 
                         input.find("create") != std::string::npos ||
                         input.find("implement") != std::string::npos ||
                         input.find("write") != std::string::npos;
                         
    // Use the Coder agent for code-related requests
    if (isCodeRequest && m_coder) {
        log("Using Coder agent for code generation request");
        std::string generatedCode = m_coder->generateCode(input, fileRefs);
        handleResponse("Coder", generatedCode);
        log("Code generation completed successfully");
        return 0;
    }
    
    // Get model config for regular LLM requests
    auto modelConfig=config.getModelConfig(config.getModel());

    if(!modelConfig)
    {
        handleError("Model configuration not found for: "+config.getModel());
        return 1;
    }

    // Build context-aware messages
    std::vector<std::string> contextMessages = buildMessage(input);
    
    hermes::CompletionRequest request{
        .model=modelConfig->model,
        .messages={
            {"user", input}
        }
    };

    hermes::ErrorCode result;

    if(modelConfig->streaming)
    {
        result=hermes::streamingCompletion(request,
            [this](const std::string &content)
            {
                handleResponse("streaming", content);
            });
    }
    else
    {
        hermes::CompletionResponse response;
        result=hermes::completion(request, response);
        if(result==hermes::ErrorCode::Success)
        {
            handleResponse(response.provider, response.text);
        }
    }

    if(result!=hermes::ErrorCode::Success)
    {
        handleError(request.model+" completion failed with error code: "+
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
