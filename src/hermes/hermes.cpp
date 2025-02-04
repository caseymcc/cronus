#include "hermes/hermes.h"
#include "hermes/model_manager.h"
#include "hermes/providers/openai_llm.h"
#include "hermes/providers/anthropic_llm.h"
#include "hermes/providers/deepseek_llm.h"

#include <memory>

namespace hermes
{

struct Hermes
{
    static Hermes &instance()
    {
        static Hermes instance;
        return instance;
    }

    bool initialized=false;
};

ErrorCode initialize(const std::vector<std::filesystem::path> &configPaths)
{
    if(!ModelManager::instance().initialize(configPaths))
    {
        return ErrorCode::InvalidRequest;
    }

    Hermes::instance().initialized=true;
    return ErrorCode::Success;
}

bool doesModelNeedApiKey(const std::string &model)
{
    auto provider=ModelManager::instance().getProvider(model);
 
    if(!provider)
    {
        return false;
    }

    return *provider=="openai";
}

ErrorCode completion(const CompletionRequest &request, CompletionResponse &response)
{
    if(!Hermes::instance().initialized)
    {
        return ErrorCode::InvalidRequest;
    }

//    auto provider=ModelManager::instance().getProvider(request.model);
//    if(!provider)
//    {
//        return ErrorCode::UnknownModel;
//    }
    std::optional<ModelInfo> modelInfo=ModelManager::instance().getModelInfo(request.model);

    if(!modelInfo)
    {
        return ErrorCode::UnknownModel;
    }

    std::unique_ptr<BaseLLM> llm;

    if(modelInfo->provider=="openai")
    {
        llm=std::make_unique<OpenAILLM>(*modelInfo);
    }
    else if(modelInfo->provider=="anthropic")
    {
        llm=std::make_unique<AnthropicLLM>(*modelInfo);
    }
    else if(modelInfo->provider=="deepseek")
    {
        llm=std::make_unique<DeepseekLLM>(*modelInfo);
    }
    else
    {
        return ErrorCode::UnsupportedProvider;
    }

    return llm->completion(request, response);
}

} // namespace hermes
