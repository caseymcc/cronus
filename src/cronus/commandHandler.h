#ifndef _cronus_command_handler_h_
#define _cronus_command_handler_h_

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>

namespace cronus {

class SourceMap;

/**
 * @class CommandHandler
 * @brief Handles command processing for the Cronus system
 * 
 * This class processes commands that start with a slash (/) character.
 * It supports commands like /add and /remove for managing files.
 */
class CommandHandler {
public:
    /**
     * @brief Constructor
     * @param sourceMap Shared pointer to the source map
     */
    explicit CommandHandler(std::shared_ptr<SourceMap> sourceMap);

    /**
     * @brief Check if a string is a command
     * @param input The input string to check
     * @return True if the input is a command (starts with /)
     */
    bool isCommand(const std::string& input) const;

    /**
     * @brief Process a command
     * @param input The command string to process
     * @return Response message from the command execution
     */
    std::string processCommand(const std::string& input);

private:
    std::shared_ptr<SourceMap> m_sourceMap;
    
    // Command handlers
    std::string handleAddCommand(const std::vector<std::string>& args);
    std::string handleRemoveCommand(const std::vector<std::string>& args);
    std::string handleHelpCommand(const std::vector<std::string>& args);
    
    // Parse command into command name and arguments
    std::pair<std::string, std::vector<std::string>> parseCommand(const std::string& input) const;
    
    // Map of command names to handler functions
    using CommandFunction = std::function<std::string(const std::vector<std::string>&)>;
    std::unordered_map<std::string, CommandFunction> m_commandMap;
};

} // namespace cronus

#endif // _cronus_command_handler_h_
