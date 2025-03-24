#include "cronus/agents/coder.h"
#include "cronus/logger.h"
#include "cronus/config.h"
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <cmath>

namespace cronus
{
namespace agents
{

Coder::Coder(std::shared_ptr<SourceMap> sourceMap)
    : m_sourceMap(sourceMap)
{
    // Get the model info to set the max tokens
    const auto& config = Config::instance();
    auto modelConfig = config.getModelConfig(config.getModel());
    
    if (modelConfig) {
        // Set max tokens based on the model's context window
        m_maxTokens = modelConfig->max_input_tokens > 0 ? 
                      modelConfig->max_input_tokens : 4096;
        
        logInfo("Coder agent initialized with max tokens: " + std::to_string(m_maxTokens));
    } else {
        logInfo("Coder agent initialized with default max tokens: " + std::to_string(m_maxTokens));
    }
}

std::string Coder::generateCode(const std::string &description,
    const std::vector<std::string> &context)
{
    // Extract context if not provided
    std::vector<std::string> contextToUse=context;
    if(contextToUse.empty())
    {
        contextToUse=extractContext(description);
    }

    // Build context string from files
    std::stringstream contextStr;
    for(const auto &ctx:contextToUse)
    {
        std::filesystem::path filePath(ctx);
        if (std::filesystem::exists(filePath)) {
            std::ifstream file(filePath);
            if (file) {
                contextStr << "File: " << filePath.filename().string() << "\n";
                contextStr << "```\n";
                contextStr << std::string(std::istreambuf_iterator<char>(file), 
                                         std::istreambuf_iterator<char>());
                contextStr << "\n```\n\n";
            }
        }
    }
    
    // Add user request to history
    std::string userMessage = "Generate code for: " + description;
    if (!contextToUse.empty()) {
        userMessage += "\n\nContext:\n" + contextStr.str();
    }
    addToHistory("user", userMessage);

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
    
    // Add assistant response to history
    addToHistory("assistant", result.str());

    return result.str();
}

std::string Coder::explainCode(const std::string &code)
{
    // Add user request to history
    addToHistory("user", "Explain this code:\n```\n" + code + "\n```");
    
    // TODO: Implement code explanation using LLM
    // This is a placeholder implementation
    std::string explanation = "This code appears to be a function definition. It declares a function "
        "that doesn't return a value and has no parameters.";
    
    // Add assistant response to history
    addToHistory("assistant", explanation);
    
    return explanation;
}

std::string Coder::suggestRefactoring(const std::string &code, const std::string &goal)
{
    // Add user request to history
    addToHistory("user", "Suggest refactoring for this code with the goal of " + goal + ":\n```\n" + code + "\n```");
    
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
    
    // Add assistant response to history
    addToHistory("assistant", result.str());

    return result.str();
}

std::vector<std::string> Coder::identifyBugs(const std::string &code)
{
    // Add user request to history
    addToHistory("user", "Identify bugs in this code:\n```\n" + code + "\n```");
    
    // TODO: Implement bug identification using LLM
    // This is a placeholder implementation
    std::vector<std::string> bugs;
    bugs.push_back("No bugs identified yet. This is a placeholder implementation.");
    
    // Add assistant response to history
    std::stringstream result;
    result << "Bugs found:\n";
    for (const auto& bug : bugs) {
        result << "- " << bug << "\n";
    }
    addToHistory("assistant", result.str());
    
    return bugs;
}

std::string Coder::generateTests(const std::string &code, const std::string &framework)
{
    // Add user request to history
    addToHistory("user", "Generate tests for this code using " + framework + " framework:\n```\n" + code + "\n```");
    
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
    
    // Add assistant response to history
    addToHistory("assistant", result.str());

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

void Coder::addToHistory(const std::string& role, const std::string& content)
{
    // Estimate token count for this message
    size_t tokenCount = estimateTokenCount(content);
    
    // Add to history
    m_chatHistory.push_back({role, content, tokenCount});
    m_totalTokens += tokenCount;
    
    // Trim history if it exceeds max tokens
    while (m_totalTokens > m_maxTokens && m_chatHistory.size() > 1) {
        m_totalTokens -= m_chatHistory.front().tokenCount;
        m_chatHistory.pop_front();
    }
    
    logInfo("Chat history updated. Current token count: " + std::to_string(m_totalTokens));
}

size_t Coder::estimateTokenCount(const std::string& text) const
{
    // Simple estimation: ~4 characters per token on average
    // This is a rough approximation, actual tokenization depends on the model
    constexpr double CHARS_PER_TOKEN = 4.0;
    return static_cast<size_t>(std::ceil(text.length() / CHARS_PER_TOKEN));
}

std::vector<hermes::Message> Coder::getChatHistoryForLLM() const
{
    std::vector<hermes::Message> messages;
    
    // Convert our internal chat history to the format expected by Hermes
    for (const auto& msg : m_chatHistory) {
        messages.push_back({msg.role, msg.content});
    }
    
    return messages;
}

} // namespace agents
} // namespace cronus
