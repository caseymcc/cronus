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

    std::string get_model() const { return model_; }
    std::string get_provider() const { return provider_; }
    void set_model_and_provider(const std::string &combined);

    std::optional<std::string> get_api_key(const std::string &provider) const;
    void set_api_key(const std::string &provider, const std::string &key);

private:
    Config()=default;
    void load_from_env();
    void load_from_file(const std::filesystem::path &config_path);

    std::string model_{ "gpt-3.5-turbo" };
    std::string provider_{ "openai" };
    std::map<std::string, std::string> api_keys_;
};

} // namespace cronus

#endif//_cronus_config_h_
