#ifndef _cronus_directory_initializer_h_
#define _cronus_directory_initializer_h_

#include "cronus/modeDetector.h"

#include <filesystem>
#include <string>
#include <optional>

namespace cronus
{

/**
 * @brief Initialization result with status information
 */
struct InitializationResult
{
    bool success;
    std::string message;
    std::filesystem::path cronusDir;
    OperationalMode mode;
};

/**
 * @brief Creates and manages .cronus/ directory structure
 * 
 * Handles initialization of both single-agent and multi-agent mode
 * directory structures, including configuration files, subdirectories,
 * and marker files.
 */
class DirectoryInitializer
{
public:
    /**
     * @brief Initialize .cronus/ directory structure
     * @param workingDir Working directory for initialization
     * @param mode Operational mode (detected or explicitly specified)
     * @param sourceRepo Optional source repository URL for multi-agent mode
     * @return Initialization result
     */
    static InitializationResult initialize(
        const std::filesystem::path &workingDir,
        OperationalMode mode,
        const std::string &sourceRepo = ""
    );

    /**
     * @brief Initialize single-agent mode directory structure
     * 
     * Creates:
     * .cronus/
     * ├── config.json
     * ├── state.json
     * ├── lock
     * ├── tasks/
     * ├── context/
     * ├── cache/
     * └── logs/
     * 
     * @param workingDir Working directory
     * @return True if successful
     */
    static bool initializeSingleAgent(const std::filesystem::path &workingDir);

    /**
     * @brief Initialize multi-agent mode directory structure
     * 
     * Creates:
     * .cronus/
     * ├── multi-agent.marker
     * ├── config.json
     * ├── shared/
     * │   ├── cache/
     * │   └── templates/
     * ├── comparison/
     * └── logs/
     * 
     * @param workingDir Working directory
     * @param sourceRepo Optional source repository URL
     * @return True if successful
     */
    static bool initializeMultiAgent(const std::filesystem::path &workingDir, const std::string &sourceRepo = "");

    /**
     * @brief Create default config.json for single-agent mode
     * @param configPath Path where config.json should be created
     * @return True if successful
     */
    static bool createSingleAgentConfig(const std::filesystem::path &configPath);

    /**
     * @brief Create default config.json for multi-agent mode
     * @param configPath Path where config.json should be created
     * @param sourceRepo Optional source repository URL
     * @return True if successful
     */
    static bool createMultiAgentConfig(const std::filesystem::path &configPath, const std::string &sourceRepo = "");

    /**
     * @brief Create default state.json file
     * @param statePath Path where state.json should be created
     * @param mode Operational mode
     * @return True if successful
     */
    static bool createStateFile(const std::filesystem::path &statePath, OperationalMode mode);

    /**
     * @brief Validate that directory structure is correct
     * @param workingDir Working directory to validate
     * @param mode Expected operational mode
     * @return True if structure is valid
     */
    static bool validateStructure(const std::filesystem::path &workingDir, OperationalMode mode);

    /**
     * @brief Initialize agent workspace (for multi-agent mode)
     * 
     * Creates directory structure for a specific agent in multi-agent mode:
     * agents/<agentId>/
     * ├── .cronus/
     * │   ├── config.json
     * │   ├── state.json
     * │   ├── tasks/
     * │   ├── context/
     * │   ├── cache/
     * │   └── logs/
     * └── .git/  (if cloning from source)
     * 
     * @param workspaceRoot Multi-agent workspace root
     * @param agentId Unique identifier for the agent
     * @param sourceRepo Optional source repository to clone
     * @return True if successful
     */
    static bool initializeAgentWorkspace(
        const std::filesystem::path &workspaceRoot,
        const std::string &agentId,
        const std::string &sourceRepo = ""
    );

private:
    static constexpr const char *CONFIG_DIR = ".cronus";
    static constexpr const char *CONFIG_FILE = "config.json";
    static constexpr const char *STATE_FILE = "state.json";
    static constexpr const char *MULTI_AGENT_MARKER = "multi-agent.marker";
    static constexpr const char *LOCK_FILE = "lock";

    /**
     * @brief Create a directory with error handling
     * @param path Directory path to create
     * @return True if successful or already exists
     */
    static bool createDirectory(const std::filesystem::path &path);

    /**
     * @brief Write JSON content to file
     * @param path File path
     * @param content JSON string content
     * @return True if successful
     */
    static bool writeFile(const std::filesystem::path &path, const std::string &content);
};

} // namespace cronus

#endif // _cronus_directory_initializer_h_
