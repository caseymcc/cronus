#include "llm_hermes/model_manager.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace llm_hermes
{

ModelManager& ModelManager::instance()
{
    static ModelManager instance;
    return instance;
}

bool ModelManager::initialize(const std::vector<std::filesystem::path>& configPaths)
{
    // Clear existing data
    m_models.clear();
    m_modelProviderMap.clear();

    bool anyLoaded = false;
    
    // Process directories in order, allowing later ones to override earlier ones
    for (const auto& configPath : configPaths) {
        auto modelsPath = configPath / "models";
        
        if (!std::filesystem::exists(modelsPath)) {
            continue;
        }

        // Iterate through all JSON files in the models directory
        for (const auto& entry : std::filesystem::directory_iterator(modelsPath)) {
            if (entry.path().extension() != ".json") {
                continue;
            }

            if (loadModelFile(entry.path())) {
                anyLoaded = true;
            }
        }
    }

    m_initialized = anyLoaded;
    return anyLoaded;
}

bool ModelManager::loadModelFile(const std::filesystem::path& filePath)
{
    try {
        std::ifstream file(filePath);
        nlohmann::json config = nlohmann::json::parse(file);

        if (!config.contains("models") || !config["models"].is_array()) {
            return false;
        }

        for (const auto& modelJson : config["models"]) {
            ModelInfo info;
            
            // Required fields
            if (!modelJson.contains("model") || !modelJson.contains("litellm_provider")) {
                continue;
            }
            
            info.model = modelJson["model"].get<std::string>();
            info.provider = modelJson["litellm_provider"].get<std::string>();
            
            // Optional fields
            if (modelJson.contains("mode")) {
                info.mode = modelJson["mode"].get<std::string>();
            }
            if (modelJson.contains("api_base")) {
                info.api_base = modelJson["api_base"].get<std::string>();
            }
            if (modelJson.contains("examples_as_sys_msg")) {
                info.examples_as_sys_msg = modelJson["examples_as_sys_msg"].get<bool>();
            }
            if (modelJson.contains("context_window")) {
                info.context_window = modelJson["context_window"].get<int>();
            }
            if (modelJson.contains("max_tokens")) {
                info.max_tokens = modelJson["max_tokens"].get<int>();
            }
            if (modelJson.contains("max_input_tokens")) {
                info.max_input_tokens = modelJson["max_input_tokens"].get<int>();
            }
            if (modelJson.contains("max_output_tokens")) {
                info.max_output_tokens = modelJson["max_output_tokens"].get<int>();
            }
            if (modelJson.contains("input_cost_per_token")) {
                info.input_cost_per_token = modelJson["input_cost_per_token"].get<double>();
            }
            if (modelJson.contains("output_cost_per_token")) {
                info.output_cost_per_token = modelJson["output_cost_per_token"].get<double>();
            }

            // Find existing model to update
            auto it = std::find_if(m_models.begin(), m_models.end(),
                [&info](const ModelInfo& existing) { return existing.model == info.model; });
            
            if (it != m_models.end()) {
                // Update existing model settings
                if (modelJson.contains("mode")) {
                    it->mode = info.mode;
                }
                if (modelJson.contains("api_base")) {
                    it->api_base = info.api_base;
                }
                if (modelJson.contains("examples_as_sys_msg")) {
                    it->examples_as_sys_msg = info.examples_as_sys_msg;
                }
                if (modelJson.contains("context_window")) {
                    it->context_window = info.context_window;
                }
                if (modelJson.contains("max_tokens")) {
                    it->max_tokens = info.max_tokens;
                }
                if (modelJson.contains("max_input_tokens")) {
                    it->max_input_tokens = info.max_input_tokens;
                }
                if (modelJson.contains("max_output_tokens")) {
                    it->max_output_tokens = info.max_output_tokens;
                }
                if (modelJson.contains("input_cost_per_token")) {
                    it->input_cost_per_token = info.input_cost_per_token;
                }
                if (modelJson.contains("output_cost_per_token")) {
                    it->output_cost_per_token = info.output_cost_per_token;
                }
            } else {
                // Add new model
                m_models.push_back(info);
            }
            m_modelProviderMap[info.model] = info.provider;
        }

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

std::optional<std::string> ModelManager::getProvider(const std::string& model) const
{
    auto it = m_modelProviderMap.find(model);
    if (it != m_modelProviderMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<ModelInfo> ModelManager::getModelInfo(const std::string& model) const
{
    auto it = std::find_if(m_models.begin(), m_models.end(),
        [&model](const ModelInfo& info) { return info.model == model; });
    
    if (it != m_models.end()) {
        return *it;
    }
    return std::nullopt;
}

} // namespace llm_hermes
