#ifndef _cronus_input_parsers_h_
#define _cronus_input_parsers_h_

#include "cronus/sourceMap.h"

#include <vector>

class InputParser
{
public:
    InputParser(std::shared_ptr<cronus::SourceMap> sourceMap, const std::filesystem::path &currentPath)
        : m_sourceMap(sourceMap), m_currentPath(currentPath) {}

    std::vector<std::string> extractFileReferences(const std::string &input) const;
    std::vector<std::string> extractTagReferences(const std::string &input) const;

private:
    std::shared_ptr<cronus::SourceMap> m_sourceMap;
    std::filesystem::path m_currentPath;

};

#endif//_cronus_input_parsers_h_