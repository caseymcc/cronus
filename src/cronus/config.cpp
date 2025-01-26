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

void Config::load_from_env()
{
    if(const char *model=std::getenv("CRONUS_MODEL"))
    {
        model_=model;
    }
    if(const char *provider=std::getenv("CRONUS_PROVIDER"))
    {
        provider_=provider;
    }
    if(const char *openai_key=std::getenv("OPENAI_API_KEY"))
    {
        api_keys_["openai"]=openai_key;
    }
    if(const char *anthropic_key=std::getenv("ANTHROPIC_API_KEY"))
    {
        api_keys_["anthropic"]=anthropic_key;
    }
}

void Config::load_from_file(const std::filesystem::path &config_path)
{
    try
    {
        YAML::Node config=YAML::LoadFile(config_path.string());

        if(config["model"])
        {
            model_=config["model"].as<std::string>();
        }
        if(config["provider"])
        {
            provider_=config["provider"].as<std::string>();
        }

        if(config["api_keys"]||config["api-keys"])
        {
            auto keys=config["api_keys"].IsDefined()?config["api_keys"]:config["api-keys"];

            if(keys["openai"])
            {
                api_keys_["openai"]=keys["openai"].as<std::string>();
            }
            if(keys["anthropic"])
            {
                api_keys_["anthropic"]=keys["anthropic"].as<std::string>();
            }
        }
    }
    catch(const std::exception &e)
    {
        std::cerr<<"Warning: Failed to load config from "<<config_path<<": "<<e.what()<<std::endl;
    }
}

void Config::load()
{
    // Load in order of precedence (later overrides earlier)
    load_from_env();

    // Load from home directory config
    if(const char *home=std::getenv("HOME"))
    {
        std::filesystem::path home_config=std::filesystem::path(home)/".cronus"/"config.yml";
        if(std::filesystem::exists(home_config))
        {
            load_from_file(home_config);
        }
    }

    // Load from current directory config
    std::filesystem::path local_config=".cronus/config.yml";
    if(std::filesystem::exists(local_config))
    {
        load_from_file(local_config);
    }
}

std::optional<std::string> Config::get_api_key(const std::string &provider) const
{
    auto it=api_keys_.find(provider);

    if(it!=api_keys_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

void Config::set_api_key(const std::string &provider, const std::string &key)
{
    api_keys_[provider]=key;
}

} // namespace cronus
