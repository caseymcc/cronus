#ifndef _cronus_sourceUtils_h_
#define _cronus_sourceUtils_h_

#include <string>

namespace cronus
{

inline bool isSourceFile(const std::string &ext)
{
    static const std::unordered_set<std::string> sourceExts=
    {
        ".cpp", ".h", ".hpp", ".c", ".cc"
        ".py", 
        ".js", ".ts", 
        ".php", ".java",
        ".swift", ".dart", 
        ".go", 
        ".rs", 
        ".lua", 
        ".rb", 
        ".hs", 
        ".scm"
    };
    return sourceExts.find(ext)!=sourceExts.end();
}

}// namespace cronus

#endif//_cronus_sourceUtils_h_