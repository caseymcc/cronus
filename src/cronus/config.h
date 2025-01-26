#ifndef _cronus_config_h_
#define _cronus_config_h_

#include <string>
#include <optional>
#include <filesystem>
#include <map>

namespace cronus
{

class Config
{
public:
    static Config &instance();

    void load();

    std::string getModel() const { return m_model; }
    std::string getProvider() const { return m_provider; }
    void setModelAndProvider(const std::string &combined);

    std::optional<std::string> getApiKey(const std::string &provider) const;
    void setApiKey(const std::string &provider, const std::string &key);

private:
    Config()=default;
    void loadFromEnv();
    void loadFromFile(const std::filesystem::path &configPath);

    std::string m_model{ "gpt-3.5-turbo" };
    std::string m_provider{ "openai" };
    std::map<std::string, std::string> m_apiKeys;
};

} // namespace cronus

#endif//_cronus_config_h_
