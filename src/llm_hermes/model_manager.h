#ifndef _llm_hermes_model_manager_h_
#define _llm_hermes_model_manager_h_

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <filesystem>

namespace llm_hermes
{

struct ModelInfo {
    std::string model;
    std::string provider;
    std::string mode{"chat"};
    std::optional<std::string> api_base;
    bool examples_as_sys_msg{false};
    int context_window{4096};
    int max_tokens{2048};
    int max_input_tokens{3072};
    int max_output_tokens{1024};
    double input_cost_per_token{0.0};
    double output_cost_per_token{0.0};
};

class ModelManager {
public:
    static ModelManager& instance();

    bool initialize(const std::filesystem::path& configPath);
    std::optional<std::string> getProvider(const std::string& model) const;
    std::optional<ModelInfo> getModelInfo(const std::string& model) const;
    const std::map<std::string, std::string>& getModelProviderMap() const { return m_modelProviderMap; }

private:
    ModelManager() = default;
    bool loadModelFile(const std::filesystem::path& filePath);

    std::vector<ModelInfo> m_models;
    std::map<std::string, std::string> m_modelProviderMap;
    bool m_initialized{false};
};

} // namespace llm_hermes

#endif//_llm_hermes_model_manager_h_
