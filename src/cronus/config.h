#ifndef _cronus_config_h_
#define _cronus_config_h_

#include <string>
#include <optional>
#include <filesystem>
#include <map>
#include <vector>

namespace cronus
{

struct ModelConfig
{
    std::string name;
    std::string model;
    bool streaming{false};
};

class Config
{
public:
    static Config &instance();

    void load(const std::string &resourceDir="");

    std::string getModel() const { return m_model; }
    std::string getResourcePath() const { return m_resourceDirectory; }
    std::string getProvider() const { return m_provider; }
    void setModelAndProvider(const std::string &combined);

    std::optional<std::string> getApiKey(const std::string &provider) const;
    void setApiKey(const std::string &provider, const std::string &key);

    std::optional<ModelConfig> getModelConfig(const std::string &model_name) const;
    const std::vector<ModelConfig> &getAvailableModels() const { return m_modelConfigs; }

private:
    Config()=default;
    void loadFromEnv();
    void loadFromFile(const std::filesystem::path &configPath);
    void loadModelDefinitions(const std::string &resourcePath);
    void loadModelsFromFile(const std::filesystem::path &configPath, bool override=false);
    void loadModelsFromDirectory(const std::filesystem::path &dirPath, bool override=false);
    std::filesystem::path getDefaultModelConfigPath() const;

    std::string m_model{ "gpt-3.5-turbo" };
    std::string m_provider{ "openai" };
    std::map<std::string, std::string> m_apiKeys;
    std::vector<ModelConfig> m_modelConfigs;
    std::string m_resourceDirectory;
};

} // namespace cronus

#endif//_cronus_config_h_
