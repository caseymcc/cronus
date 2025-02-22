#include "sourceMap.h"
#include <dirent.h>
#include <sys/stat.h>

#include <sqlite3.h>
#include <random>
#include <algorithm>
#include <chrono>
#include <networkx/pagerank.h>
#include <tree_sitter/api.h>
#include <filesystem>
#include <fstream>

// Tree-sitter language parsers
extern "C" TSLanguage *tree_sitter_cpp();
extern "C" TSLanguage *tree_sitter_python();
extern "C" TSLanguage *tree_sitter_java();

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

std::vector<std::string> SourceMap::locateSourceFiles() {
    std::vector<std::string> sourceFiles;
    std::filesystem::path currentDir(m_workingDir);

    // Helper function to parse files with Tree-sitter
    auto parseWithTreeSitter = [](const std::filesystem::path& path) {
        std::string ext = path.extension().string();
        std::string content;
        TSLanguage* language = nullptr;

        // Map file extensions to Tree-sitter parsers
        if (ext == ".cpp" || ext == ".h" || ext == ".cxx" || ext == ".hpp") {
            language = tree_sitter_cpp();
        } else if (ext == ".py") {
            language = tree_sitter_python();
        } else if (ext == ".java") {
            language = tree_sitter_java();
        }

        if (!language) {
            return std::make_pair(std::vector<Tag>(), path.last_write_time().count());
        }

        // Read file content
        try {
            std::ifstream file(path);
            content = std::string(std::istreambuf_iterator(file.rdbuf()), '\0');
        } catch (const std::ifstream::failure& e) {
            return std::make_pair(std::vector<Tag>(), path.last_write_time().count());
        }

        // Create parser and parse the file
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, language);
        TSTree* tree = ts_parser_parse_string(parser, nullptr, content.c_str(), content.size());
        TSNode root_node = ts_tree_root_node(tree);

        // Walk the syntax tree and collect identifiers
        std::vector<Tag> tags;
        std::function<void(TSNode)> walk_tree = [&](TSNode node) {
            if (ts_node_is_null(node)) return;

            // Get node information
            const char* node_type = ts_node_type(node);
            TSPoint start_point = ts_node_start_point(node);
            TSPoint end_point = ts_node_end_point(node);

            // Collect important identifiers (functions, classes, variables)
            if (strcmp(node_type, "function_definition") == 0 ||
                strcmp(node_type, "class_definition") == 0 ||
                strcmp(node_type, "identifier") == 0) {
                uint32_t start = ts_node_start_byte(node);
                uint32_t end = ts_node_end_byte(node);
                tags.emplace_back(Tag{
                    .name = content.substr(start, end - start),
                    .line = start_point.row + 1,
                    .col = start_point.column,
                    .type = node_type
                });
            }

            // Recursively walk child nodes
            uint32_t child_count = ts_node_child_count(node);
            for (uint32_t i = 0; i < child_count; i++) {
                walk_tree(ts_node_child(node, i));
            }
        };

        walk_tree(root_node);

        // Clean up
        ts_tree_delete(tree);
        ts_parser_delete(parser);

        return std::make_pair(tags, path.last_write_time().count());
    };

    // Traverse directory recursively
    for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
        if (entry.is_regular_file() && isSourceFile(entry.path())) {
            auto fileInfo = getSourceFileInfo(entry.path());
            sourceFiles.push_back(fileInfo);
        }
    }

    return sourceFiles;
}

void SourceMap::update() {
    std::vector<std::string> currentFiles = locateSourceFiles();
    
    // Compare with cached files
    for (const auto& file : currentFiles) {
        // Check if file exists in cache
        // If not, add it
        // If exists but modified, update it
    }

    // Update last update time
    lastUpdate = time(nullptr);
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
