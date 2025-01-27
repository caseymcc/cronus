#include "cronus/client_logic.h"

#include "cronus/config.h"

namespace cronus
{

ClientLogic::ClientLogic() : 
    m_currentPath(std::filesystem::current_path()),
    m_taskSystem(std::make_unique<TaskSystem>())
{
}

ClientLogic::~ClientLogic() = default;

void ClientLogic::start()
{
    m_ui.onInput = [this](const std::string& input) {
        auto future = m_taskSystem->enqueue([this, input]() {
            return processCompletion(input);
        });
        // Handle the future in a separate task to avoid blocking
        m_taskSystem->enqueue([future = std::move(future)]() mutable {
            future.get();
        });
    };
    m_ui.run();
}

int ClientLogic::processCompletion(const std::string &input)
{
    const auto &config = Config::instance();
    llm_hermes::CompletionRequest request{
        .model = config.getModel(),
        .messages = {
            {"user", input}
        }
    };

    // Add API key if configured
    auto apiKey = config.getApiKey(config.getProvider());
    if(apiKey)
    {
        request.api_key = *apiKey;
    }

    llm_hermes::CompletionResponse response;
    llm_hermes::ErrorCode result=llm_hermes::completion(request, response);

    if(result!=llm_hermes::ErrorCode::Success)
    {
        m_ui.displayError(request.model+" completion failed with error code: "+
            std::to_string(static_cast<int>(result)));
        return 1;
    }

    m_ui.displayResponse(response.provider, response.text);
    return 0;
}

void ClientLogic::updateDirectoryTree() {
    auto contents = getCurrentDirectoryContents();
    m_ui.updateDirectoryTree(contents);
}

std::vector<std::pair<bool, std::string>> ClientLogic::getCurrentDirectoryContents() const {
    std::vector<std::pair<bool, std::string>> contents;
    for(const auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
        contents.emplace_back(
            entry.is_directory(),
            entry.path().filename().string()
        );
    }
    return contents;
}

} // namespace cronus
