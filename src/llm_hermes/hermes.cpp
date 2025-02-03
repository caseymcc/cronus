#include "llm_hermes/hermes.h"
#include "llm_hermes/model_manager.h"
#include "llm_hermes/providers/openai_llm.h"
#include "llm_hermes/providers/anthropic_llm.h"
#include "llm_hermes/providers/deepseek_llm.h"

#include <memory>

namespace llm_hermes
{

struct Hermes
{
    Hermes &instance()
    {
        static Hermes instance;
        return instance;
    }

    bool llm_hermes_initialized=false;
};

ErrorCode initialize(const std::string &configPath)
{
    if (!ModelManager::instance().initialize(configPath)) {
        return ErrorCode::InvalidRequest;
    }
    return ErrorCode::Success;
}

ErrorCode completion(const CompletionRequest &request, CompletionResponse &response)
{
    if(!llm_hermes_initialized)
    {
        return ErrorCode::InvalidRequest;
    }

    auto provider = ModelManager::instance().getProvider(request.model);
    if (!provider) {
        return ErrorCode::UnknownModel;
    }
    std::unique_ptr<BaseLLM> llm;

    if(*provider=="openai")
    {
        llm=std::make_unique<OpenAILLM>();
    }
    else if(*provider=="anthropic")
    {
        llm=std::make_unique<AnthropicLLM>();
    }
    else if(*provider=="deepseek")
    {
        llm=std::make_unique<DeepseekLLM>();
    }
    else
    {
        return ErrorCode::UnsupportedProvider;
    }

    return llm->completion(request, response);
}

} // namespace llm_hermes
