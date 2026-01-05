#ifndef _cronus_config_h_
#define _cronus_config_h_

#include "cronus/modeDetector.h"

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
    
    /**
     * @brief Get the configuration paths used by the system
     * @return Vector of filesystem paths to configuration directories
     */
    std::vector<std::filesystem::path> getConfigPaths() const;

    /**
     * @brief Get the operational mode (single-agent or multi-agent)
     * @return Current operational mode
     */
    OperationalMode getOperationalMode() const { return m_operationalMode; }

    /**
     * @brief Set the operational mode
     * @param mode Operational mode to set
     */
    void setOperationalMode(OperationalMode mode) { m_operationalMode = mode; }

    /**
     * @brief Get the working directory
     * @return Working directory path
     */
    std::filesystem::path getWorkingDirectory() const { return m_workingDirectory; }

    /**
     * @brief Get the notification host for startup notifications
     * @return Notification host (default: localhost)
     */
    std::string getNotificationHost() const { return m_notificationHost; }

    /**
     * @brief Get the notification port for startup notifications
     * @return Notification port (default: 8999)
     */
    int getNotificationPort() const { return m_notificationPort; }

    /**
     * @brief Set the notification host
     * @param host Notification host
     */
    void setNotificationHost(const std::string &host) { m_notificationHost = host; }

    /**
     * @brief Set the notification port
     * @param port Notification port
     */
    void setNotificationPort(int port) { m_notificationPort = port; }

private:
    Config()=default;
    void loadFromEnv();
    void loadFromFile(const std::filesystem::path &configPath);
    void loadFromJsonFile(const std::filesystem::path &configPath);
    void loadModelDefinitions(const std::string &resourcePath);
    void loadModelsFromFile(const std::filesystem::path &configPath, bool override=false);
    void loadModelsFromDirectory(const std::filesystem::path &dirPath, bool override=false);
    std::filesystem::path getDefaultModelConfigPath() const;

    std::string m_model{ "gpt-3.5-turbo" };
    std::string m_provider{ "openai" };
    std::map<std::string, std::string> m_apiKeys;
    std::vector<ModelConfig> m_modelConfigs;
    std::string m_resourceDirectory;
    std::filesystem::path m_workingDirectory{ "." };
    OperationalMode m_operationalMode{ OperationalMode::SingleAgent };
    std::string m_notificationHost{ "localhost" };
    int m_notificationPort{ 8999 };
};

} // namespace cronus

#endif//_cronus_config_h_
