#include "llm_hermes/hermes.h"
#include "llm_hermes/providers/openai_llm.h"
#include "llm_hermes/providers/anthropic_llm.h"

#include <memory>

namespace llm_hermes {

ErrorCode completion(const CompletionRequest& request, CompletionResponse& response) {
    auto it = MODEL_PROVIDER_MAP.find(request.model);
    if (it == MODEL_PROVIDER_MAP.end()) {
        return ErrorCode::UnknownModel;
    }

    const std::string& provider = it->second;
    std::unique_ptr<BaseLLM> llm;
    
    if (provider == "openai") {
        llm = std::make_unique<OpenAILLM>();
    }
    else if (provider == "anthropic") {
        llm = std::make_unique<AnthropicLLM>();
    }
    else {
        return ErrorCode::UnsupportedProvider;
    }
    
    return llm->completion(request, response);
}

} // namespace llm_hermes
