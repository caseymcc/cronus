#define YAML_CPP_NOEXCEPT 1

#include "cronus/config.h"
#include "cronus/logger.h"

#include "hermes/hermes.h"

#include <yaml-cpp/yaml.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace cronus
{

Config &Config::instance()
{
    static Config instance;
    return instance;
}

void Config::setModelAndProvider(const std::string &combined)
{
    auto pos=combined.find('/');
    if(pos!=std::string::npos)
    {
        m_provider=combined.substr(0, pos);
        m_model=combined.substr(pos+1);
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
        setModelAndProvider(config["model"].as<std::string>());
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
        if(!model["model_name"]||!model["litellm_params"])
        {
            logWarning("Skipping invalid model entry in: "+configPath.string());
            continue;
        }

        const auto &params=model["litellm_params"];
        if(!params["model"]||!params["provider"]||!params["api_base"])
        {
            logWarning("Skipping model with missing parameters in: "+configPath.string());
            continue;
        }

        ModelConfig modelConfig;
        modelConfig.model_name=model["model_name"].as<std::string>();
        modelConfig.actual_model=params["model"].as<std::string>();
        modelConfig.provider=params["provider"].as<std::string>();
        modelConfig.api_base=params["api_base"].as<std::string>();
        
        // Load require_api_key if present
        if (model["require_api_key"]) {
            modelConfig.require_api_key = model["require_api_key"].as<bool>();
        }

        if(override)
        {
            // Update existing config if present
            auto it=std::find_if(m_modelConfigs.begin(), m_modelConfigs.end(),
                [&](const ModelConfig &cfg) { return cfg.model_name==modelConfig.model_name; });

            if(it!=m_modelConfigs.end())
            {
                // Update existing values
                it->actual_model=modelConfig.actual_model;
                it->provider=modelConfig.provider;
                it->api_base=modelConfig.api_base;
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
        if(config.model_name==model_name)
        {
            return config;
        }
    }
    return std::nullopt;
}

void Config::load(const std::string &resourceDir)
{
    std::filesystem::path resourcePath=resourceDir;
    std::vector<std::filesystem::path> configPaths;
    const char *home=std::getenv("HOME");

    configPaths.push_back(resourcePath/"hermes");
    if(home)
    {
        configPaths.push_back(std::filesystem::path(home)/".cronus"/"hermes");
    }
    configPaths.push_back(std::filesystem::path(".cronus/hermes"));

    hermes::initialize(configPaths);

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
    }

    // Load from current directory config
    std::filesystem::path localConfig=".cronus/config.yml";
    if(std::filesystem::exists(localConfig))
    {
        loadFromFile(localConfig);
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
