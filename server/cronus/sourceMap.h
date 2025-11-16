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

struct DirectoryEntry {
    std::string name;
    std::string path;
    bool isDirectory;
    std::vector<std::string> children; // Paths to child entries
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

    std::vector<Tag> getTags(const std::string &fname, const std::string &rel_fname);
    
    // Get access to the file cache
    const std::unordered_map<std::string, FileTags>& getFileCache() const { return m_fileCache; }
    
    // Directory structure functions
    void updateDirectoryStructure();
    const std::unordered_map<std::string, DirectoryEntry>& getDirectoryStructure() const { 
        return m_directoryStructure; 
    }
    
    // Search for files containing part of the filename
    std::vector<std::string> findFilesByPartialName(const std::string &partialName) const;

    std::vector<std::pair<std::string, std::vector<Tag>>> getRankedTagsMap(
        const std::vector<std::string> &chat_fnames,
        const std::vector<std::string> &other_fnames,
        const std::vector<std::string> &mentioned_fnames,
        const std::vector<std::string> &mentioned_idents,
        bool force_refresh=false
    );

    void update();

private:
    std::vector<std::string> locateSourceFiles();

    bool parseWithTreeSitter(const std::string &path, FileTags &fileTags);
    bool parseFile(const std::string &fileName, FileTags &fileTags);

    void ensureCacheDirectory();
    std::filesystem::path getCachePath() const;
    void loadFromCache();
    void updateCachedFile(const std::string &updatedFile, FileTags &tags);
    void updateCachedFiles(const std::vector<std::string> &updatedFiles);
    void saveToCache();

    std::mutex m_cacheMutex;
    std::unordered_map<std::string, FileTags> m_fileCache;
    std::unordered_map<std::string, DirectoryEntry> m_directoryStructure;
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
