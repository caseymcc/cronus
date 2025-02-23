#include "sourceMap.h"
#include <dirent.h>
#include <sys/stat.h>

#include <sqlite3.h>
#include <random>
#include <algorithm>
#include <chrono>
#include <networkx/pagerank.h>

#include <filesystem>
#include <fstream>

// Tree-sitter language parsers
extern "C" TSLanguage *tree_sitter_cpp();
extern "C" TSLanguage *tree_sitter_python();
extern "C" TSLanguage *tree_sitter_java();

namespace cronus
{

TreeSitterParser treeSitterParsers[]={
    { { ".cpp", ".h", ".cxx", ".hpp" }, tree_sitter_cpp() },
    { { ".py" }, tree_sitter_python() },
    { { ".java" }, tree_sitter_java() }
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
    m_workingDir=workingDir;
    m_maxMapTokens=1024;
    m_maxContextWindow=0;
    m_mapMulNoFiles=8;
    m_verbose=false;
    m_repoContentPrefix="";
}

bool SourceMap::parseWithTreeSitter(FileTags &fileTags, const std::filesystem::path &path)
{
    std::string ext=path.extension().string();
    std::string content;
    TSLanguage *language=nullptr;

    // Map file extensions to Tree-sitter parsers
    if(ext==".cpp"||ext==".h"||ext==".cxx"||ext==".hpp")
    {
        language=tree_sitter_cpp();
    }
    else if(ext==".py")
    {
        language=tree_sitter_python();
    }
    else if(ext==".java")
    {
        language=tree_sitter_java();
    }

    if(!language)
        return false;

    // Read file content
    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }
    content = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

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

        TagType type=getTagType(nodeType);
        // Collect important identifiers (functions, classes, variables)
        if(type!=TagType::Unset)
        {
            uint32_t start=ts_node_start_byte(node);
            uint32_t end=ts_node_end_byte(node);

            fileTags.m_tags.emplace_back(Tag{
                .name=content.substr(start, end-start),
                .start=startPoint.row+1,
                .end=endPoint.row,
                .type=type
                });
        }

        uint32_t child_count = ts_node_child_count(node);
        for (int32_t i = child_count - 1; i >= 0; --i)
        {
            nodeStack.push(ts_node_child(node, i));
        }
    }

    // Clean up
    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return true;
};

bool SourceMap::parseFile(std::string &fileName)
{
    std::unordered_map<std::string, FileTags>::iterator tagsIter=m_tagsCache.find(fileName);

    if(tagsIter == m_tagsCache.end())
    {
        tagsIter=m_tagsCache[fileName].insert({
            .fileName=fileName,
            .relativeFileName=getRelFname(fileName),
            .m_time=getMTime(fileName)
        });
    }

    if(canParseWithTreeSitter(fileName))
        return parseWithTreeSitter(*tagsIter, fileName);

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
        {
            auto fileInfo=getSourceFileInfo(entry.path());
            sourceFiles.push_back(fileInfo);
        }
    }

    return sourceFiles;
}

void SourceMap::update()
{
    std::vector<std::string> currentFiles=locateSourceFiles();

    // Compare with cached files
    for(const auto &file:currentFiles)
    {
        // Check if file exists in cache
        // If not, add it
        // If exists but modified, update it
    }

    // Update last update time
    lastUpdate=time(nullptr);
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


}//namespace cronus
