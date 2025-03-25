#include "cronus/utils/promptManager.h"
#include "cronus/logger.h"

#include <fstream>

namespace cronus
{
namespace utils
{

PromptManager& PromptManager::instance()
{
    static PromptManager instance;
    return instance;
}

bool PromptManager::initialize(const std::vector<std::filesystem::path>& configPaths)
{
    // Clear existing data
    m_prompts.clear();
    
    bool anyLoaded = false;
    
    // Process directories in order, allowing later ones to override earlier ones
    for (const auto& configPath : configPaths) {
        auto promptsPath = configPath / "prompts";
        
        if (!std::filesystem::exists(promptsPath)) {
            continue;
        }

        // Iterate through all JSON files in the prompts directory
        for (const auto& entry : std::filesystem::directory_iterator(promptsPath)) {
            if (entry.path().extension() != ".json") {
                continue;
            }

            if (loadPromptsFile(entry.path())) {
                anyLoaded = true;
            }
        }
    }

    m_initialized = anyLoaded;
    
    if (m_initialized) {
        logInfo("PromptManager initialized successfully");
    } else {
        logWarning("PromptManager initialized but no prompt files were loaded");
    }
    
    return anyLoaded;
}

bool PromptManager::loadPromptsFile(const std::filesystem::path& filePath)
{
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            logError("Failed to open prompt file: " + filePath.string());
            return false;
        }
        
        nlohmann::json config = nlohmann::json::parse(file);

        if (!config.contains("prompts") || !config["prompts"].is_object()) {
            logError("Invalid prompt file format: " + filePath.string());
            return false;
        }

        // Process each agent's prompts
        for (const auto& [agentName, agentPrompts] : config["prompts"].items()) {
            if (!agentPrompts.is_object()) {
                continue;
            }
            
            // Process each prompt for this agent
            for (const auto& [promptName, promptVariants] : agentPrompts.items()) {
                if (!promptVariants.is_object()) {
                    continue;
                }
                
                // Get or create the prompt variants
                auto& variants = m_prompts[agentName][promptName];
                
                // Process default prompt
                if (promptVariants.contains("default") && promptVariants["default"].is_string()) {
                    variants.defaultPrompt = promptVariants["default"].get<std::string>();
                }
                
                // Process model-specific prompts
                if (promptVariants.contains("models") && promptVariants["models"].is_object()) {
                    for (const auto& [modelName, modelPrompt] : promptVariants["models"].items()) {
                        if (modelPrompt.is_string()) {
                            variants.modelSpecific[modelName] = modelPrompt.get<std::string>();
                        }
                    }
                }
                
                // Process provider-specific prompts
                if (promptVariants.contains("providers") && promptVariants["providers"].is_object()) {
                    for (const auto& [providerName, providerPrompt] : promptVariants["providers"].items()) {
                        if (providerPrompt.is_string()) {
                            variants.providerSpecific[providerName] = providerPrompt.get<std::string>();
                        }
                    }
                }
                
                // Process model-provider-specific prompts
                if (promptVariants.contains("model_providers") && promptVariants["model_providers"].is_object()) {
                    for (const auto& [modelName, providers] : promptVariants["model_providers"].items()) {
                        if (providers.is_object()) {
                            for (const auto& [providerName, prompt] : providers.items()) {
                                if (prompt.is_string()) {
                                    variants.modelProviderSpecific[modelName][providerName] = prompt.get<std::string>();
                                }
                            }
                        }
                    }
                }
            }
        }

        logInfo("Loaded prompts from: " + filePath.string());
        return true;
    }
    catch (const std::exception& e) {
        logError("Error loading prompt file " + filePath.string() + ": " + e.what());
        return false;
    }
}

std::optional<std::string> PromptManager::getPrompt(
    const std::string& agentName,
    const std::string& promptName,
    const std::string& modelName,
    const std::string& providerName) const
{
    // Check if we have prompts for this agent
    auto agentIt = m_prompts.find(agentName);
    if (agentIt == m_prompts.end()) {
        return std::nullopt;
    }
    
    // Check if we have this specific prompt
    auto promptIt = agentIt->second.find(promptName);
    if (promptIt == agentIt->second.end()) {
        return std::nullopt;
    }
    
    const auto& variants = promptIt->second;
    
    // Try to find the most specific prompt first
    
    // 1. Check for model+provider specific prompt
    if (!modelName.empty() && !providerName.empty()) {
        auto modelIt = variants.modelProviderSpecific.find(modelName);
        if (modelIt != variants.modelProviderSpecific.end()) {
            auto providerIt = modelIt->second.find(providerName);
            if (providerIt != modelIt->second.end()) {
                return providerIt->second;
            }
        }
    }
    
    // 2. Check for model-specific prompt
    if (!modelName.empty()) {
        auto modelIt = variants.modelSpecific.find(modelName);
        if (modelIt != variants.modelSpecific.end()) {
            return modelIt->second;
        }
    }
    
    // 3. Check for provider-specific prompt
    if (!providerName.empty()) {
        auto providerIt = variants.providerSpecific.find(providerName);
        if (providerIt != variants.providerSpecific.end()) {
            return providerIt->second;
        }
    }
    
    // 4. Fall back to default prompt
    if (!variants.defaultPrompt.empty()) {
        return variants.defaultPrompt;
    }
    
    return std::nullopt;
}

} // namespace utils
} // namespace cronus
