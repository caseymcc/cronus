#include "cronus/client_logic.h"

#include "cronus/config.h"

namespace cronus
{

ClientLogic::ClientLogic(TerminalUI &ui) : ui_(ui) {}

int ClientLogic::run()
{
    ui_.display_welcome();

    std::string user_input=ui_.get_user_input();
    return process_completion(user_input);
}

int ClientLogic::process_completion(const std::string &input)
{
    const auto &config=Config::instance();
    llm_hermes::CompletionRequest request{
        .model=config.get_model(),
        .messages={
            {"user", input}
        }
    };

    // Add API key if configured
    auto api_key=config.get_api_key(config.get_provider());
    if(api_key)
    {
        request.api_key=*api_key;
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
