#include "llm_hermes/hermes.h"

#include <iostream>

int main()
{
    std::cout<<"Cronus Client"<<std::endl;

    try
    {
        // Using OpenAI
        auto response=llm_hermes::completion(
            {
                .model="gpt-3.5-turbo",
                .messages={
                    {"user", "Hello, how are you?"}
                }
            });
        std::cout<<"OpenAI Response: "<<response.text<<std::endl;

        // Using Anthropic
        auto claude_response=llm_hermes::completion(
            {
                .model="claude-2",
                .messages=
                {
                    {"user", "Hello, how are you?"}
                }
            });
        std::cout<<"Anthropic Response: "<<claude_response.text<<std::endl;

    }
    catch (const std::exception& e)
    {
        std::cerr<<"Error: "<<e.what()<<std::endl;
    }

    return 0;
}
