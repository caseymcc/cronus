#ifndef _cronus_sourceMap_h_
#define _cronus_sourceMap_h_

#include <tree_sitter/api.h>

#include <vector>
#include <string>
#include <filesystem>
#include <mutex>
#include <unordered_map>

namespace cronus
{

struct TreeSitterParser
{
    std::vector<std::string> ext;
    TSLanguage *language;
};


struct Tag
{
    enum class Type
    {
        Unset,
        Function,
        Class,
        Identifier
    };

    std::string name;
    Type type;

    int start;
    int end;

    bool operator!=(const Tag &other) const
    {
        return name!=other.name||
            type!=other.type||
            start!=other.start||
            end!=other.end;
    }
};

Tag::Type getTagType(std::string &nodeType);

struct FileTags
{
    std::string m_fileName;
    std::string m_relativeFileName;
    int m_time;

    bool m_isSource;
    std::vector<Tag> m_tags;
};



TSLanguage *getTreeSitterParser(const std::string &ext);
bool canParseWithTreeSitter(const std::string &ext);

class SourceMap
{
public:
    SourceMap(std::string &workingDir);
    ~SourceMap();

    std::string getRelativeFname(const std::string &fname);

    void tagsCacheError(const std::string &error="");

    void loadTagsCache();

    void saveTagsCache();

    int getMTime(const std::string &fname);

    std::vector<Tag> get_tags(const std::string &fname, const std::string &rel_fname);

    std::vector<std::pair<std::string, std::vector<Tag>>> get_ranked_tags_map(
        const std::vector<std::string> &chat_fnames,
        const std::vector<std::string> &other_fnames,
        const std::vector<std::string> &mentioned_fnames,
        const std::vector<std::string> &mentioned_idents,
        bool force_refresh=false
    );

private:
    std::vector<std::string> locateSourceFiles();

    bool parseWithTreeSitter(FileTags &fileTags, const std::string &path);
    bool parseFile(FileTags &fileTags, const std::string &fileName);

    void update();

    void ensureCacheDirectory();
    std::filesystem::path getCachePath() const;
    void loadFromCache();
    void updateCachedFile(std::string &updatedFile, FileTags &tags);
    void updateCachedFiles(std::vector<std::string> &updatedFiles);
    void saveToCache();
    void saveToCache() { saveTagsCache(); }

    std::mutex m_cache_mutex;
    std::unordered_map<std::string, FileTags> m_fileCache;
    std::string m_workingDir;
    int m_maxMapTokens;
    int m_maxContextWindow;
    int m_mapMulNoFiles;
    bool m_verbose;
    std::string m_repoContentPrefix;
    time_t lastUpdate;
};

} // namespace cronus

#endif// _cronus_sourceMap_h_
