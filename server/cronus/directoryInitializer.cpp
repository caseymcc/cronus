#include "cronus/directoryInitializer.h"
#include "cronus/logger.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace cronus
{

InitializationResult DirectoryInitializer::initialize(
    const std::filesystem::path &workingDir,
    OperationalMode mode,
    const std::string &sourceRepo)
{
    InitializationResult result;
    result.mode = mode;
    result.cronusDir = workingDir / CONFIG_DIR;

    // Check if already initialized
    if(std::filesystem::exists(result.cronusDir))
    {
        result.success = false;
        result.message = "Directory already initialized. .cronus/ directory exists.";
        return result;
    }

    bool success = false;
    if(mode == OperationalMode::SingleAgent)
    {
        success = initializeSingleAgent(workingDir);
        result.message = success 
            ? "Single-agent mode initialized successfully"
            : "Failed to initialize single-agent mode";
    }
    else
    {
        success = initializeMultiAgent(workingDir, sourceRepo);
        result.message = success 
            ? "Multi-agent mode initialized successfully"
            : "Failed to initialize multi-agent mode";
    }

    result.success = success;
    return result;
}

bool DirectoryInitializer::initializeSingleAgent(const std::filesystem::path &workingDir)
{
    std::filesystem::path cronusDir = workingDir / CONFIG_DIR;

    try
    {
        // Create main directory structure
        if(!createDirectory(cronusDir)) return false;
        if(!createDirectory(cronusDir / "tasks")) return false;
        if(!createDirectory(cronusDir / "context")) return false;
        if(!createDirectory(cronusDir / "cache")) return false;
        if(!createDirectory(cronusDir / "logs")) return false;

        // Create config.json
        if(!createSingleAgentConfig(cronusDir / CONFIG_FILE))
        {
            logError("Failed to create config.json");
            return false;
        }

        // Create state.json
        if(!createStateFile(cronusDir / STATE_FILE, OperationalMode::SingleAgent))
        {
            logError("Failed to create state.json");
            return false;
        }

        // Create empty lock file
        std::ofstream lockFile(cronusDir / LOCK_FILE);
        lockFile.close();

        // Set restrictive permissions on .cronus directory (user only)
        std::filesystem::permissions(cronusDir, 
            std::filesystem::perms::owner_read | 
            std::filesystem::perms::owner_write | 
            std::filesystem::perms::owner_exec,
            std::filesystem::perm_options::replace);

        logInfo("Single-agent directory structure created at: " + cronusDir.string());
        return true;
    }
    catch(const std::exception &e)
    {
        logError("Error initializing single-agent directory: " + std::string(e.what()));
        return false;
    }
}

bool DirectoryInitializer::initializeMultiAgent(const std::filesystem::path &workingDir, const std::string &sourceRepo)
{
    std::filesystem::path cronusDir = workingDir / CONFIG_DIR;

    try
    {
        // Create main directory structure
        if(!createDirectory(cronusDir)) return false;
        if(!createDirectory(cronusDir / "shared")) return false;
        if(!createDirectory(cronusDir / "shared" / "cache")) return false;
        if(!createDirectory(cronusDir / "shared" / "templates")) return false;
        if(!createDirectory(cronusDir / "shared" / "models")) return false;
        if(!createDirectory(cronusDir / "comparison")) return false;
        if(!createDirectory(cronusDir / "logs")) return false;

        // Create multi-agent marker
        std::ofstream markerFile(cronusDir / MULTI_AGENT_MARKER);
        markerFile << "# This file indicates multi-agent mode\n";
        markerFile.close();

        // Create config.json
        if(!createMultiAgentConfig(cronusDir / CONFIG_FILE, sourceRepo))
        {
            logError("Failed to create config.json");
            return false;
        }

        // Create state.json
        if(!createStateFile(cronusDir / STATE_FILE, OperationalMode::MultiAgent))
        {
            logError("Failed to create state.json");
            return false;
        }

        // Set restrictive permissions
        std::filesystem::permissions(cronusDir,
            std::filesystem::perms::owner_read |
            std::filesystem::perms::owner_write |
            std::filesystem::perms::owner_exec,
            std::filesystem::perm_options::replace);

        logInfo("Multi-agent directory structure created at: " + cronusDir.string());
        return true;
    }
    catch(const std::exception &e)
    {
        logError("Error initializing multi-agent directory: " + std::string(e.what()));
        return false;
    }
}

bool DirectoryInitializer::createSingleAgentConfig(const std::filesystem::path &configPath)
{
    nlohmann::json config = {
        {"version", "1.0"},
        {"mode", "single-agent"},
        {"model", {
            {"provider", "openai"},
            {"name", "gpt-3.5-turbo"},
            {"temperature", 0.7},
            {"max_tokens", 4096}
        }},
        {"api_keys", {
            {"openai", "${OPENAI_API_KEY}"},
            {"anthropic", "${ANTHROPIC_API_KEY}"}
        }},
        {"loreforge", {
            {"enabled", true},
            {"cache_dir", ".cronus/cache"}
        }},
        {"workspace", {
            {"working_directory", "."}
        }}
    };

    try
    {
        std::ofstream file(configPath);
        if(!file.is_open())
        {
            return false;
        }
        file << config.dump(2) << std::endl;
        return true;
    }
    catch(const std::exception &e)
    {
        logError("Error creating single-agent config: " + std::string(e.what()));
        return false;
    }
}

bool DirectoryInitializer::createMultiAgentConfig(const std::filesystem::path &configPath, const std::string &sourceRepo)
{
    nlohmann::json config = {
        {"version", "1.0"},
        {"mode", "multi-agent"},
        {"server", {
            {"host", "localhost"},
            {"port", 9000},
            {"api_path", "/api"},
            {"enable_cors", true},
            {"log_level", "info"},
            {"max_concurrent_agents", 10}
        }},
        {"workspace", {
            {"root", "."},
            {"source_repository", sourceRepo},
            {"auto_sync", false},
            {"sync_interval_minutes", 30}
        }},
        {"agents", nlohmann::json::array()},
        {"shared", {
            {"cache_dir", ".cronus/shared/cache"},
            {"templates_dir", ".cronus/shared/templates"},
            {"models_dir", ".cronus/shared/models"},
            {"loreforge", {
                {"enabled", true},
                {"embedding_model", "text-embedding-3-small"}
            }}
        }},
        {"comparison", {
            {"enabled", true},
            {"metrics", nlohmann::json::array({"correctness", "code_quality", "performance"})}
        }}
    };

    try
    {
        std::ofstream file(configPath);
        if(!file.is_open())
        {
            return false;
        }
        file << config.dump(2) << std::endl;
        return true;
    }
    catch(const std::exception &e)
    {
        logError("Error creating multi-agent config: " + std::string(e.what()));
        return false;
    }
}

bool DirectoryInitializer::createStateFile(const std::filesystem::path &statePath, OperationalMode mode)
{
    // Get current timestamp in ISO 8601 format
    auto now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&now), "%Y-%m-%dT%H:%M:%SZ");
    std::string timestamp = ss.str();

    nlohmann::json state;
    state["version"] = "1.0";
    state["mode"] = operationalModeToString(mode);
    state["created_at"] = timestamp;
    state["last_updated"] = timestamp;

    if(mode == OperationalMode::SingleAgent)
    {
        state["agent"] = {
            {"status", "idle"},
            {"current_task", nullptr},
            {"stats", {
                {"messages", 0},
                {"files_modified", 0},
                {"tasks_completed", 0}
            }}
        };
    }
    else
    {
        state["workspace"] = {
            {"root", "."},
            {"agent_count", 0}
        };
        state["agents"] = nlohmann::json::array();
    }

    try
    {
        std::ofstream file(statePath);
        if(!file.is_open())
        {
            return false;
        }
        file << state.dump(2) << std::endl;
        return true;
    }
    catch(const std::exception &e)
    {
        logError("Error creating state file: " + std::string(e.what()));
        return false;
    }
}

bool DirectoryInitializer::validateStructure(const std::filesystem::path &workingDir, OperationalMode mode)
{
    std::filesystem::path cronusDir = workingDir / CONFIG_DIR;

    if(!std::filesystem::exists(cronusDir))
    {
        return false;
    }

    // Common checks
    if(!std::filesystem::exists(cronusDir / CONFIG_FILE))
    {
        return false;
    }

    if(mode == OperationalMode::MultiAgent)
    {
        // Multi-agent specific checks
        if(!std::filesystem::exists(cronusDir / MULTI_AGENT_MARKER))
        {
            return false;
        }
        if(!std::filesystem::exists(cronusDir / "shared"))
        {
            return false;
        }
    }
    else
    {
        // Single-agent specific checks
        if(!std::filesystem::exists(cronusDir / "tasks"))
        {
            return false;
        }
        if(!std::filesystem::exists(cronusDir / "context"))
        {
            return false;
        }
    }

    return true;
}

bool DirectoryInitializer::initializeAgentWorkspace(
    const std::filesystem::path &workspaceRoot,
    const std::string &agentId,
    const std::string &sourceRepo)
{
    std::filesystem::path agentsDir = workspaceRoot / "agents";
    std::filesystem::path agentDir = agentsDir / agentId;
    std::filesystem::path cronusDir = agentDir / CONFIG_DIR;

    try
    {
        // Create agents directory if it doesn't exist
        if(!createDirectory(agentsDir))
        {
            return false;
        }

        // Create agent directory
        if(!createDirectory(agentDir))
        {
            return false;
        }

        // Create .cronus subdirectories
        if(!createDirectory(cronusDir)) return false;
        if(!createDirectory(cronusDir / "tasks")) return false;
        if(!createDirectory(cronusDir / "context")) return false;
        if(!createDirectory(cronusDir / "cache")) return false;
        if(!createDirectory(cronusDir / "logs")) return false;

        // Create agent-specific config
        nlohmann::json agentConfig = {
            {"version", "1.0"},
            {"agent_id", agentId},
            {"mode", "single-agent"},  // Each agent operates in single-agent mode
            {"model", {
                {"provider", "openai"},
                {"name", "gpt-3.5-turbo"},
                {"temperature", 0.7},
                {"max_tokens", 4096}
            }},
            {"workspace", {
                {"working_directory", "."}
            }}
        };

        std::ofstream configFile(cronusDir / CONFIG_FILE);
        configFile << agentConfig.dump(2) << std::endl;
        configFile.close();

        // Create agent state file
        createStateFile(cronusDir / STATE_FILE, OperationalMode::SingleAgent);

        // Clone source repository if provided
        if(!sourceRepo.empty())
        {
            std::string cloneCmd = "git clone " + sourceRepo + " " + agentDir.string();
            int result = std::system(cloneCmd.c_str());
            if(result != 0)
            {
                logWarning("Failed to clone repository for agent " + agentId);
                // Continue anyway - agent can work without source
            }
            else
            {
                logInfo("Cloned repository for agent " + agentId);
            }
        }

        logInfo("Agent workspace initialized: " + agentDir.string());
        return true;
    }
    catch(const std::exception &e)
    {
        logError("Error initializing agent workspace: " + std::string(e.what()));
        return false;
    }
}

bool DirectoryInitializer::createDirectory(const std::filesystem::path &path)
{
    try
    {
        if(std::filesystem::exists(path))
        {
            return true;
        }
        return std::filesystem::create_directories(path);
    }
    catch(const std::filesystem::filesystem_error &e)
    {
        logError("Failed to create directory " + path.string() + ": " + e.what());
        return false;
    }
}

bool DirectoryInitializer::writeFile(const std::filesystem::path &path, const std::string &content)
{
    try
    {
        std::ofstream file(path);
        if(!file.is_open())
        {
            return false;
        }
        file << content;
        return true;
    }
    catch(const std::exception &e)
    {
        logError("Failed to write file " + path.string() + ": " + e.what());
        return false;
    }
}

} // namespace cronus
