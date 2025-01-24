#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>

namespace cronus {

struct Message {
    std::string role;
    std::string content;
};

struct CompletionRequest {
    std::string model;
    std::vector<Message> messages;
    float temperature = 0.7f;
    int max_tokens = 1000;
};

struct CompletionResponse {
    std::string text;
    std::string model;
    int tokens_used;
};

class LLMClient {
public:
    virtual ~LLMClient() = default;
    virtual CompletionResponse complete(const CompletionRequest& request) = 0;
};

class OpenAIClient : public LLMClient {
public:
    OpenAIClient(const std::string& api_key);
    CompletionResponse complete(const CompletionRequest& request) override;

private:
    std::string api_key_;
};

class AnthropicClient : public LLMClient {
public:
    AnthropicClient(const std::string& api_key);
    CompletionResponse complete(const CompletionRequest& request) override;

private:
    std::string api_key_;
};

} // namespace cronus
