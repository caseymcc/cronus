#ifndef _llm_hermes_hermes_h_
#define _llm_hermes_hermes_h_

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <optional>

namespace llm_hermes
{

enum class ErrorCode {
    Success = 0,
    ApiKeyNotFound,
    UnknownModel,
    UnsupportedProvider,
    NetworkError,
    InvalidResponse,
    InvalidRequest
};

struct Message {
    std::string role;
    std::string content;
};

struct CompletionRequest {
    std::string model;           // e.g., "gpt-3.5-turbo", "claude-2"
    std::vector<Message> messages;
    std::optional<float> temperature;
    std::optional<int> max_tokens;
    std::optional<std::string> api_key;  // Optional override of env var
};

struct CompletionResponse {
    std::string text;
    std::string model;
    int tokens_used;
    std::string provider;  // "openai", "anthropic", etc.
};

// Main completion function (similar to litellm.completion)
ErrorCode completion(const CompletionRequest& request, CompletionResponse& response);


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
