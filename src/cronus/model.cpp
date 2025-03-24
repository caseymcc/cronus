#include "cronus/model.h"
#include "cronus/config.h"
#include "cronus/logger.h"

#include "hermes/hermes.h"
#include "hermes/modelManager.h"

namespace cronus
{

Model::Model(const std::string &modelName, const std::string &provider)
    : m_currentModel(""), m_currentProvider(""), m_maxTokens(0), 
      m_maxInputTokens(0), m_maxOutputTokens(0)
{
    setModel(modelName, provider);
}

bool Model::setModel(const std::string &modelName, const std::string &provider)
{
    // Get model info from ModelManager
    auto &modelManager = hermes::ModelManager::instance();
    auto modelInfo = modelManager.getModelInfo(modelName);
    
    if (!modelInfo) {
        logError("Model not found: " + modelName);
        return false;
    }
    
    m_currentModel = modelName;
    m_currentProvider = provider;
    
    // Set token limits based on model info
    m_maxInputTokens = modelInfo->maxInputTokens;
    m_maxOutputTokens = modelInfo->maxOutputTokens;
    m_maxTokens = modelInfo->contextWindow;
    
    logInfo("Model changed to: " + m_currentModel + 
            " with max tokens: " + std::to_string(m_maxTokens) +
            " (input: " + std::to_string(m_maxInputTokens) + 
            ", output: " + std::to_string(m_maxOutputTokens) + ")");
    
    return true;
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
