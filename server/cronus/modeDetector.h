#ifndef _cronus_mode_detector_h_
#define _cronus_mode_detector_h_

#include <filesystem>
#include <optional>
#include <string>

namespace cronus
{

/**
 * @brief Operational mode for Cronus
 */
enum class OperationalMode
{
    SingleAgent,  ///< Single agent working in an existing repository
    MultiAgent    ///< Multiple agents with isolated workspaces
};

/**
 * @brief Mode detection result with contextual information
 */
struct ModeDetectionResult
{
    OperationalMode mode;
    bool hasExistingConfig;     ///< True if .cronus/ already exists
    bool hasVersionControl;     ///< True if .git/ or similar exists
    bool isEmptyDirectory;      ///< True if directory is empty/nearly empty
    bool hasMultiAgentMarker;   ///< True if multi-agent.marker exists
    std::string detectionReason; ///< Human-readable reason for detection
};

/**
 * @brief Detects the appropriate operational mode for Cronus
 * 
 * Detection algorithm:
 * 1. Check for existing .cronus/multi-agent.marker -> MultiAgent
 * 2. Check for existing .cronus/config.json and parse mode -> Use specified mode
 * 3. Check for version control (.git, .svn, .hg) -> SingleAgent
 * 4. Check if directory is empty/nearly empty -> MultiAgent (default for empty dirs)
 * 5. Directory with content but no VCS -> SingleAgent
 */
class ModeDetector
{
public:
    /**
     * @brief Detect operational mode for a working directory
     * @param workingDir Directory to analyze
     * @return Detection result with mode and contextual info
     */
    static ModeDetectionResult detect(const std::filesystem::path &workingDir);

    /**
     * @brief Check if directory has an existing Cronus configuration
     * @param workingDir Directory to check
     * @return True if .cronus/ exists and has a config.json
     */
    static bool hasExistingConfig(const std::filesystem::path &workingDir);

    /**
     * @brief Check if directory is a version-controlled repository
     * @param workingDir Directory to check
     * @return True if .git/, .svn/, or .hg/ exists
     */
    static bool hasVersionControl(const std::filesystem::path &workingDir);

    /**
     * @brief Check if directory is empty or nearly empty
     * @param workingDir Directory to check
     * @param threshold Maximum number of entries to consider "nearly empty"
     * @return True if directory has <= threshold entries
     */
    static bool isEmptyDirectory(const std::filesystem::path &workingDir, int threshold = 1);

    /**
     * @brief Read mode from existing config.json
     * @param configPath Path to config.json file
     * @return Mode if successfully parsed, nullopt otherwise
     */
    static std::optional<OperationalMode> readModeFromConfig(const std::filesystem::path &configPath);

private:
    static constexpr const char *MULTI_AGENT_MARKER = "multi-agent.marker";
    static constexpr const char *CONFIG_DIR = ".cronus";
    static constexpr const char *CONFIG_FILE = "config.json";
};

/**
 * @brief Convert OperationalMode to string
 */
const char* operationalModeToString(OperationalMode mode);

/**
 * @brief Convert string to OperationalMode
 */
std::optional<OperationalMode> stringToOperationalMode(const std::string &str);

} // namespace cronus

#endif // _cronus_mode_detector_h_
