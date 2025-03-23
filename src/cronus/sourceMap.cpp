#include "sourceMap.h"
#include <dirent.h>
#include <random>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stack>

// Tree-sitter language parsers
extern "C"
{
    TSLanguage *tree_sitter_c();
    TSLanguage *tree_sitter_cpp();
    TSLanguage *tree_sitter_c_sharp();
    TSLanguage *tree_sitter_css();
    TSLanguage *tree_sitter_go();
    TSLanguage *tree_sitter_html();
    TSLanguage *tree_sitter_java();
    TSLanguage *tree_sitter_javascript();
    TSLanguage *tree_sitter_json();
    TSLanguage *tree_sitter_php();
    TSLanguage *tree_sitter_python();
    TSLanguage *tree_sitter_ruby();
    TSLanguage *tree_sitter_rust();
    TSLanguage *tree_sitter_toml();
    TSLanguage *tree_sitter_typescript();
    TSLanguage *tree_sitter_yaml();
}

namespace cronus
{

bool isSourceFile(const std::filesystem::path &path)
{
    static const std::vector<std::string> sourceExts={
        ".cpp", ".h", ".hpp", ".cxx", ".cc",
        ".py", ".java"
    };

    std::string ext=path.extension().string();
    return std::find(sourceExts.begin(), sourceExts.end(), ext)!=sourceExts.end();
}

TreeSitterParser treeSitterParsers[]={
    { { ".c" }, tree_sitter_c() },
    { { ".cpp", ".h", ".cxx", ".hpp" }, tree_sitter_cpp() },
    { { ".cs" }, tree_sitter_c_sharp() },
    { { ".css" }, tree_sitter_css() },
    { { ".go" }, tree_sitter_go() },
    { { ".html" }, tree_sitter_html() },
    { { ".java" }, tree_sitter_java() },
    { { ".js", ".javascript" }, tree_sitter_javascript() },
    { { ".json" }, tree_sitter_json() },
    { { ".php" }, tree_sitter_php() },
    { { ".py", ".python" }, tree_sitter_python() },
    { { ".rb" }, tree_sitter_ruby() },
    { { ".rs" }, tree_sitter_rust() },
    { { ".toml" }, tree_sitter_toml() },
    { { ".ts" }, tree_sitter_typescript() },
    { { ".yml", ".yaml" }, tree_sitter_yaml() }
};

Tag::Type getTagType(std::string &nodeType)
{
    if(nodeType=="function_definition")
        return Tag::Type::Function;
    else if(nodeType=="class_definition")
        return Tag::Type::Class;
    else if(nodeType=="identifier")
        return Tag::Type::Identifier;
    return Tag::Type::Unset;
}

TSLanguage *getTreeSitterParser(const std::string &ext)
{
    for(auto &parser:treeSitterParsers)
    {
        if(std::find(parser.ext.begin(), parser.ext.end(), ext)!=parser.ext.end())
        {
            return parser.language;
        }
    }
    return nullptr;
}

bool canParseWithTreeSitter(const std::string &ext)
{
    return getTreeSitterParser(ext)!=nullptr;
}

SourceMap::SourceMap(std::string &workingDir)
{
    // Nothing to load yet
    m_maxMapTokens=1024;
    m_maxContextWindow=0;
    m_mapMulNoFiles=8;
    m_verbose=false;
    m_repoContentPrefix="";
}

SourceMap::~SourceMap()
{
    m_fileCache.clear();
}

bool SourceMap::parseWithTreeSitter(FileTags &fileTags, const std::string &fileName)
{
    std::filesystem::path path(fileName);
    std::string ext=path.extension().string();
    std::string content;

    TSLanguage *language=getTreeSitterParser(ext);

    if(!language)
        return false;

    // Read file content
    std::ifstream file(path);

    if(!file.is_open())
    {
        return false;
    }
    content=std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    fileTags.m_tags.clear();

    // Create parser and parse the file
    TSParser *parser=ts_parser_new();
    ts_parser_set_language(parser, language);
    TSTree *tree=ts_parser_parse_string(parser, nullptr, content.c_str(), content.size());
    TSNode root_node=ts_tree_root_node(tree);

    // Walk the syntax tree and collect identifiers
    std::vector<TSNode> nodes;

    nodes.push_back(root_node);
    while(!nodes.empty())
    {
        TSNode node=nodes.back();

        nodes.pop_back();

        if(ts_node_is_null(node)) continue;

        // Get node information
        const char *nodeType=ts_node_type(node);
        TSPoint startPoint=ts_node_start_point(node);
        TSPoint endPoint=ts_node_end_point(node);

        std::string nodeTypeStr(nodeType);
        Tag::Type type=getTagType(nodeTypeStr);
        // Collect important identifiers (functions, classes, variables)
        if(type!=Tag::Type::Unset)
        {
            uint32_t start=ts_node_start_byte(node);
            uint32_t end=ts_node_end_byte(node);

            fileTags.m_tags.emplace_back(Tag{
                content.substr(start, end-start),
                type,
                static_cast<int>(startPoint.row+1),
                static_cast<int>(endPoint.row)
                });
        }

        uint32_t child_count=ts_node_child_count(node);
        for(int32_t i=child_count-1; i>=0; --i)
        {
            nodes.push_back(ts_node_child(node, i));
        }
    }

    // Clean up
    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return true;
};

bool SourceMap::parseFile(FileTags &fileTags, const std::string &fileName)
{
    if(canParseWithTreeSitter(fileName))
    {
        return parseWithTreeSitter(fileTags, fileName);
    }

    return false;
}

std::vector<std::string> SourceMap::locateSourceFiles()
{
    std::vector<std::string> sourceFiles;
    std::filesystem::path currentDir(m_workingDir);

    // Traverse directory recursively
    for(const auto &entry:std::filesystem::directory_iterator(currentDir))
    {
        if(entry.is_regular_file()&&isSourceFile(entry.path()))
            sourceFiles.push_back(entry.path().string());
    }

    return sourceFiles;
}

void SourceMap::update()
{
    std::filesystem::path currentDir(m_workingDir);
    std::vector<std::string> cachedFiles;
    std::vector<std::string> updatedFiles;

    for(const auto &fileEntry:m_fileCache)
    {
        cachedFiles.push_back(fileEntry.first);
    }

    for(const auto &entry:std::filesystem::directory_iterator(currentDir))
    {
        if(!entry.is_regular_file())
            continue;

        auto iter=m_fileCache.find(entry.path().string());
        int time=getMTime(entry.path().string());

        if(iter!=m_fileCache.end())
        {
            if(time!=iter->second.m_time)
            {
                iter->second.m_time=time;

                if(iter->second.m_isSource)
                {
                    parseFile(iter->second, entry.path().string());
                    updatedFiles.push_back(entry.path().string());
                }
            }
        }
        else
        {
            bool isSource=isSourceFile(entry.path());

            FileTags newTags;
            newTags.m_fileName=entry.path().string();
            newTags.m_relativeFileName=getRelativeFname(entry.path().string());
            newTags.m_time=time;
            newTags.m_isSource=isSource;
            auto [it, inserted]=m_fileCache.insert({ entry.path().string(), std::move(newTags) });
            iter=it;

            if(isSource)
            {
                parseFile(iter->second, entry.path().string());
                updatedFiles.push_back(entry.path().string());
            }
        }

        auto fileIter=std::find(cachedFiles.begin(), cachedFiles.end(), entry.path().string());
        if(fileIter!=cachedFiles.end())
        {
            cachedFiles.erase(fileIter);
        }
    }

    // Handle deleted files
    if(!cachedFiles.empty())
    {
        for(const auto &file:cachedFiles)
        {
            auto iter=m_fileCache.find(file);

            m_fileCache.erase(file);
            updatedFiles.push_back(file);
        }
    }

    if(!updatedFiles.empty())
    {
        saveToCache();
    }
}

void SourceMap::saveToCache()
{
    saveTagsCache();
}


void SourceMap::ensureCacheDirectory()
{
    std::filesystem::path cachePath=getCachePath();
    if(!std::filesystem::exists(cachePath))
    {
        std::filesystem::create_directories(cachePath);
    }
}

std::filesystem::path SourceMap::getCachePath() const
{
    return std::filesystem::path(".cronus")/"cache";
}

void SourceMap::loadFromCache()
{
    std::filesystem::path cacheDir=getCachePath();

    if(!std::filesystem::exists(cacheDir))
        return;

    for(const auto &entry:std::filesystem::recursive_directory_iterator(cacheDir))
    {
        if(!entry.is_regular_file()||entry.path().extension()!=".cache")
            continue;

        std::ifstream cache(entry.path());
        
        if(!cache.is_open())
            continue;

        std::string line;

        if(std::getline(cache, line))
        {
            std::istringstream iss(line);
            FileTags tags;

            std::getline(iss, tags.m_fileName, '|');
            std::getline(iss, tags.m_relativeFileName, '|');
            iss>>tags.m_time;
            iss.ignore();
            iss>>tags.m_isSource;

            size_t tagCount;
            iss>>tagCount;

            for(size_t i=0; i<tagCount; ++i)
            {
                Tag tag;
                std::getline(iss, tag.name, '|');
                int typeInt;
                iss>>typeInt;
                tag.type=static_cast<Tag::Type>(typeInt);
                iss>>tag.start;
                iss>>tag.end;
                tags.m_tags.push_back(tag);
            }

            m_fileCache[tags.m_fileName]=std::move(tags);
        }
    }
}

void SourceMap::updateCachedFile(const std::string &updatedFile, FileTags &tags)
{
    std::filesystem::path cachePath=getCachePath()/(tags.m_relativeFileName+".cache");

    std::ofstream cache(cachePath);

    if(!cache.is_open())
    {
        return;
    }

    cache<<tags.m_fileName<<'|'
        <<tags.m_relativeFileName<<'|'
        <<tags.m_time<<' '
        <<tags.m_isSource<<' '
        <<tags.m_tags.size();

    for(const auto &tag:tags.m_tags)
    {
        cache<<' '<<tag.name<<'|'
            <<static_cast<int>(tag.type)<<' '
            <<tag.start<<' '
            <<tag.end;
    }
    cache<<'\n';
}

void SourceMap::updateCachedFiles(const std::vector<std::string> &updatedFiles)
{
    for(const auto &file:updatedFiles)
    {
        auto iter=m_fileCache.find(file);

        if(iter!=m_fileCache.end())
            updateCachedFile(file, iter->second);
    }
}

std::string SourceMap::getRelativeFname(const std::string &fileName)
{
    std::error_code ec;
    auto relPath=std::filesystem::relative(fileName, m_workingDir, ec);

    if(ec)
    {
        return fileName;
    }
    return relPath.string();
}

void SourceMap::tagsCacheError(const std::string &error)
{
    if(m_verbose)
    {
        std::cout<<"Tags cache error: "<<error<<std::endl;
    }
    
    // Try to load the cache first
    loadTagsCache();
    
    // If cache is empty or failed to load, generate it
    if (m_fileCache.empty())
    {
        std::cout<<"Cache not found or empty. Generating source map..."<<std::endl;
        update();
        std::cout<<"Source map generated and saved to cache."<<std::endl;
    }
}

void SourceMap::loadTagsCache()
{
    loadFromCache();
}

void SourceMap::saveTagsCache()
{
    ensureCacheDirectory();

    for(const auto &[path, tags]:m_fileCache)
    {
        std::filesystem::path cachePath=getCachePath()/(tags.m_relativeFileName+".cache");

        // Create subdirectories if needed
        std::filesystem::create_directories(cachePath.parent_path());

        std::ofstream cache(cachePath);
        if(!cache.is_open())
        {
            continue;
        }

        cache<<tags.m_fileName<<'|'
            <<tags.m_relativeFileName<<'|'
            <<tags.m_time<<' '
            <<tags.m_isSource<<' '
            <<tags.m_tags.size();

        for(const auto &tag:tags.m_tags)
        {
            cache<<' '<<tag.name<<'|'
                <<static_cast<int>(tag.type)<<' '
                <<tag.start<<' '
                <<tag.end;
        }
        cache<<'\n';
    }
}

int SourceMap::getMTime(const std::string &fileName)
{
    std::filesystem::path filePath(fileName);

    if(!std::filesystem::exists(filePath))
    {
        std::cout<<"File not found error: "<<fileName<<std::endl;
        return -1;
    }
    return std::filesystem::last_write_time(filePath).time_since_epoch().count();
}

std::vector<Tag> SourceMap::getTags(const std::string &fname, const std::string &rel_fname)
{
    // Implementation pending
    return {};
}

std::vector<std::pair<std::string, std::vector<Tag>>> SourceMap::getRankedTagsMap(
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


}//namespace cronus
