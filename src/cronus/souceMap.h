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
        int mTime;
        std::vector<Tag> mData;
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
        std::mutex mCacheMutex;
        std::unordered_map<std::string, CacheItem> mTagsCache;
        std::string mRoot;
        int mMaxMapTokens;
        int mMaxContextWindow;
        int mMapMulNoFiles;
        bool mVerbose;
        std::string mRepoContentPrefix;
    };
} // namespace cronus

#endif// _cronus_sourceMap_h_
