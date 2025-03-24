#include "cronus/agents/coder.h"
#include "cronus/logger.h"
#include <algorithm>
#include <filesystem>
#include <sstream>

namespace cronus
{
namespace agents
{

Coder::Coder(std::shared_ptr<SourceMap> sourceMap)
    : m_sourceMap(sourceMap)
{
    logInfo("Coder agent initialized");
}

std::string Coder::generateCode(const std::string &description,
    const std::vector<std::string> &context)
{
    // Extract context if not provided
    std::vector<std::string> contextToUse=context;s
    if(contextToUse.empty())
    {
        contextToUse=extractContext(description);
    }

    // TODO: Implement actual code generation using LLM
    // This is a placeholder implementation
    std::stringstream result;
    result<<"// Generated code based on: "<<description<<"\n";
    result<<"// Context files used: ";

    for(const auto &ctx:contextToUse)
    {
        result<<std::filesystem::path(ctx).filename().string()<<" ";
    }

    result<<"\n\n";
    result<<"// TODO: Implement the actual functionality\n";
    result<<"void generatedFunction() {\n";
    result<<"    // Implementation pending\n";
    result<<"}\n";

    return result.str();
}

std::string Coder::explainCode(const std::string &code)
{
    // TODO: Implement code explanation using LLM
    // This is a placeholder implementation
    return "This code appears to be a function definition. It declares a function "
        "that doesn't return a value and has no parameters.";
}

std::string Coder::suggestRefactoring(const std::string &code, const std::string &goal)
{
    // TODO: Implement refactoring suggestions using LLM
    // This is a placeholder implementation
    std::stringstream result;
    result<<"// Original code:\n";
    result<<code<<"\n\n";
    result<<"// Refactored code for goal: "<<goal<<"\n";
    result<<"// TODO: Implement actual refactoring\n";
    result<<code<<"\n\n";
    result<<"// Explanation of changes:\n";
    result<<"// No changes made yet. This is a placeholder implementation.";

    return result.str();
}

std::vector<std::string> Coder::identifyBugs(const std::string &code)
{
    // TODO: Implement bug identification using LLM
    // This is a placeholder implementation
    std::vector<std::string> bugs;
    bugs.push_back("No bugs identified yet. This is a placeholder implementation.");
    return bugs;
}

std::string Coder::generateTests(const std::string &code, const std::string &framework)
{
    // TODO: Implement test generation using LLM
    // This is a placeholder implementation
    std::stringstream result;
    result<<"// Generated tests for code using framework: "<<framework<<"\n";
    result<<"// TODO: Implement actual test generation\n";
    result<<"#include <gtest/gtest.h>\n\n";
    result<<"TEST(GeneratedTest, BasicFunctionality) {\n";
    result<<"    // TODO: Implement test\n";
    result<<"    EXPECT_TRUE(true);\n";
    result<<"}\n";

    return result.str();
}

std::vector<std::string> Coder::extractContext(const std::string &description)
{
    std::vector<std::string> context;

    // Extract potential file references from the description
    std::istringstream iss(description);
    std::string word;

    while(iss>>word)
    {
        // Look for words that might be file references
        if(word.find('.')!=std::string::npos)
        {
            // Use the source map to find matching files
            std::vector<std::string> matchingFiles=m_sourceMap->findFilesByPartialName(word);
            if(!matchingFiles.empty())
            {
                // Add all matching files to the context
                context.insert(context.end(), matchingFiles.begin(), matchingFiles.end());
            }
        }
    }

    // Look for code identifiers (functions, classes, etc.)
    for(const auto &[path, fileTags]:m_sourceMap->getFileCache())
    {
        for(const auto &tag:fileTags.m_tags)
        {
            if(description.find(tag.name)!=std::string::npos)
            {
                // Add files containing mentioned identifiers
                context.push_back(path);
                break; // Only add each file once
            }
        }
    }

    // Remove duplicates
    std::sort(context.begin(), context.end());
    context.erase(std::unique(context.begin(), context.end()), context.end());

    return context;
}

std::string Coder::formatCode(const std::string &code, const std::string &language)
{
    // TODO: Implement code formatting based on project standards
    // This is a placeholder implementation
    return code;
}

} // namespace agents
} // namespace cronus
