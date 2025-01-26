#ifndef _llm_hermes_providers_base_llm_h_
#define _llm_hermes_providers_base_llm_h_

#include "llm_hermes/hermes.h"

namespace llm_hermes
{

class BaseLLM
{
public:
    virtual ~BaseLLM()=default;

    virtual ErrorCode completion(const CompletionRequest &request,
        CompletionResponse &response)=0;

protected:
    ErrorCode getApiKey(const std::string &provider, std::string &apiKey);
};

} // namespace llm_hermes

#endif//_llm_hermes_providers_base_llm_h_
