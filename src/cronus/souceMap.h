#ifndef _cronus_sourceMap_h_
#define _cronus_sourceMap_h_

#include <vector>
#include <string>
#include <filesystem>
#include <mutex>

namespace cronus
{
    struct Tag
    {
        std::string rel_fname;
        std::string fname;
        std::string name;
        std::string kind;
        int line;
    };

    struct CacheItem
    {
        int m_time;
        std::vector<Tag> m_data;
    };

    class SourceMap
    {
    public:
        SourceMap(std::string &workingDir);
        ~SourceMap();

        std::string getRelFname(const std::string &fname);
    
        
        
        void tagsCacheError(const std::string &error = "");
        
        void loadTagsCache();
        
        void saveTagsCache();
        
        int getMTime(const std::string &fname);
        
        std::vector<Tag> get_tags(const std::string &fname, const std::string &rel_fname);
        
        std::vector<std::pair<std::string, std::vector<Tag>>> get_ranked_tags_map(
            const std::vector<std::string> &chat_fnames,
            const std::vector<std::string> &other_fnames,
            const std::vector<std::string> &mentioned_fnames,
            const std::vector<std::string> &mentioned_idents,
            bool force_refresh = false
        );

    private:
       std::vector<std::string> locateSourceFiles();
    
       void update();

        std::mutex m_cache_mutex;
        std::unordered_map<std::string, CacheItem> m_tags_cache;
        std::string m_workingDir;
        int m_maxMapTokens;
        int m_maxContextWindow;
        int m_mapMulNoFiles;
        bool m_verbose;
        std::string m_repo_content_prefix;
        time_t lastUpdate;
    };
} // namespace cronus

#endif// _cronus_sourceMap_h_
