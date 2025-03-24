#include "cronus/model.h"
#include "cronus/config.h"
#include "cronus/logger.h"

namespace cronus
{

Model::Model(const std::string& modelName, const std::string& provider, size_t maxTokens)
    : m_currentModel(modelName), m_currentProvider(provider), m_maxTokens(maxTokens)
{
    logInfo("Model initialized: " + m_currentModel + 
            " with max tokens: " + std::to_string(m_maxTokens));
}

std::string Model::generate(
    const std::vector<hermes::Message> &messages,
    bool streaming,
    std::function<void(const std::string &)> callback)
{
    const auto &config=Config::instance();
    auto modelConfig=config.getModelConfig(m_currentModel);

    if(!modelConfig)
    {
        logError("Model configuration not found for: "+m_currentModel);
        return "Error: Model configuration not found.";
    }

    hermes::CompletionRequest request{
        .model=m_currentModel,
        .messages=messages
    };

    if(streaming&&callback)
    {
        hermes::ErrorCode result=hermes::streamingCompletion(request, callback);

        if(result!=hermes::ErrorCode::Success)
        {
            std::string errorMsg=m_currentModel+" streaming completion failed with error code: "+
                std::to_string(static_cast<int>(result));
            logError(errorMsg);
            return "Error: "+errorMsg;
        }

        return ""; // Response handled by callback in streaming mode
    }
    else
    {
        hermes::CompletionResponse response;
        hermes::ErrorCode result=hermes::completion(request, response);

        if(result!=hermes::ErrorCode::Success)
        {
            std::string errorMsg=m_currentModel+" completion failed with error code: "+
                std::to_string(static_cast<int>(result));
            logError(errorMsg);
            return "Error: "+errorMsg;
        }

        return response.text;
    }
}

size_t Model::getMaxTokens() const
{
    return m_maxTokens;
}

std::string Model::getModelName() const
{
    return m_currentModel;
}

} // namespace cronus
