#include "cronus/agents/promptManager.h"
#include "cronus/logger.h"

#include <fstream>
#include <algorithm>

namespace cronus
{
namespace agents
{

PromptManager &PromptManager::instance()
{
    static PromptManager instance;
    return instance;
}

bool PromptManager::initialize(const std::vector<std::filesystem::path> &configPaths)
{
    // Clear existing data
    m_promptConfigs.clear();

    bool anyLoaded=false;

    // Process directories in order, allowing later ones to override earlier ones
    for(const auto &configPath:configPaths)
    {
        auto promptsPath=configPath/"prompts";

        if(!std::filesystem::exists(promptsPath))
        {
            continue;
        }

        // Iterate through all JSON files in the prompts directory
        for(const auto &entry:std::filesystem::directory_iterator(promptsPath))
        {
            if(entry.path().extension()!=".json")
            {
                continue;
            }

            if(loadPromptsFile(entry.path()))
            {
                anyLoaded=true;
            }
        }
    }

    m_initialized=anyLoaded;

    if(m_initialized)
    {
        logInfo("PromptManager initialized successfully");
    }
    else
    {
        logWarning("PromptManager initialized but no prompt files were loaded");
    }

    return anyLoaded;
}

bool PromptManager::loadPromptsFile(const std::filesystem::path &filePath)
{
    try
    {
        std::ifstream file(filePath);
        if(!file.is_open())
        {
            logError("Failed to open prompt file: "+filePath.string());
            return false;
        }

        nlohmann::json config=nlohmann::json::parse(file);

        if(!config.contains("prompts"))
        {
            logError("Invalid prompt file format (missing 'prompts' key): "+filePath.string());
            return false;
        }

        if(!config["prompts"].is_array())
        {
            logError("Invalid prompt file format (prompts is neither array nor object): "+filePath.string());
            return false;
        }

        for(const auto &promptConfig:config["prompts"])
        {
            PromptConfig newConfig;

            // Get models list
            if(promptConfig.contains("models")&&promptConfig["models"].is_array())
            {
                for(const auto &model:promptConfig["models"])
                {
                    if(model.is_string())
                    {
                        newConfig.models.push_back(model.get<std::string>());
                    }
                }
            }

            // Get providers list
            if(promptConfig.contains("providers")&&promptConfig["providers"].is_array())
            {
                for(const auto &provider:promptConfig["providers"])
                {
                    if(provider.is_string())
                    {
                        newConfig.providers.push_back(provider.get<std::string>());
                    }
                }
            }

            // Process each agent's prompts
            for(const auto &[agentName, agentPrompts]:promptConfig.items())
            {
                // Skip the models and providers keys
                if(agentName=="models"||agentName=="providers")
                {
                    continue;
                }

                if(!agentPrompts.is_object())
                {
                    continue;
                }

                // Process each prompt for this agent
                for(const auto &[promptName, promptText]:agentPrompts.items())
                {
                    if(promptText.is_string())
                    {
                        newConfig.agentPrompts[agentName][promptName]=promptText.get<std::string>();
                    }
                }
            }

            // Add the config if it has any prompts
            if(!newConfig.agentPrompts.empty())
            {
                m_promptConfigs.push_back(newConfig);
            }
        }
        logInfo("Loaded prompts from: "+filePath.string());
        return true;
    }
    catch(const std::exception &e)
    {
        logError("Error loading prompt file "+filePath.string()+": "+e.what());
        return false;
    }
}

std::optional<std::string> PromptManager::getPrompt(
    const std::string &agentName,
    const std::string &promptName,
    const std::string &modelName,
    const std::string &providerName) const
{
    // First, try to find a prompt specific to both model and provider
    if(!modelName.empty()&&!providerName.empty())
    {
        for(const auto &config:m_promptConfigs)
        {
            // Check if this config applies to this model and provider
            bool modelMatches=std::find(config.models.begin(), config.models.end(), modelName)!=config.models.end();
            bool providerMatches=std::find(config.providers.begin(), config.providers.end(), providerName)!=config.providers.end();

            if(modelMatches&&providerMatches)
            {
                // Check if this config has the requested agent and prompt
                auto agentIt=config.agentPrompts.find(agentName);
                if(agentIt!=config.agentPrompts.end())
                {
                    auto promptIt=agentIt->second.find(promptName);
                    if(promptIt!=agentIt->second.end())
                    {
                        return promptIt->second;
                    }
                }
            }
        }
    }

    // Next, try to find a model-specific prompt
    if(!modelName.empty())
    {
        for(const auto &config:m_promptConfigs)
        {
            // Check if this config applies to this model
            bool modelMatches=std::find(config.models.begin(), config.models.end(), modelName)!=config.models.end();
            bool noProviders=config.providers.empty();

            if(modelMatches&&noProviders)
            {
                // Check if this config has the requested agent and prompt
                auto agentIt=config.agentPrompts.find(agentName);
                if(agentIt!=config.agentPrompts.end())
                {
                    auto promptIt=agentIt->second.find(promptName);
                    if(promptIt!=agentIt->second.end())
                    {
                        return promptIt->second;
                    }
                }
            }
        }
    }

    // Next, try to find a provider-specific prompt
    if(!providerName.empty())
    {
        for(const auto &config:m_promptConfigs)
        {
            // Check if this config applies to this provider
            bool providerMatches=std::find(config.providers.begin(), config.providers.end(), providerName)!=config.providers.end();
            bool noModels=config.models.empty();

            if(providerMatches&&noModels)
            {
                // Check if this config has the requested agent and prompt
                auto agentIt=config.agentPrompts.find(agentName);
                if(agentIt!=config.agentPrompts.end())
                {
                    auto promptIt=agentIt->second.find(promptName);
                    if(promptIt!=agentIt->second.end())
                    {
                        return promptIt->second;
                    }
                }
            }
        }
    }

    // Finally, try to find a default prompt
    for(const auto &config:m_promptConfigs)
    {
        // Check if this is a default config
        bool isDefault=std::find(config.models.begin(), config.models.end(), "default")!=config.models.end();

        if(isDefault)
        {
            // Check if this config has the requested agent and prompt
            auto agentIt=config.agentPrompts.find(agentName);
            if(agentIt!=config.agentPrompts.end())
            {
                auto promptIt=agentIt->second.find(promptName);
                if(promptIt!=agentIt->second.end())
                {
                    return promptIt->second;
                }
            }
        }
    }

    return std::nullopt;
}

} // namespace agents
} // namespace cronus
