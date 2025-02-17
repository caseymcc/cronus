#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <filesystem>
#include <optional>

namespace cronus {
    struct Tag {
        std::string rel_fname;
        std::string fname;
        std::string name;
        std::string kind;
        int line;
    };

    class RepoMap {
    public:
        RepoMap(const std::string& root = "", 
                int map_tokens = 1024,
                int max_context_window = 0,
                int map_mul_no_files = 8,
                bool verbose = false,
                std::string repo_content_prefix = "");

        ~RepoMap();

        std::string get_rel_fname(const std::string& fname);
        
        void tags_cache_error(const std::string& error = "");
        
        void load_tags_cache();
        
        void save_tags_cache();

        int get_mtime(const std::string& fname);
        
        std::vector<Tag> get_tags(const std::string& fname, const std::string& rel_fname);
        
        std::vector<std::pair<std::string, std::vector<Tag>>> get_ranked_tags_map(
            const std::vector<std::string>& chat_fnames,
            const std::vector<std::string>& other_fnames,
            const std::vector<std::string>& mentioned_fnames,
            const std::vector<std::string>& mentioned_idents,
            bool force_refresh = false
        );

    private:
        struct CacheItem {
            int mtime;
            std::vector<Tag> data;
        };

        std::mutex m_cache_mutex;
        std::unordered_map<std::string, CacheItem> m_tags_cache;
        std::string m_root;
        int m_max_map_tokens;
        int m_max_context_window;
        int m_map_mul_no_files;
        bool m_verbose;
        std::string m_repo_content_prefix;
    };
}
