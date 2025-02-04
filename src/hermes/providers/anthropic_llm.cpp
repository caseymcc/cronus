#include "hermes/providers/anthropic_llm.h"

namespace hermes
{

ErrorCode AnthropicLLM::completion(const CompletionRequest &request,
    CompletionResponse &response)
{
    std::string api_key;
    if(request.api_key.has_value())
    {
        api_key=request.api_key.value();
    }
    else
    {
        auto result=getApiKey("anthropic", api_key);
        if(result!=ErrorCode::Success)
        {
            return result;
        }
    }

    // Implementation using cpr for Anthropic API
    // TODO: Implement actual API call
    response.provider="anthropic";
    return ErrorCode::Success;
}

} // namespace hermes
