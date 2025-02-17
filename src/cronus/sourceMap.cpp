#include "sourceMap.h"
#include <sqlite3.h>
#include <random>
#include <algorithm>
#include <chrono>
#include <networkx/pagerank.h>

using namespace cronus;

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
    // Implementation pending
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
