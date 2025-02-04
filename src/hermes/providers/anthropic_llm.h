#ifndef _hermes_providers_anthropic_llm_h_
#define _hermes_providers_anthropic_llm_h_

#include "hermes/providers/base_llm.h"
#include "hermes/model_manager.h"

namespace hermes
{

class AnthropicLLM : public BaseLLM
{
public:
    AnthropicLLM(ModelInfo& modelInfo) : m_modelInfo(modelInfo) {};

    ErrorCode completion(const CompletionRequest& request,
                        CompletionResponse& response) override;

private:
    ModelInfo m_modelInfo;
};

} // namespace hermes

#endif//_hermes_providers_anthropic_llm_h_
