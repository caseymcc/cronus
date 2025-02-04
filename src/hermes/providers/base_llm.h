#ifndef _hermes_providers_base_llm_h_
#define _hermes_providers_base_llm_h_

#include "hermes/hermes.h"

namespace hermes
{

class BaseLLM
{
public:
    virtual ~BaseLLM()=default;

    virtual ErrorCode completion(const CompletionRequest &request,
        CompletionResponse &response)=0;
    
    virtual ErrorCode streamingCompletion(const CompletionRequest &request,
        std::function<void(const std::string&)> callback)=0;

protected:
    ErrorCode getApiKey(const std::string &provider, std::string &apiKey);
};

} // namespace hermes

#endif//_hermes_providers_base_llm_h_
