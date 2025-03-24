#include "cronus/utils/diffHandler.h"
#include "cronus/logger.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace cronus;
using namespace cronus::utils;

// Simple test function to demonstrate DiffHandler usage
void testDiffHandler() {
    // Example original content
    std::string original = 
        "Line 1\n"
        "Line 2\n"
        "Line 3\n"
        "Line 4\n"
        "Line 5\n";
    
    // Example diff that removes Line 3 and adds two new lines
    std::string diff = 
        "--- original\n"
        "+++ modified\n"
        "@@ -2,3 +2,4 @@\n"
        " Line 2\n"
        "-Line 3\n"
        "+Line 3 modified\n"
        "+New line inserted\n"
        " Line 4\n";
    
    // Apply the diff to the string
    auto result = DiffHandler::applyToString(original, diff);
    
    if (result) {
        logInfo("Diff applied successfully");
        logInfo("Original content:\n" + original);
        logInfo("Modified content:\n" + *result);
    } else {
        logError("Failed to apply diff");
    }
    
    // Create a temporary file for testing file operations
    std::string tempFilePath = "temp_diff_test.txt";
    std::ofstream tempFile(tempFilePath);
    tempFile << original;
    tempFile.close();
    
    // Apply the diff to the file
    if (DiffHandler::applyToFile(tempFilePath, diff)) {
        logInfo("Diff applied to file successfully");
        
        // Read and display the modified file
        std::ifstream modifiedFile(tempFilePath);
        std::string modifiedContent((std::istreambuf_iterator<char>(modifiedFile)), 
                                   std::istreambuf_iterator<char>());
        modifiedFile.close();
        
        logInfo("File content after diff:\n" + modifiedContent);
    } else {
        logError("Failed to apply diff to file");
    }
    
    // Clean up the temporary file
    std::filesystem::remove(tempFilePath);
    
    // Test creating a diff
    std::string modified = 
        "Line 1\n"
        "Line 2\n"
        "Line 3 modified\n"
        "New line inserted\n"
        "Line 4\n"
        "Line 5\n";
    
    std::string generatedDiff = DiffHandler::createDiff(original, modified);
    logInfo("Generated diff:\n" + generatedDiff);
    
    // Apply the generated diff to verify it works
    auto verifyResult = DiffHandler::applyToString(original, generatedDiff);
    if (verifyResult && *verifyResult == modified) {
        logInfo("Generated diff verified successfully");
    } else {
        logError("Generated diff verification failed");
    }
}

// Uncomment to run the test
/*
int main() {
    testDiffHandler();
    return 0;
}
*/
