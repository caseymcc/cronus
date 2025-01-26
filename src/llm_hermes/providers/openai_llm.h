#ifndef _llm_hermes_providers_openai_llm_h_
#define _llm_hermes_providers_openai_llm_h_

#include "llm_hermes/providers/base_llm.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

namespace llm_hermes {

class OpenAILLM : public BaseLLM {
public:
    OpenAILLM();
    ErrorCode completion(const CompletionRequest& request,
                        CompletionResponse& response) override;

private:
    static constexpr const char* API_URL = "https://api.openai.com/v1/chat/completions";
    
    ErrorCode parseResponse(const cpr::Response& rawResponse,
                          CompletionResponse& response);
    
    nlohmann::json createRequestBody(const CompletionRequest& request);
    cpr::Header createHeaders(const std::string& apiKey);
};

} // namespace llm_hermes

#endif//_llm_hermes_providers_openai_llm_h_
