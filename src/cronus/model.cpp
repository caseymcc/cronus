#include "cronus/model.h"
#include "cronus/config.h"
#include "cronus/logger.h"

namespace cronus {

Model::Model() {
    loadModelConfig();
}

void Model::loadModelConfig() {
    const auto& config = Config::instance();
    m_currentModel = config.getModel();
    m_currentProvider = config.getProvider();
    
    auto modelConfig = config.getModelConfig(m_currentModel);
    if (modelConfig) {
        m_maxTokens = modelConfig->max_input_tokens > 0 ? 
                      modelConfig->max_input_tokens : 4096;
        logInfo("Model initialized: " + m_currentModel + 
                " with max tokens: " + std::to_string(m_maxTokens));
    } else {
        m_maxTokens = 4096; // Default fallback
        logWarning("Model config not found for: " + m_currentModel + 
                   ". Using default max tokens: " + std::to_string(m_maxTokens));
    }
}

std::string Model::generate(
    const std::vector<hermes::Message>& messages,
    bool streaming,
    std::function<void(const std::string&)> callback) 
{
    const auto& config = Config::instance();
    auto modelConfig = config.getModelConfig(m_currentModel);
    
    if (!modelConfig) {
        logError("Model configuration not found for: " + m_currentModel);
        return "Error: Model configuration not found.";
    }
    
    hermes::CompletionRequest request{
        .model = modelConfig->model,
        .messages = messages
    };
    
    if (streaming && callback) {
        hermes::ErrorCode result = hermes::streamingCompletion(request, callback);
        
        if (result != hermes::ErrorCode::Success) {
            std::string errorMsg = m_currentModel + " streaming completion failed with error code: " +
                std::to_string(static_cast<int>(result));
            logError(errorMsg);
            return "Error: " + errorMsg;
        }
        
        return ""; // Response handled by callback in streaming mode
    } else {
        hermes::CompletionResponse response;
        hermes::ErrorCode result = hermes::completion(request, response);
        
        if (result != hermes::ErrorCode::Success) {
            std::string errorMsg = m_currentModel + " completion failed with error code: " +
                std::to_string(static_cast<int>(result));
            logError(errorMsg);
            return "Error: " + errorMsg;
        }
        
        return response.text;
    }
}

size_t Model::getMaxTokens() const {
    return m_maxTokens;
}

std::string Model::getModelName() const {
    return m_currentModel;
}

} // namespace cronus
