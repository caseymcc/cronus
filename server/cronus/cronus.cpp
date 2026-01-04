#include "cronus/cronus.h"

#include "cronus/webServer.h"
#include "cronus/config.h"
#include "cronus/agents/promptManager.h"

namespace cronus
{

Cronus::Cronus() :
    m_currentPath(std::filesystem::current_path()),
    m_workingDir(m_currentPath.string())
{
}

Cronus::~Cronus()
{
    stop();
}

void Cronus::run(const std::string& resourcePath)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_running)
            return; // Already running

        m_running = true;
        m_resourcePath = resourcePath;
    }

    startWebServer(9000);
    m_workerThread = std::thread(&Cronus::workerLoop, this);

    //wait for thread to start
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        while(!m_workerRunning)
        {
            m_condition.wait(lock);
        }
    }
}

void Cronus::stop()
{
    // Stop the web server if it's running
    stopWebServer();
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_running = false;
    }

    m_condition.notify_all();
    if (m_workerThread.joinable())
    {
        m_workerThread.join();
    }
}

void Cronus::waitForComplete()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (!m_workerRunning)
        return; // Worker already stopped

    while(m_workerRunning)
    {
        m_condition.wait(lock);
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
    {
        std::unique_lock<std::mutex> lock(m_mutex);   
        // Set the worker running flag to true at the start
        m_workerRunning = true;
    }
    m_condition.notify_all();

    logInfo("Worker thread started");
    m_workingDir = m_currentPath.string();

    m_sourceMap = std::make_shared<SourceMap>(m_workingDir);
    m_inputParser = std::make_shared<InputParser>(m_sourceMap, m_currentPath);

    // Get model configuration from Config
    const auto &config = Config::instance();
    
    auto modelConfig = config.getModelConfig(config.getModel());
    
    m_model = std::make_shared<Model>(config.getModel(), config.getProvider());
    
    // Initialize PromptManager with the same paths used for configuration
    agents::PromptManager::instance().initialize(config.getConfigPaths());
    
    m_coder = std::make_shared<agents::Coder>(m_sourceMap, m_model);
    m_commandHandler = std::make_shared<CommandHandler>(m_sourceMap, m_addedFiles);

    // Try to load the source map cache
    logInfo("Loading source map cache...");
    m_sourceMap->loadTagsCache();
    
    // Build the directory structure for file browsing
    logInfo("Building directory structure...");
    m_sourceMap->updateDirectoryStructure();

    bool running=true;

    while(running)
    {
        Task task;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            
            if(!m_running)
            {
                running=m_running;
                continue;
            }
            
            if(m_tasks.empty())
            {
                m_condition.wait(lock);
                continue;
            }

            task=std::move(m_tasks.front());
            m_tasks.pop();    
        }

        switch(task.type)
        {
        case Task::Type::Input:
            handleInput(task.input);
            break;
        case Task::Type::DirectoryContents:
            break;
        }
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex);   
        // Set the worker running flag to true at the start
        m_workerRunning = false;
    }
    m_condition.notify_all();
}

std::future<int> Cronus::processInput(const std::string &input)
{
    Task task{ Task::Type::Input, 0, input, Config::instance().getProvider() };
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

int Cronus::handleInput(const std::string &input)
{
    const auto &config = Config::instance();
    log("Processing completion request...");

    // Check if the input is a command
    if (m_commandHandler && m_commandHandler->isCommand(input))
    {
        log("Processing command: " + input);
        std::string response = m_commandHandler->processCommand(input);
        handleResponse("Command", response);
        log("Command processed successfully");
        
        // Update the source map after command execution
        // Commands may have modified files or added new ones
        updateSourceMap();
        return 0;
    }

    // Extract file references for context
    std::vector<std::string> fileRefs = m_inputParser->extractFileReferences(input);

    if (m_coder)
    {
        log("Using Coder agent for code generation request");
        std::string generatedCode = m_coder->generateCode(input, fileRefs, m_addedFiles);
        handleResponse("Coder", generatedCode);
        log("Code generation completed successfully");
        
        // Update the source map after code generation
        // Code generation may have modified files
        updateSourceMap();
        return 0;
    }

    // Build context-aware messages
    std::vector<std::string> contextMessages=buildMessage(input);

    // Use the Model class for generation
    std::string response=m_model->generate(
        { {"user", input} },
        config.getModelConfig(config.getModel())->streaming,
        [this](const std::string &content)
        {
            handleResponse("streaming", content);
        }
    );

    // Check if there was an error (error responses start with "Error:")
    if(response.substr(0, 6)=="Error:")
    {
        handleError(response);
        return 1;
    }

    // For non-streaming responses, handle the response here
    if(!config.getModelConfig(config.getModel())->streaming)
    {
        handleResponse(config.getProvider(), response);
    }

    log("Completion processed successfully");
    return 0;
}

void Cronus::startWebServer(int port)
{
    if (!m_webServer) {
        m_webServer = std::make_unique<WebServer>(*this, port);
    }
    
    if (!m_webServer->isRunning()) {
        m_webServer->start();
        
        // Wire up the response callback to broadcast messages to all connected clients
        setResponseCallback([this](const std::string& provider, const std::string& response) {
            if (m_webServer && m_webServer->isRunning()) {
                m_webServer->broadcastMessage("agent", response);
            }
        });
        
        Logger::instance().info("Web Server started on " + m_webServer->getBaseUrl());
    } else {
        Logger::instance().warning("Web Server is already running");
    }
}

void Cronus::stopWebServer()
{
    if (m_webServer && m_webServer->isRunning()) {
        m_webServer->stop();
        Logger::instance().info("Web Server stopped");
    }
}

bool Cronus::isWebServerRunning() const
{
    return m_webServer && m_webServer->isRunning();
}

std::string Cronus::getWebServerBaseUrl() const
{
    if (m_webServer) {
        return m_webServer->getBaseUrl();
    }
    return "";
}

std::vector<std::string> Cronus::extractFileReferences(const std::string &input) const
{
    if (m_inputParser) {
        return m_inputParser->extractFileReferences(input);
    }
    return {};
}

std::vector<std::string> Cronus::extractTagReferences(const std::string &input) const
{
    if (m_inputParser) {
        return m_inputParser->extractTagReferences(input);
    }
    return {};
}

void Cronus::updateSourceMap()
{
    if (m_sourceMap) {
        m_sourceMap->update();
        m_sourceMap->updateDirectoryStructure();
        
        // Notify connected clients about directory changes
        if (m_webServer) {
            m_webServer->broadcastDirectoryUpdate();
        }
    }
}

} // namespace cronus
