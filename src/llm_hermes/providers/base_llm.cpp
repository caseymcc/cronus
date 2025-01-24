#include "llm_hermes/providers/base_llm.h"
#include <cstdlib>

namespace llm_hermes
{

ErrorCode BaseLLM::get_api_key(const std::string &provider, std::string &api_key)
{
    if(provider=="openai")
    {
        if(auto key=std::getenv("OPENAI_API_KEY"))
        {
            api_key=key;
            return ErrorCode::Success;
        }
    }
    else if(provider=="anthropic")
    {
        if(auto key=std::getenv("ANTHROPIC_API_KEY"))
        {
            api_key=key;
            return ErrorCode::Success;
        }
    }
    return ErrorCode::ApiKeyNotFound;
}

} // namespace llm_hermes
