#include "cronus/agents/coder.h"
#include "cronus/logger.h"
#include "cronus/config.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <cmath>
#include <fstream>

namespace cronus
{
namespace agents
{

Coder::Coder(std::shared_ptr<SourceMap> sourceMap, std::shared_ptr<Model> model)
    : m_sourceMap(sourceMap), m_model(model)
{
    // Set max tokens based on the model's context window
    m_maxTokens = m_model->getMaxTokens();
    
    logInfo("Coder agent initialized with max tokens: " + std::to_string(m_maxTokens));
}

std::string Coder::generateCode(const std::string &description,
    const std::vector<std::string> &context,
    const std::vector<std::string> &addedFiles)
{
    // Extract context if not provided
    std::vector<std::string> contextToUse=context;
    if(contextToUse.empty())
    {
        contextToUse=extractContext(description);
    }
    
    // Add explicitly added files to the context
    for (const auto& file : addedFiles) {
        if (std::find(contextToUse.begin(), contextToUse.end(), file) == contextToUse.end()) {
            contextToUse.push_back(file);
        }
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

    // Use the model to generate code
    std::vector<hermes::Message> messages = getChatHistoryForLLM();
    
    // If chat history is empty, add a system message
    if (messages.empty()) {
        messages.push_back({"system", "You are a coding assistant that helps generate high-quality code based on descriptions and context."});
    }
    
    // Generate code using the model
    std::string generatedCode = m_model->generate(messages);
    
    // If there was an error, return a placeholder
    if (generatedCode.substr(0, 6) == "Error:") {
        logError("Code generation failed: " + generatedCode);
        
        std::stringstream result;
        result<<"// Code generation failed\n";
        result<<"// " << generatedCode << "\n";
        result<<"// Falling back to placeholder implementation\n\n";
        result<<"// TODO: Implement the actual functionality\n";
        result<<"void generatedFunction() {\n";
        result<<"    // Implementation pending\n";
        result<<"}\n";
        
        generatedCode = result.str();
    }
    
    // Add assistant response to history
    addToHistory("assistant", generatedCode);

    return generatedCode;
}

std::string Coder::explainCode(const std::string &code)
{
    // Add user request to history
    addToHistory("user", "Explain this code:\n```\n" + code + "\n```");
    
    // Use the model to explain the code
    std::vector<hermes::Message> messages = getChatHistoryForLLM();
    
    // If chat history is empty, add a system message
    if (messages.empty()) {
        messages.push_back({"system", "You are a coding assistant that helps explain code clearly and concisely."});
    }
    
    // Generate explanation using the model
    std::string explanation = m_model->generate(messages);
    
    // If there was an error, return a placeholder
    if (explanation.substr(0, 6) == "Error:") {
        logError("Code explanation failed: " + explanation);
        explanation = "I couldn't analyze this code due to a technical issue. Please try again later.";
    }
    
    // Add assistant response to history
    addToHistory("assistant", explanation);
    
    return explanation;
}

std::string Coder::suggestRefactoring(const std::string &code, const std::string &goal)
{
    // Add user request to history
    addToHistory("user", "Suggest refactoring for this code with the goal of " + goal + ":\n```\n" + code + "\n```");
    
    // Use the model to suggest refactoring
    std::vector<hermes::Message> messages = getChatHistoryForLLM();
    
    // If chat history is empty, add a system message
    if (messages.empty()) {
        messages.push_back({"system", "You are a coding assistant that helps refactor code to improve quality and meet specific goals."});
    }
    
    // Generate refactoring using the model
    std::string refactoring = m_model->generate(messages);
    
    // If there was an error, return a placeholder
    if (refactoring.substr(0, 6) == "Error:") {
        logError("Code refactoring failed: " + refactoring);
        
        std::stringstream result;
        result<<"// Original code:\n";
        result<<code<<"\n\n";
        result<<"// Refactoring failed due to a technical issue\n";
        result<<"// " << refactoring << "\n";
        result<<"// Explanation of changes:\n";
        result<<"// No changes made due to error.";
        
        refactoring = result.str();
    }
    
    // Add assistant response to history
    addToHistory("assistant", refactoring);

    return refactoring;
}

std::vector<std::string> Coder::identifyBugs(const std::string &code)
{
    // Add user request to history
    addToHistory("user", "Identify bugs in this code:\n```\n" + code + "\n```");
    
    // Use the model to identify bugs
    std::vector<hermes::Message> messages = getChatHistoryForLLM();
    
    // If chat history is empty, add a system message
    if (messages.empty()) {
        messages.push_back({"system", "You are a coding assistant that helps identify bugs and issues in code. List each bug on a separate line starting with '- '."});
    }
    
    // Generate bug list using the model
    std::string bugResponse = m_model->generate(messages);
    
    // Parse the response into individual bugs
    std::vector<std::string> bugs;
    
    if (bugResponse.substr(0, 6) == "Error:") {
        logError("Bug identification failed: " + bugResponse);
        bugs.push_back("Bug identification failed due to a technical issue.");
    } else {
        std::istringstream iss(bugResponse);
        std::string line;
        
        while (std::getline(iss, line)) {
            // Look for lines that start with "- " which indicate a bug
            if (line.size() > 2 && line[0] == '-' && line[1] == ' ') {
                bugs.push_back(line.substr(2));
            }
        }
        
        // If no bugs were found in the expected format, add the whole response
        if (bugs.empty() && !bugResponse.empty()) {
            bugs.push_back("Analysis: " + bugResponse);
        }
    }
    
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
    
    // Use the model to generate tests
    std::vector<hermes::Message> messages = getChatHistoryForLLM();
    
    // If chat history is empty, add a system message
    if (messages.empty()) {
        messages.push_back({"system", "You are a coding assistant that helps generate comprehensive unit tests for code."});
    }
    
    // Generate tests using the model
    std::string tests = m_model->generate(messages);
    
    // If there was an error, return a placeholder
    if (tests.substr(0, 6) == "Error:") {
        logError("Test generation failed: " + tests);
        
        std::stringstream result;
        result<<"// Test generation failed due to a technical issue\n";
        result<<"// " << tests << "\n";
        result<<"// Falling back to placeholder implementation\n\n";
        result<<"// Generated tests for code using framework: "<<framework<<"\n";
        result<<"#include <gtest/gtest.h>\n\n";
        result<<"TEST(GeneratedTest, BasicFunctionality) {\n";
        result<<"    // TODO: Implement test\n";
        result<<"    EXPECT_TRUE(true);\n";
        result<<"}\n";
        
        tests = result.str();
    }
    
    // Add assistant response to history
    addToHistory("assistant", tests);

    return tests;
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
