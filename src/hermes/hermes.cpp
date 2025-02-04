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
    std::map<std::string, std::unique_ptr<BaseLLM>> llms;
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

std::unique_ptr<BaseLLM> createLLM(const ModelInfo& modelInfo) {
    if(modelInfo.provider=="openai")
    {
        return std::make_unique<OpenAILLM>(modelInfo);
    }
    else if(modelInfo.provider=="anthropic")
    {
        return std::make_unique<AnthropicLLM>(modelInfo);
    }
    else if(modelInfo.provider=="deepseek")
    {
        return std::make_unique<DeepseekLLM>(modelInfo);
    }
    return nullptr;
}

BaseLLM &getLLM(const CompletionRequest &request)
{
    auto& hermes = Hermes::instance();
    
    // Check if we already have an LLM instance for this model
    auto it = hermes.llms.find(request.model);
    if (it == hermes.llms.end()) {
        // Create new LLM instance
        auto llm = createLLM(*modelInfo);
        if (!llm) {
            return ErrorCode::UnsupportedProvider;
        }
        it = hermes.llms.emplace(request.model, std::move(llm)).first;
    }

    return *(it->second);
}

ErrorCode completion(const CompletionRequest &request, CompletionResponse &response)
{
    if(!Hermes::instance().initialized)
    {
        return ErrorCode::InvalidRequest;
    }

    std::optional<ModelInfo> modelInfo=ModelManager::instance().getModelInfo(request.model);
    if(!modelInfo)
    {
        return ErrorCode::UnknownModel;
    }

    BaseLLM &llm=getLLM(request);

    return llm.second->completion(request, response);
}

ErrorCode streamingCompletion(const CompletionRequest &request,
    std::function<void(const std::string&)> callback)
{
    if(!Hermes::instance().initialized)
    {
        return ErrorCode::InvalidRequest;
    }

    std::optional<ModelInfo> modelInfo=ModelManager::instance().getModelInfo(request.model);
    if(!modelInfo)
    {
        return ErrorCode::UnknownModel;
    }

    BaseLLM &llm=getLLM(request);

    return llm.second->streamingCompletion(request, callback);
}

} // namespace hermes
