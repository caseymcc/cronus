#include <iostream>
#include "cronus/llm.hpp"

int main() {
    std::cout << "Cronus LLM Client" << std::endl;

    // Example usage
    cronus::CompletionRequest request{
        .model = "gpt-3.5-turbo",
        .messages = {
            {"system", "You are a helpful assistant."},
            {"user", "Hello, how are you?"}
        }
    };

    // Create OpenAI client (API key should be from environment variable in production)
    auto client = cronus::OpenAIClient("your-api-key");
    
    try {
        auto response = client.complete(request);
        std::cout << "Response: " << response.text << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
