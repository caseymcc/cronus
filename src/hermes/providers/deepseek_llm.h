#ifndef _hermes_providers_deepseek_llm_h_
#define _hermes_providers_deepseek_llm_h_

#include "hermes/providers/base_llm.h"
#include "hermes/model_manager.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

namespace hermes
{

class DeepseekLLM : public BaseLLM
{
public:
    DeepseekLLM(ModelInfo &modelInfo) : m_modelInfo(modelInfo) {};
    
    ErrorCode completion(const CompletionRequest &request,
        CompletionResponse &response) override;
        
    ErrorCode streamingCompletion(const CompletionRequest &request,
        std::function<void(const std::string&)> callback) override;

private:
    ErrorCode parseResponse(const cpr::Response &rawResponse,
        CompletionResponse &response);

    nlohmann::json createRequestBody(const CompletionRequest &request);
    cpr::Header createHeaders();

    ModelInfo m_modelInfo;

    std::string m_apiUrl="https://api.deepseek.com/chat/completions";
    std::string m_apiKey="";
};

} // namespace hermes

#endif//_hermes_providers_deepseek_llm_h_
