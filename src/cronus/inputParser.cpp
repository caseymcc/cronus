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
        // Try as a full path
        if(m_sourceMap->getTags(word, "").size() > 0)
        {
            fileReferences.push_back(word);
            continue;
        }
        
        // Try as a relative path
        std::string relativePath = m_sourceMap->getRelativeFname(word);
        if(m_sourceMap->getTags("", relativePath).size() > 0)
        {
            fileReferences.push_back(word);
            continue;
        }
        
        // Try as a filename only
        std::filesystem::path wordPath(word);
        std::string filename = wordPath.filename().string();
        
        // Check if it exists in the current directory
        std::filesystem::path potentialPath = m_currentPath / filename;
        if(std::filesystem::exists(potentialPath) && !std::filesystem::is_directory(potentialPath))
        {
            fileReferences.push_back(potentialPath.string());
        }
    }
    
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
