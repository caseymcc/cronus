#ifndef _llm_hermes_providers_openai_llm_h_
#define _llm_hermes_providers_openai_llm_h_

#include "llm_hermes/providers/base_llm.h"

namespace llm_hermes
{

class OpenAILLM : public BaseLLM
{
public:
    ErrorCode completion(const CompletionRequest &request,
        CompletionResponse &response) override;
};

} // namespace llm_hermes

#endif//_llm_hermes_providers_openai_llm_h_
