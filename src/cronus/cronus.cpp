#include "cronus/cronus.h"

#include "cronus/config.h"

namespace cronus
{

Cronus::Cronus() :
    m_currentPath(std::filesystem::current_path())
{
    // Initialize source map with current working directory
    std::string workingDir = m_currentPath.string();
    m_sourceMap = std::make_unique<SourceMap>(workingDir);
    
    // Try to load the source map cache
    logInfo("Loading source map cache...");
    m_sourceMap->loadTagsCache();
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
    // Parse input for file and tag references
    std::vector<std::string> fileRefs = extractFileReferences(input);
    std::vector<std::string> tagRefs = extractTagReferences(input);
    
    // Log what we found
    if (!fileRefs.empty()) {
        std::string fileRefsStr = "Found file references: ";
        for (const auto& file : fileRefs) {
            fileRefsStr += file + ", ";
        }
        log(fileRefsStr);
    }
    
    if (!tagRefs.empty()) {
        std::string tagRefsStr = "Found tag references: ";
        for (const auto& tag : tagRefs) {
            tagRefsStr += tag + ", ";
        }
        log(tagRefsStr);
    }
    
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

std::vector<std::string> Cronus::extractFileReferences(const std::string &input) const
{
    std::vector<std::string> fileReferences;
    
    // Simple regex-like pattern matching for file paths
    // Look for patterns like: filename.ext, path/to/file.ext, ./file.ext, etc.
    std::istringstream iss(input);
    std::string word;
    
    while (iss >> word) {
        // Check if word looks like a file path
        if (word.find('.') != std::string::npos) {
            // Check if it's in our source map
            std::filesystem::path potentialPath = m_currentPath / word;
            if (std::filesystem::exists(potentialPath) && !std::filesystem::is_directory(potentialPath)) {
                fileReferences.push_back(potentialPath.string());
            } else {
                // Try relative to current directory
                for (const auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
                    if (entry.path().filename().string() == word) {
                        fileReferences.push_back(entry.path().string());
                        break;
                    }
                }
            }
        }
    }
    
    return fileReferences;
}

std::vector<std::string> Cronus::extractTagReferences(const std::string &input) const
{
    std::vector<std::string> tagReferences;
    
    // Get all files in the source map
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_currentPath)) {
        if (!entry.is_regular_file()) continue;
        
        // Get tags for this file
        std::vector<Tag> tags = m_sourceMap->getTags(entry.path().string(), "");
        
        // Check if any tag names are mentioned in the input
        for (const auto& tag : tags) {
            if (input.find(tag.name) != std::string::npos) {
                tagReferences.push_back(tag.name);
            }
        }
    }
    
    // Remove duplicates
    std::sort(tagReferences.begin(), tagReferences.end());
    tagReferences.erase(std::unique(tagReferences.begin(), tagReferences.end()), tagReferences.end());
    
    return tagReferences;
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
