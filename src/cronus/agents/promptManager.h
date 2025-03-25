#ifndef _cronus_agents_promptManager_h_
#define _cronus_agents_promptManager_h_

#include <string>
#include <map>
#include <optional>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace cronus
{
namespace agents
{

/**
 * @class PromptManager
 * @brief Manages prompts for different models and providers
 *
 * This class provides functionality to load and retrieve prompts
 * for different models and providers, with fallback to default prompts.
 */
class PromptManager
{
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static PromptManager& instance();

    /**
     * @brief Initialize the prompt manager with config paths
     * @param configPaths Paths to search for prompt configuration files
     * @return True if initialization was successful
     */
    bool initialize(const std::vector<std::filesystem::path>& configPaths);

    /**
     * @brief Get a prompt for a specific agent and task
     * @param agentName The name of the agent (e.g., "coder")
     * @param promptName The name of the prompt (e.g., "system_message")
     * @param modelName The name of the model (optional)
     * @param providerName The name of the provider (optional)
     * @return The prompt string or nullopt if not found
     */
    std::optional<std::string> getPrompt(
        const std::string& agentName,
        const std::string& promptName,
        const std::string& modelName = "",
        const std::string& providerName = "") const;

private:
    PromptManager() = default;
    
    /**
     * @brief Load prompts from a configuration file
     * @param filePath Path to the prompt configuration file
     * @return True if loading was successful
     */
    bool loadPromptsFile(const std::filesystem::path& filePath);

    // Structure: agent -> prompt_name -> {default, model_specific, provider_specific, model_provider_specific}
    struct PromptVariants {
        std::string defaultPrompt;
        std::map<std::string, std::string> modelSpecific;
        std::map<std::string, std::string> providerSpecific;
        std::map<std::string, std::map<std::string, std::string>> modelProviderSpecific;
    };

    std::map<std::string, std::map<std::string, PromptVariants>> m_prompts;
    bool m_initialized{false};
};

} // namespace agents
} // namespace cronus

#endif // _cronus_agents_promptManager_h_
