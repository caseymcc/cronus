#ifndef _llm_hermes_hermes_h
#define _llm_hermes_hermes_h

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <optional>

namespace llm_hermes
{

struct Message
{
    std::string role;
    std::string content;
};

struct CompletionRequest
{
    std::string model;           // e.g., "gpt-3.5-turbo", "claude-2"
    std::vector<Message> messages;
    std::optional<float> temperature;
    std::optional<int> max_tokens;
    std::optional<std::string> api_key;  // Optional override of env var
};

struct CompletionResponse
{
    std::string text;
    std::string model;
    int tokens_used;
    std::string provider;  // "openai", "anthropic", etc.
};

// Main completion function (similar to litellm.completion)
CompletionResponse completion(const CompletionRequest& request);

// Helper to get API key from environment
std::string get_api_key(const std::string& provider);

namespace providers
{
    // Provider-specific implementations
    CompletionResponse openai_completion(const CompletionRequest& request);
    CompletionResponse anthropic_completion(const CompletionRequest& request);
    // Add more providers as needed
}

// Model to provider mapping
const std::map<std::string, std::string> MODEL_PROVIDER_MAP =
{
    {"gpt-3.5-turbo", "openai"},
    {"gpt-4", "openai"},
    {"claude-2", "anthropic"},
    {"claude-instant-1", "anthropic"}
};

}//namespace llm_hermes

#endif//_llm_hermes_hermes_h
