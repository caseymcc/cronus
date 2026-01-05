#define YAML_CPP_NOEXCEPT 1

#include "cronus/config.h"
#include "cronus/logger.h"
#include "cronus/modeDetector.h"
#include "cronus/directoryInitializer.h"

#include "loreforge/loreforge.h"

#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <fstream>

namespace cronus
{

Config &Config::instance()
{
    static Config instance;
    return instance;    
}

void Config::setModelAndProvider(const std::string &combined)
{
    auto pos = combined.find('/');
    if (pos != std::string::npos)
    {
        m_provider = combined.substr(0, pos);
        m_model = combined.substr(pos + 1);
    }
    else
    {
        m_provider = "";
        m_model = combined;
    }
}

void Config::loadFromEnv()
{
    if(const char *model=std::getenv("CRONUS_MODEL"))
    {
        setModelAndProvider(model);
    }
    if(const char *openaiKey=std::getenv("OPENAI_API_KEY"))
    {
        m_apiKeys["openai"]=openaiKey;
    }
    if(const char *anthropicKey=std::getenv("ANTHROPIC_API_KEY"))
    {
        m_apiKeys["anthropic"]=anthropicKey;
    }
}

void Config::loadFromFile(const std::filesystem::path &configPath)
{
    if(!std::filesystem::exists(configPath))
    {
        logWarning("Config file does not exist: "+configPath.string());
        return;
    }

    YAML::Node config=YAML::LoadFile(configPath.string());
    if(config.IsNull())
    {
        logWarning("Failed to parse config file: "+configPath.string());
        return;
    }

    if(!config.IsDefined())
    {
        logWarning("Error in config file: "+configPath.string());
        return;
    }

    
    if(config["model"])
    {
        std::string modelName = config["model"].as<std::string>();
        // Check if this is a model name in our configurations
        auto modelConfig = getModelConfig(modelName);

        if(modelConfig)
            setModelAndProvider(modelConfig->model);
        else
            setModelAndProvider(modelName);
    }

    if(config["api_keys"]||config["api-keys"])
    {
        auto keys=config["api_keys"].IsDefined()?config["api_keys"]:config["api-keys"];

        if(keys["openai"]&&keys["openai"].IsScalar())
        {
            m_apiKeys["openai"]=keys["openai"].as<std::string>();
        }
        if(keys["anthropic"]&&keys["anthropic"].IsScalar())
        {
            m_apiKeys["anthropic"]=keys["anthropic"].as<std::string>();
        }
    }
}

void Config::loadFromJsonFile(const std::filesystem::path &configPath)
{
    if(!std::filesystem::exists(configPath))
    {
        logWarning("JSON config file does not exist: "+configPath.string());
        return;
    }

    try
    {
        std::ifstream file(configPath);
        if(!file.is_open())
        {
            logWarning("Failed to open JSON config file: "+configPath.string());
            return;
        }

        nlohmann::json config;
        file >> config;

        // Load mode if specified
        if(config.contains("mode"))
        {
            std::string modeStr = config["mode"].get<std::string>();
            auto mode = stringToOperationalMode(modeStr);
            if(mode.has_value())
            {
                m_operationalMode = mode.value();
            }
        }

        // Load model configuration
        if(config.contains("model"))
        {
            if(config["model"].is_string())
            {
                setModelAndProvider(config["model"].get<std::string>());
            }
            else if(config["model"].is_object())
            {
                auto modelObj = config["model"];
                if(modelObj.contains("name"))
                {
                    std::string modelName = modelObj["name"].get<std::string>();
                    if(modelObj.contains("provider"))
                    {
                        std::string provider = modelObj["provider"].get<std::string>();
                        setModelAndProvider(provider + "/" + modelName);
                    }
                    else
                    {
                        setModelAndProvider(modelName);
                    }
                }
            }
        }

        // Load API keys
        if(config.contains("api_keys"))
        {
            auto keys = config["api_keys"];
            if(keys.contains("openai") && keys["openai"].is_string())
            {
                std::string key = keys["openai"].get<std::string>();
                // Expand environment variables
                if(key.find("${") == 0)
                {
                    std::string envVar = key.substr(2, key.length() - 3);
                    const char *envValue = std::getenv(envVar.c_str());
                    if(envValue)
                    {
                        m_apiKeys["openai"] = envValue;
                    }
                }
                else
                {
                    m_apiKeys["openai"] = key;
                }
            }
            if(keys.contains("anthropic") && keys["anthropic"].is_string())
            {
                std::string key = keys["anthropic"].get<std::string>();
                // Expand environment variables
                if(key.find("${") == 0)
                {
                    std::string envVar = key.substr(2, key.length() - 3);
                    const char *envValue = std::getenv(envVar.c_str());
                    if(envValue)
                    {
                        m_apiKeys["anthropic"] = envValue;
                    }
                }
                else
                {
                    m_apiKeys["anthropic"] = key;
                }
            }
        }
    }
    catch(const std::exception &e)
    {
        logError("Error parsing JSON config: " + std::string(e.what()));
    }
}

std::filesystem::path Config::getDefaultModelConfigPath() const
{
#ifdef _WIN32
    if(const char *appdata=std::getenv("APPDATA"))
    {
        return std::filesystem::path(appdata)/"cronus"/"model_config.yml";
    }
#else
    if(const char *xdg_data=std::getenv("XDG_DATA_HOME"))
    {
        return std::filesystem::path(xdg_data)/"cronus"/"model_config.yml";
    }
    if(const char *home=std::getenv("HOME"))
    {
        return std::filesystem::path(home)/".local"/"share"/"cronus"/"model_config.yml";
    }
#endif
    Logger::instance().error("Could not determine default model config path");
    return std::filesystem::path();
}

void Config::loadModelsFromFile(const std::filesystem::path &configPath, bool override)
{
    if(!std::filesystem::exists(configPath))
    {
        logWarning("Model config file does not exist: "+configPath.string());
        return;
    }

    YAML::Node config=YAML::LoadFile(configPath.string());
    if(config.IsNull())
    {
        logWarning("Failed to parse model config file: "+configPath.string());
        return;
    }

    if(!config.IsDefined())
    {
        logWarning("Error in model config file: "+configPath.string());
        return;
    }

    if(!config["model_list"]||!config["model_list"].IsSequence())
    {
        logWarning("Invalid model_list in config: "+configPath.string());
        return;
    }

    for(const auto &model:config["model_list"])
    {
        if(!model["name"]||!model["model"])
        {
            logWarning("Skipping invalid model entry in: "+configPath.string());
            continue;
        }

        ModelConfig modelConfig;

        modelConfig.name=model["name"].as<std::string>();
        modelConfig.model=model["model"].as<std::string>();
        
        // Load streaming if present
        if (model["streaming"]) {
            modelConfig.streaming = model["streaming"].as<bool>();
        }
        
        if(override)
        {
            // Update existing config if present
            auto it=std::find_if(m_modelConfigs.begin(), m_modelConfigs.end(),
                [&](const ModelConfig &cfg) { return cfg.name==modelConfig.name; });

            if(it!=m_modelConfigs.end())
            {
                // Update existing values
                it->model=modelConfig.model;
                continue;
            }
        }
        m_modelConfigs.push_back(modelConfig);
        continue;
    }
}

void Config::loadModelsFromDirectory(const std::filesystem::path &dirPath, bool override)
{
    if(!std::filesystem::exists(dirPath)||!std::filesystem::is_directory(dirPath))
    {
        return;
    }

    for(const auto &entry:std::filesystem::directory_iterator(dirPath))
    {
        if(entry.is_regular_file()&&
            (entry.path().extension()==".yml"||entry.path().extension()==".yaml"))
        {
            loadModelsFromFile(entry.path(), override);
        }
    }
}

void Config::loadModelDefinitions(const std::string &resourcePath)
{
    m_resourceDirectory=resourcePath;
    // First load from resources
    auto resourceModelPath=std::filesystem::path(m_resourceDirectory)/"models";
    loadModelsFromDirectory(resourceModelPath);

    // Then load from user's config directory
    if(const char *home=std::getenv("HOME"))
    {
        auto userConfigPath=std::filesystem::path(home)/".cronus"/"models";
        loadModelsFromDirectory(userConfigPath, true);
    }

    // Finally load from current directory
    std::filesystem::path localConfig=".cronus/models";
    loadModelsFromDirectory(localConfig, true);
}

std::optional<ModelConfig> Config::getModelConfig(const std::string &model_name) const
{
    for(const auto &config:m_modelConfigs)
    {
        if(config.name==model_name)
        {
            return config;
        }
    }
    return std::nullopt;
}

std::vector<std::filesystem::path> Config::getConfigPaths() const
{
    std::vector<std::filesystem::path> configPaths;
    
    // Add resource directory
    configPaths.push_back(std::filesystem::path(m_resourceDirectory));
    
    // Add home directory config
    const char *home = std::getenv("HOME");
    if(home)
    {
        configPaths.push_back(std::filesystem::path(home)/".cronus");
    }
    
    // Add current directory config
    configPaths.push_back(std::filesystem::path(".cronus"));
    
    return configPaths;
}

void Config::load(const std::string &resourceDir)
{
    std::filesystem::path resourcePath=resourceDir;
    std::vector<std::filesystem::path> configPaths;
    const char *home=std::getenv("HOME");

    // Set working directory
    m_workingDirectory = std::filesystem::current_path();

    // Detect operational mode
    ModeDetectionResult detectionResult = ModeDetector::detect(m_workingDirectory);
    m_operationalMode = detectionResult.mode;

    logInfo("Operational mode detected: " + std::string(operationalModeToString(m_operationalMode)));
    logInfo("Detection reason: " + detectionResult.detectionReason);

    // Initialize .cronus/ directory if it doesn't exist
    if(!detectionResult.hasExistingConfig)
    {
        logInfo("No existing configuration found. Initializing directory structure...");
        
        auto initResult = DirectoryInitializer::initialize(m_workingDirectory, m_operationalMode);
        
        if(initResult.success)
        {
            logInfo(initResult.message);
        }
        else
        {
            logWarning(initResult.message);
        }
    }
    else
    {
        logInfo("Using existing .cronus/ configuration");
        
        // Validate existing structure
        if(!DirectoryInitializer::validateStructure(m_workingDirectory, m_operationalMode))
        {
            logWarning("Existing directory structure may be incomplete or invalid");
        }
    }

    configPaths.push_back(resourcePath/"loreforge");
    if(home)
    {
        configPaths.push_back(std::filesystem::path(home)/".cronus"/"loreforge");
    }
    configPaths.push_back(std::filesystem::path(".cronus/loreforge"));

    // TODO: Initialize loreforge with config paths when the function is implemented
    // loreforge::initialize(configPaths);

    // Load model definitions first
    loadModelDefinitions(resourceDir);

    // Load in order of precedence (later overrides earlier)
    loadFromEnv();

    // Load from home directory config
    if(home)
    {
        std::filesystem::path homeConfig=std::filesystem::path(home)/".cronus"/"config.yml";
        if(std::filesystem::exists(homeConfig))
        {
            loadFromFile(homeConfig);
        }
        
        // Also try JSON config
        std::filesystem::path homeConfigJson=std::filesystem::path(home)/".cronus"/"config.json";
        if(std::filesystem::exists(homeConfigJson))
        {
            loadFromJsonFile(homeConfigJson);
        }
    }

    // Load from current directory config
    std::filesystem::path localConfig=".cronus/config.yml";
    if(std::filesystem::exists(localConfig))
    {
        loadFromFile(localConfig);
    }
    
    // Also try JSON config (preferred for new installations)
    std::filesystem::path localConfigJson=".cronus/config.json";
    if(std::filesystem::exists(localConfigJson))
    {
        loadFromJsonFile(localConfigJson);
    }
}

std::optional<std::string> Config::getApiKey(const std::string &provider) const
{
    auto it=m_apiKeys.find(provider);

    if(it!=m_apiKeys.end())
    {
        return it->second;
    }
    return std::nullopt;
}

void Config::setApiKey(const std::string &provider, const std::string &key)
{
    m_apiKeys[provider]=key;
}

} // namespace cronus
