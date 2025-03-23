std::vector<std::string> InputParser::extractFileReferences(const std::string &input) const
{
    std::vector<std::string> fileReferences;

    // Simple regex-like pattern matching for file paths
    // Look for patterns like: filename.ext, path/to/file.ext, ./file.ext, etc.
    std::istringstream iss(input);
    std::string word;

    while(iss>>word)
    {
        // Check if word looks like a file path
        if(word.find('.')!=std::string::npos)
        {
            // Check if it's in our source map
            std::filesystem::path potentialPath=m_currentPath/word;
            if(std::filesystem::exists(potentialPath)&&!std::filesystem::is_directory(potentialPath))
            {
                fileReferences.push_back(potentialPath.string());
            }
            else
            {
                // Try relative to current directory
                for(const auto &entry:std::filesystem::directory_iterator(m_currentPath))
                {
                    if(entry.path().filename().string()==word)
                    {
                        fileReferences.push_back(entry.path().string());
                        break;
                    }
                }
            }
        }
    }

    return fileReferences;
}

std::vector<std::string> InputParser::extractTagReferences(const std::string &input) const
{
    std::vector<std::string> tagReferences;

    // Get all files in the source map
    for(const auto &entry:std::filesystem::recursive_directory_iterator(m_currentPath))
    {
        if(!entry.is_regular_file()) continue;

        // Get tags for this file
        std::vector<Tag> tags=m_sourceMap->getTags(entry.path().string(), "");

        // Check if any tag names are mentioned in the input
        for(const auto &tag:tags)
        {
            if(input.find(tag.name)!=std::string::npos)
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