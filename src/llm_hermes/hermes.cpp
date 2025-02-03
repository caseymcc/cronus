#include "llm_hermes/hermes.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

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
    std::filesystem::path modelsPath = std::filesystem::path(configPath) / "models";
    
    if (!std::filesystem::exists(modelsPath)) {
        return ErrorCode::InvalidRequest;
    }

    // Clear existing mappings
    MODEL_PROVIDER_MAP.clear();

    // Iterate through all JSON files in the models directory
    for (const auto &entry : std::filesystem::directory_iterator(modelsPath)) {
        if (entry.path().extension() != ".json") {
            continue;
        }

        try {
            std::ifstream file(entry.path());
            nlohmann::json modelConfig = nlohmann::json::parse(file);

            // Each JSON file can contain multiple model mappings
            for (const auto &[model, provider] : modelConfig.items()) {
                if (provider.is_string()) {
                    MODEL_PROVIDER_MAP[model] = provider.get<std::string>();
                }
            }
        }
        catch (const std::exception &) {
            // Skip invalid files but continue processing others
            continue;
        }
    }

    llm_hermes_initialized=true;
    return ErrorCode::Success;
}

ErrorCode completion(const CompletionRequest &request, CompletionResponse &response)
{
    if(!llm_hermes_initialized)
    {
        return ErrorCode::InvalidRequest;
    }

    auto it=MODEL_PROVIDER_MAP.find(request.model);
    if(it==MODEL_PROVIDER_MAP.end())
    {
        return ErrorCode::UnknownModel;
    }

    const std::string &provider=it->second;
    std::unique_ptr<BaseLLM> llm;

    if(provider=="openai")
    {
        llm=std::make_unique<OpenAILLM>();
    }
    else if(provider=="anthropic")
    {
        llm=std::make_unique<AnthropicLLM>();
    }
    else if(provider=="deepseek")
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
