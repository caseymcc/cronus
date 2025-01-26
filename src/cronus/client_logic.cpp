#include "cronus/client_logic.h"

#include "cronus/config.h"

namespace cronus
{

ClientLogic::ClientLogic(TerminalUI &ui) : ui_(ui) {}

int ClientLogic::run()
{
    ui_.display_welcome();

    std::string userInput = ui_.getUserInput();
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
        request.apiKey = *apiKey;
    }

    llm_hermes::CompletionResponse response;
    llm_hermes::ErrorCode result=llm_hermes::completion(request, response);

    if(result!=llm_hermes::ErrorCode::Success)
    {
        ui_.display_error(model+" completion failed with error code: "+
            std::to_string(static_cast<int>(result)));
        return 1;
    }

    ui_.display_response(response.provider, response.text);
    return 0;
}

} // namespace cronus
