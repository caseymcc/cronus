#include "cronus/commandHandler.h"
#include "cronus/sourceMap.h"
#include "cronus/logger.h"
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace cronus
{

CommandHandler::CommandHandler(std::shared_ptr<SourceMap> sourceMap)
    : m_sourceMap(sourceMap)
{
    // Register command handlers
    m_commandMap["/add"]=[this](const auto &args) { return handleAddCommand(args); };
    m_commandMap["/remove"]=[this](const auto &args) { return handleRemoveCommand(args); };
    m_commandMap["/help"]=[this](const auto &args) { return handleHelpCommand(args); };

    logInfo("Command handler initialized");
}

bool CommandHandler::isCommand(const std::string &input) const
{
    return !input.empty()&&input[0]=='/';
}

std::string CommandHandler::processCommand(const std::string &input)
{
    auto [command, args]=parseCommand(input);

    // Look up the command in the map
    auto it=m_commandMap.find(command);
    if(it!=m_commandMap.end())
    {
        // Execute the command handler
        return it->second(args);
    }

    // Command not found
    return "Unknown command: "+command+"\nType /help for available commands.";
}

std::pair<std::string, std::vector<std::string>> CommandHandler::parseCommand(const std::string &input) const
{
    std::istringstream iss(input);
    std::string command;
    iss>>command;

    std::vector<std::string> args;
    std::string arg;
    while(iss>>arg)
    {
        args.push_back(arg);
    }

    return { command, args };
}

std::string CommandHandler::handleAddCommand(const std::vector<std::string> &args)
{
    if(args.empty())
    {
        return "Usage: /add <filename>\nAdds a file to the current context.";
    }

    std::string filename=args[0];
    std::filesystem::path filePath(filename);

    // Check if file exists
    if(!std::filesystem::exists(filePath))
    {
        // Try to find the file using partial name matching
        auto matchingFiles=m_sourceMap->findFilesByPartialName(filename);
        if(matchingFiles.empty())
        {
            return "Error: File not found: "+filename;
        }
        else if(matchingFiles.size()>1)
        {
            std::stringstream ss;
            ss<<"Multiple files match '"<<filename<<"'. Please be more specific:\n";
            for(const auto &file:matchingFiles)
            {
                ss<<"- "<<file<<"\n";
            }
            return ss.str();
        }
        else
        {
            // One match found
            filename=matchingFiles[0];
            filePath=std::filesystem::path(filename);
        }
    }

    // Update the source map with this file
    m_sourceMap->update();

    return "Added file to context: "+filePath.string();
}

std::string CommandHandler::handleRemoveCommand(const std::vector<std::string> &args)
{
    if(args.empty())
    {
        return "Usage: /remove <filename>\nRemoves a file from the current context.";
    }

    std::string filename=args[0];

    // Try to find the file using partial name matching
    auto matchingFiles=m_sourceMap->findFilesByPartialName(filename);
    if(matchingFiles.empty())
    {
        return "Error: File not found in context: "+filename;
    }
    else if(matchingFiles.size()>1)
    {
        std::stringstream ss;
        ss<<"Multiple files match '"<<filename<<"'. Please be more specific:\n";
        for(const auto &file:matchingFiles)
        {
            ss<<"- "<<file<<"\n";
        }
        return ss.str();
    }

    // One match found - we don't actually remove it from the source map,
    // but we can acknowledge the command
    return "Removed file from context: "+matchingFiles[0];
}

std::string CommandHandler::handleHelpCommand(const std::vector<std::string> &args)
{
    std::stringstream ss;
    ss<<"Available commands:\n";
    ss<<"  /add <filename>    - Add a file to the current context\n";
    ss<<"  /remove <filename> - Remove a file from the current context\n";
    ss<<"  /help              - Show this help message\n";
    return ss.str();
}

} // namespace cronus
