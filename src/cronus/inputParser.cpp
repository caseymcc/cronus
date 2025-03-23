#include "cronus/inputParser.h"

#include <algorithm>

std::vector<std::string> InputParser::extractFileReferences(const std::string &input) const
{
    std::vector<std::string> fileReferences;
    std::vector<std::string> words;
    
    // Extract words from input
    std::istringstream iss(input);
    std::string word;
    while(iss >> word)
    {
        // Only consider words that might be file paths (contain a dot)
        if(word.find('.') != std::string::npos)
        {
            words.push_back(word);
        }
    }
    
    // Check if any of the words match files in the source map
    for(const auto& word : words)
    {
        // Use the new findFilesByPartialName function
        std::vector<std::string> matchingFiles = m_sourceMap->findFilesByPartialName(word);
        if (!matchingFiles.empty()) {
            // Add all matching files to the references
            fileReferences.insert(fileReferences.end(), matchingFiles.begin(), matchingFiles.end());
            continue;
        }
        
        // If no matches found, check if it exists in the current directory
        std::filesystem::path potentialPath = m_currentPath / word;
        if(std::filesystem::exists(potentialPath) && !std::filesystem::is_directory(potentialPath))
        {
            fileReferences.push_back(potentialPath.string());
        }
    }
    
    // Remove duplicates
    std::sort(fileReferences.begin(), fileReferences.end());
    fileReferences.erase(std::unique(fileReferences.begin(), fileReferences.end()), fileReferences.end());
    
    return fileReferences;
}

std::vector<std::string> InputParser::extractTagReferences(const std::string &input) const
{
    std::vector<std::string> tagReferences;
    
    // Get all files from the source map cache directly
    for(const auto& [path, tags] : m_sourceMap->getFileCache())
    {
        // Check each tag in the file
        for(const auto& tag : tags.m_tags)
        {
            // Only add if the tag name is mentioned in the input
            if(input.find(tag.name) != std::string::npos)
            {
                tagReferences.push_back(tag.name);
            }
        }
    }
    
    // Remove duplicates
    std::sort(tagReferences.begin(), tagReferences.end());
    tagReferences.erase(std::unique(tagReferences.begin(), tagReferences.end()), tagReferences.end());
    
    return tagReferences;
}
#include "cronus/inputParser.h"
#include <sstream>
#include <algorithm>
#include <filesystem>
