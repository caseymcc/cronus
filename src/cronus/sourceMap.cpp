#include "sourceMap.h"
#include <dirent.h>
#include <sys/stat.h>

#include <sqlite3.h>
#include <random>
#include <algorithm>
#include <chrono>
#include <networkx/pagerank.h>

namespace cronus
{


SourceMap::SourceMap(std::string &workingDir)
{
    m_workingDir=workingDir;
    m_maxMapTokens=1024;
    m_maxContextWindow=0;
    m_mapMulNoFiles=8;
    m_verbose=false;
    m_repoContentPrefix="";
}

SourceMap::~SourceMap()
{
    // Cleanup cache
}

std::string SourceMap::getRelFname(const std::string &fileName)
{
    try
    {
        return std::filesystem::relpath(fileName, m_workingDir);
    }
    catch(const std::filesystem::filesystem_error &e)
    {
        return fileName;
    }
}

void SourceMap::tags_cache_error(const std::string &error)
{
    if(m_verbose)
    {
        std::cout<<"Tags cache error: "<<error<<std::endl;
    }
}

void SourceMap::loadTagsCache()
{
    m_workingDir=workingDir;
    m_maxMapTokens=1024;
    m_maxContextWindow=0;
    m_mapMulNoFiles=8;
    m_verbose=false;
    m_repoContentPrefix="";
}

void SourceMap::saveTagsCache()
{
    // Implementation pending
}

int SourceMap::getMTime(const std::string &fileName)
{
    std::filesystem::path filePath(fileName);

    if(!std::filesystem::exists(filePath))
    {
        std::cout<<"File not found error: "<<fileName<<std::endl;
        return -1;
    }
    return filePath.last_write_time().count();
}

std::vector<Tag> SourceMap::getTags(const std::string &fname, const std::string &rel_fname)
{
    // Implementation pending
    return {};
}

std::vector<std::pair<std::string, std::vector<Tag>>> SourceMap::get_ranked_tags_map(
    const std::vector<std::string> &chat_fnames,
    const std::vector<std::string> &other_fnames,
    const std::vector<std::string> &mentioned_fnames,
    const std::vector<std::string> &mentioned_idents,
    bool force_refresh
)
{
    // Implementation pending
    return {};
}

std::vector<std::string> SourceMap::locateSourceFiles()
{
    std::vector<std::string> sourceFiles;
    std::filesystem::path currentDir(m_workingDir);

    // Helper function to check if a file is a source file
    auto isSourceFile=[](const std::filesystem::path &path)
        {
            std::string ext=path.extension();
            return isSourceFile(ext);
        };

    // Traverse directory recursively
    for(const auto &entry:std::filesystem::directory_iterator(currentDir))
    {
        if(entry.is_regular_file()&&isSourceFile(entry.path()))
        {
            sourceFiles.push_back(entry.path().string());
        }
    }

    return sourceFiles;
}

}//namespace cronus