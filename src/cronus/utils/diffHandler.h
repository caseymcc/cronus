#ifndef _cronus_utils_diffHandler_h_
#define _cronus_utils_diffHandler_h_

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace cronus {
namespace utils {

/**
 * @class DiffHandler
 * @brief Utility for parsing and applying diffs to files
 *
 * This class provides functionality to parse unified diff format
 * and apply the changes to existing files or strings.
 */
class DiffHandler {
public:
    /**
     * @brief Apply a diff to a file
     * @param filePath Path to the file to modify
     * @param diffContent The diff content in unified format
     * @return True if successful, false otherwise
     */
    static bool applyToFile(const std::filesystem::path& filePath, const std::string& diffContent);
    
    /**
     * @brief Apply a diff to a string
     * @param content The original content to modify
     * @param diffContent The diff content in unified format
     * @return The modified content if successful, nullopt otherwise
     */
    static std::optional<std::string> applyToString(const std::string& content, const std::string& diffContent);
    
    /**
     * @brief Create a diff between two strings
     * @param original The original content
     * @param modified The modified content
     * @return A diff in unified format
     */
    static std::string createDiff(const std::string& original, const std::string& modified);

private:
    /**
     * @brief Parse a unified diff into hunks
     * @param diffContent The diff content
     * @return Vector of parsed hunks
     */
    static std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> parseDiff(const std::string& diffContent);
    
    /**
     * @brief Find the position in content where a hunk should be applied
     * @param content The content to search in
     * @param hunk The hunk to find position for
     * @return The line number where the hunk should be applied, or -1 if not found
     */
    static int findHunkPosition(const std::vector<std::string>& contentLines, 
                               const std::vector<std::string>& hunkLines);
};

} // namespace utils
} // namespace cronus

#endif // _cronus_utils_diffHandler_h_
