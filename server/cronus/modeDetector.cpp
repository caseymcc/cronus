#include "cronus/modeDetector.h"
#include "cronus/logger.h"

#include <nlohmann/json.hpp>
#include <fstream>

namespace cronus
{

ModeDetectionResult ModeDetector::detect(const std::filesystem::path &workingDir)
{
    ModeDetectionResult result;
    result.hasExistingConfig = false;
    result.hasVersionControl = false;
    result.isEmptyDirectory = false;
    result.hasMultiAgentMarker = false;

    std::filesystem::path cronusDir = workingDir / CONFIG_DIR;
    std::filesystem::path multiAgentMarkerPath = cronusDir / MULTI_AGENT_MARKER;
    std::filesystem::path configPath = cronusDir / CONFIG_FILE;

    // 1. Check for multi-agent marker
    if(std::filesystem::exists(multiAgentMarkerPath))
    {
        result.mode = OperationalMode::MultiAgent;
        result.hasMultiAgentMarker = true;
        result.hasExistingConfig = std::filesystem::exists(configPath);
        result.detectionReason = "Multi-agent marker file detected";
        return result;
    }

    // 2. Check for existing config.json
    if(std::filesystem::exists(configPath))
    {
        result.hasExistingConfig = true;
        auto modeFromConfig = readModeFromConfig(configPath);
        
        if(modeFromConfig.has_value())
        {
            result.mode = modeFromConfig.value();
            result.detectionReason = "Mode specified in existing config.json";
            return result;
        }
        
        // Config exists but doesn't specify mode - fall through to other checks
        logWarning("Existing config.json doesn't specify mode, using detection heuristics");
    }

    // 3. Check for version control
    result.hasVersionControl = hasVersionControl(workingDir);
    if(result.hasVersionControl)
    {
        result.mode = OperationalMode::SingleAgent;
        result.detectionReason = "Version control repository detected (.git, .svn, or .hg)";
        return result;
    }

    // 4. Check if directory is empty or nearly empty
    result.isEmptyDirectory = isEmptyDirectory(workingDir);
    if(result.isEmptyDirectory)
    {
        result.mode = OperationalMode::MultiAgent;
        result.detectionReason = "Empty/nearly empty directory - defaulting to multi-agent mode";
        return result;
    }

    // 5. Default: directory has content but no VCS
    result.mode = OperationalMode::SingleAgent;
    result.detectionReason = "Directory contains files but no version control - defaulting to single-agent mode";
    return result;
}

bool ModeDetector::hasExistingConfig(const std::filesystem::path &workingDir)
{
    std::filesystem::path configPath = workingDir / CONFIG_DIR / CONFIG_FILE;
    return std::filesystem::exists(configPath);
}

bool ModeDetector::hasVersionControl(const std::filesystem::path &workingDir)
{
    return std::filesystem::exists(workingDir / ".git") ||
           std::filesystem::exists(workingDir / ".svn") ||
           std::filesystem::exists(workingDir / ".hg");
}

bool ModeDetector::isEmptyDirectory(const std::filesystem::path &workingDir, int threshold)
{
    if(!std::filesystem::exists(workingDir))
    {
        return true;
    }

    try
    {
        int count = 0;
        for(const auto &entry : std::filesystem::directory_iterator(workingDir))
        {
            // Ignore hidden files except .cronus
            std::string filename = entry.path().filename().string();
            if(filename[0] == '.' && filename != CONFIG_DIR)
            {
                continue;
            }

            count++;
            if(count > threshold)
            {
                return false;
            }
        }
        return count <= threshold;
    }
    catch(const std::filesystem::filesystem_error &e)
    {
        logError("Error checking directory contents: " + std::string(e.what()));
        return false;
    }
}

std::optional<OperationalMode> ModeDetector::readModeFromConfig(const std::filesystem::path &configPath)
{
    try
    {
        std::ifstream file(configPath);
        if(!file.is_open())
        {
            return std::nullopt;
        }

        nlohmann::json config;
        file >> config;

        if(config.contains("mode"))
        {
            std::string modeStr = config["mode"].get<std::string>();
            return stringToOperationalMode(modeStr);
        }
    }
    catch(const std::exception &e)
    {
        logError("Error reading mode from config: " + std::string(e.what()));
    }

    return std::nullopt;
}

const char* operationalModeToString(OperationalMode mode)
{
    switch(mode)
    {
        case OperationalMode::SingleAgent:
            return "single-agent";
        case OperationalMode::MultiAgent:
            return "multi-agent";
        default:
            return "unknown";
    }
}

std::optional<OperationalMode> stringToOperationalMode(const std::string &str)
{
    if(str == "single-agent" || str == "single")
    {
        return OperationalMode::SingleAgent;
    }
    else if(str == "multi-agent" || str == "multi")
    {
        return OperationalMode::MultiAgent;
    }
    return std::nullopt;
}

} // namespace cronus
