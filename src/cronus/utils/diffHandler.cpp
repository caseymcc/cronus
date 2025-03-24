#include "cronus/utils/diffHandler.h"
#include "cronus/logger.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace cronus {
namespace utils {

bool DiffHandler::applyToFile(const std::filesystem::path& filePath, const std::string& diffContent) {
    // Read the file content
    std::ifstream file(filePath);
    if (!file) {
        logError("Failed to open file for applying diff: " + filePath.string());
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Apply the diff to the content
    auto result = applyToString(content, diffContent);
    if (!result) {
        logError("Failed to apply diff to file: " + filePath.string());
        return false;
    }
    
    // Write the modified content back to the file
    std::ofstream outFile(filePath);
    if (!outFile) {
        logError("Failed to open file for writing after diff: " + filePath.string());
        return false;
    }
    
    outFile << *result;
    outFile.close();
    
    logInfo("Successfully applied diff to file: " + filePath.string());
    return true;
}

std::optional<std::string> DiffHandler::applyToString(const std::string& content, const std::string& diffContent) {
    // Split content into lines
    std::vector<std::string> contentLines;
    std::istringstream contentStream(content);
    std::string line;
    
    while (std::getline(contentStream, line)) {
        contentLines.push_back(line);
    }
    
    // Parse the diff into hunks
    auto hunks = parseDiff(diffContent);
    if (hunks.empty()) {
        logWarning("No valid hunks found in diff");
        return content; // Return original if no hunks
    }
    
    // Apply each hunk
    for (const auto& [removeLines, addLines] : hunks) {
        // Find where this hunk should be applied
        int position = findHunkPosition(contentLines, removeLines);
        if (position < 0) {
            logError("Failed to find position for hunk in content");
            return std::nullopt;
        }
        
        // Remove the lines that should be removed
        contentLines.erase(contentLines.begin() + position, 
                          contentLines.begin() + position + removeLines.size());
        
        // Insert the new lines
        contentLines.insert(contentLines.begin() + position, addLines.begin(), addLines.end());
    }
    
    // Join the lines back into a string
    std::ostringstream result;
    for (size_t i = 0; i < contentLines.size(); ++i) {
        result << contentLines[i];
        if (i < contentLines.size() - 1) {
            result << "\n";
        }
    }
    
    return result.str();
}

std::string DiffHandler::createDiff(const std::string& original, const std::string& modified) {
    // Split both strings into lines
    std::vector<std::string> originalLines;
    std::istringstream originalStream(original);
    std::string line;
    
    while (std::getline(originalStream, line)) {
        originalLines.push_back(line);
    }
    
    std::vector<std::string> modifiedLines;
    std::istringstream modifiedStream(modified);
    
    while (std::getline(modifiedStream, line)) {
        modifiedLines.push_back(line);
    }
    
    // Simple diff algorithm (this is a basic implementation)
    // For a real-world application, consider using a more sophisticated diff algorithm
    
    std::ostringstream diffStream;
    diffStream << "--- original\n";
    diffStream << "+++ modified\n";
    
    // Find differences (very basic approach)
    size_t i = 0;
    while (i < originalLines.size() || i < modifiedLines.size()) {
        // Find a block of different lines
        size_t startDiff = i;
        
        while (i < originalLines.size() && i < modifiedLines.size() && 
               originalLines[i] == modifiedLines[i]) {
            i++;
        }
        
        if (i >= originalLines.size() && i >= modifiedLines.size()) {
            break; // End of both files with no differences
        }
        
        // Find the end of the different block
        size_t originalEnd = i;
        while (originalEnd < originalLines.size() && 
               (originalEnd >= modifiedLines.size() || 
                originalLines[originalEnd] != modifiedLines[originalEnd - originalEnd + i])) {
            originalEnd++;
        }
        
        size_t modifiedEnd = i;
        while (modifiedEnd < modifiedLines.size() && 
               (modifiedEnd >= originalLines.size() || 
                originalLines[modifiedEnd - modifiedEnd + i] != modifiedLines[modifiedEnd])) {
            modifiedEnd++;
        }
        
        // Output the hunk header
        diffStream << "@@ -" << (startDiff + 1) << "," << (originalEnd - startDiff) 
                  << " +" << (startDiff + 1) << "," << (modifiedEnd - startDiff) << " @@\n";
        
        // Output the context and changes
        for (size_t j = startDiff; j < originalEnd; j++) {
            diffStream << "-" << originalLines[j] << "\n";
        }
        
        for (size_t j = i; j < modifiedEnd; j++) {
            diffStream << "+" << modifiedLines[j] << "\n";
        }
        
        i = std::max(originalEnd, modifiedEnd);
    }
    
    return diffStream.str();
}

std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> DiffHandler::parseDiff(const std::string& diffContent) {
    std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> hunks;
    
    std::istringstream diffStream(diffContent);
    std::string line;
    
    // Skip header lines (--- and +++)
    while (std::getline(diffStream, line)) {
        if (line.empty() || line[0] != '-' || line[0] != '+') {
            break;
        }
    }
    
    // Parse hunks
    std::vector<std::string> removeLines;
    std::vector<std::string> addLines;
    bool inHunk = false;
    
    // Regular expression to match hunk headers
    std::regex hunkHeaderRegex("@@ -(\\d+),(\\d+) \\+(\\d+),(\\d+) @@");
    
    do {
        // Check if this is a hunk header
        if (line.size() >= 2 && line[0] == '@' && line[1] == '@') {
            // If we were already in a hunk, save it
            if (inHunk) {
                hunks.push_back({removeLines, addLines});
                removeLines.clear();
                addLines.clear();
            }
            
            inHunk = true;
            continue;
        }
        
        // Process hunk content
        if (inHunk && !line.empty()) {
            if (line[0] == '-') {
                // Line to remove
                removeLines.push_back(line.substr(1));
            } else if (line[0] == '+') {
                // Line to add
                addLines.push_back(line.substr(1));
            } else if (line[0] == ' ') {
                // Context line (present in both versions)
                removeLines.push_back(line.substr(1));
                addLines.push_back(line.substr(1));
            }
        }
    } while (std::getline(diffStream, line));
    
    // Save the last hunk if there is one
    if (inHunk && (!removeLines.empty() || !addLines.empty())) {
        hunks.push_back({removeLines, addLines});
    }
    
    return hunks;
}

int DiffHandler::findHunkPosition(const std::vector<std::string>& contentLines, 
                                 const std::vector<std::string>& hunkLines) {
    if (hunkLines.empty()) {
        return 0; // Empty hunk can be applied at the beginning
    }
    
    // Try to find the sequence of lines in the content
    for (size_t i = 0; i <= contentLines.size() - hunkLines.size(); ++i) {
        bool match = true;
        for (size_t j = 0; j < hunkLines.size(); ++j) {
            if (contentLines[i + j] != hunkLines[j]) {
                match = false;
                break;
            }
        }
        
        if (match) {
            return static_cast<int>(i);
        }
    }
    
    return -1; // Not found
}

} // namespace utils
} // namespace cronus
