#include "cronus/client_logic.h"

#include "cronus/config.h"

namespace cronus
{

ClientLogic::ClientLogic(TerminalUI &ui) : 
    m_ui(ui),
    m_currentPath(std::filesystem::current_path()),
    m_taskSystem(std::make_unique<TaskSystem>())
{
    updateDirectoryTree();
}

ClientLogic::~ClientLogic() = default;

void ClientLogic::start()
{
    m_ui.displayWelcome();
}

std::future<int> ClientLogic::processInput(const std::string &input)
{
    return m_taskSystem->enqueue([this, input]() {
        return processCompletion(input);
    });
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
