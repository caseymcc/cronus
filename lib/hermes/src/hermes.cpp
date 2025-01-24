#include "cronus/hermes.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

namespace cronus {

OpenAIClient::OpenAIClient(const std::string& api_key) : api_key_(api_key) {}

CompletionResponse OpenAIClient::complete(const CompletionRequest& request) {
    // TODO: Implement OpenAI API call using cpr
    CompletionResponse response;
    return response;
}

AnthropicClient::AnthropicClient(const std::string& api_key) : api_key_(api_key) {}

CompletionResponse AnthropicClient::complete(const CompletionRequest& request) {
    // TODO: Implement Anthropic API call using cpr
    CompletionResponse response;
    return response;
}

} // namespace cronus
