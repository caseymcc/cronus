#include "cronus/client_logic.h"

#include "cronus/config.h"

namespace cronus
{

ClientLogic::ClientLogic(TerminalUI &ui) : m_ui(ui) {}

int ClientLogic::run()
{
    m_ui.displayWelcome();

    std::string userInput = m_ui.getUserInput();
    return processCompletion(userInput);
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

} // namespace cronus
