#include "cronus/config.h"

#include <yaml-cpp/yaml.h>

#include <cstdlib>
#include <iostream>

namespace cronus
{

Config &Config::instance()
{
    static Config instance;
    return instance;
}

void Config::setModelAndProvider(const std::string& combined) {
    auto pos = combined.find('/');
    if (pos != std::string::npos) {
        m_provider = combined.substr(0, pos);
        m_model = combined.substr(pos + 1);
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
    try
    {
        YAML::Node config=YAML::LoadFile(configPath.string());

        if(config["model"])
        {
            setModelAndProvider(config["model"].as<std::string>());
        }

        if(config["api_keys"]||config["api-keys"])
        {
            auto keys=config["api_keys"].IsDefined()?config["api_keys"]:config["api-keys"];

            if(keys["openai"])
            {
                m_apiKeys["openai"]=keys["openai"].as<std::string>();
            }
            if(keys["anthropic"])
            {
                m_apiKeys["anthropic"]=keys["anthropic"].as<std::string>();
            }
        }
    }
    catch(const std::exception &e)
    {
        std::cerr<<"Warning: Failed to load config from "<<configPath<<": "<<e.what()<<std::endl;
    }
}

void Config::load()
{
    // Load in order of precedence (later overrides earlier)
    loadFromEnv();

    // Load from home directory config
    if(const char *home=std::getenv("HOME"))
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
