#include "cronus/client_logic.h"

namespace cronus {

ClientLogic::ClientLogic(TerminalUI& ui) : ui_(ui) {}

int ClientLogic::run() {
    ui_.display_welcome();
    
    std::string user_input = ui_.get_user_input();
    
    // Try OpenAI
    int result = process_completion("gpt-3.5-turbo", user_input);
    if (result != 0) {
        return result;
    }
    
    // Try Anthropic
    result = process_completion("claude-2", user_input);
    if (result != 0) {
        return result;
    }
    
    return 0;
}

int ClientLogic::process_completion(const std::string& model, const std::string& input) {
    llm_hermes::CompletionResponse response;
    llm_hermes::ErrorCode result = llm_hermes::completion(
        {
            .model = model,
            .messages = {
                {"user", input}
            }
        },
        response
    );

    if (result != llm_hermes::ErrorCode::Success) {
        ui_.display_error(model + " completion failed with error code: " + 
                         std::to_string(static_cast<int>(result)));
        return 1;
    }
    
    ui_.display_response(response.provider, response.text);
    return 0;
}

} // namespace cronus
