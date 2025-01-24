#include "llm_hermes/hermes.h"

#include <iostream>

int main()
{
    std::cout << "Cronus Client" << std::endl;

    // Using OpenAI
    llm_hermes::CompletionResponse response;
    llm_hermes::ErrorCode result = llm_hermes::completion(
        {
            .model = "gpt-3.5-turbo",
            .messages = {
                {"user", "Hello, how are you?"}
            }
        },
        response
    );

    if (result != llm_hermes::ErrorCode::Success) {
        std::cerr << "OpenAI completion failed with error code: " << static_cast<int>(result) << std::endl;
        return 1;
    }
    std::cout << "OpenAI Response: " << response.text << std::endl;

    // Using Anthropic
    llm_hermes::CompletionResponse claude_response;
    result = llm_hermes::completion(
        {
            .model = "claude-2",
            .messages = {
                {"user", "Hello, how are you?"}
            }
        },
        claude_response
    );

    if (result != llm_hermes::ErrorCode::Success) {
        std::cerr << "Anthropic completion failed with error code: " << static_cast<int>(result) << std::endl;
        return 1;
    }
    std::cout << "Anthropic Response: " << claude_response.text << std::endl;

    return 0;
}
