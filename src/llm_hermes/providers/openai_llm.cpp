#include "llm_hermes/providers/openai_llm.h"

namespace llm_hermes
{

OpenAILLM::OpenAILLM()=default;

ErrorCode OpenAILLM::completion(const CompletionRequest &request,
    CompletionResponse &response)
{
    std::string api_key;
    if(request.api_key.has_value())
    {
        api_key=request.api_key.value();
    }
    else
    {
        auto result=get_api_key("openai", api_key);
        if(result!=ErrorCode::Success)
        {
            return result;
        }
    }

    // Create request body and headers
    auto body=create_request_body(request);
    auto headers=create_headers(api_key);

    // Make the API request
    auto raw_response=cpr::Post(
        cpr::Url{ API_URL },
        headers,
        cpr::Body{ body.dump() },
        cpr::VerifySsl{ true }
    );

    // Check for HTTP errors
    if(raw_response.status_code!=200)
    {
        return ErrorCode::NetworkError;
    }

    // Parse the response
    return parse_response(raw_response, response);
}

nlohmann::json OpenAILLM::create_request_body(const CompletionRequest &request)
{
    nlohmann::json body;
    body["model"]=request.model;

    // Convert messages to OpenAI format
    nlohmann::json messages=nlohmann::json::array();
    for(const auto &msg:request.messages)
    {
        messages.push_back({
            {"role", msg.role},
            {"content", msg.content}
            });
    }
    body["messages"]=messages;

    // Add optional parameters if present
    if(request.temperature.has_value())
    {
        body["temperature"]=request.temperature.value();
    }
    if(request.max_tokens.has_value())
    {
        body["max_tokens"]=request.max_tokens.value();
    }

    return body;
}

cpr::Header OpenAILLM::create_headers(const std::string &api_key)
{
    return cpr::Header{
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer "+api_key}
    };
}

ErrorCode OpenAILLM::parse_response(const cpr::Response &raw_response,
    CompletionResponse &response)
{
    nlohmann::json::error_code ec;
    nlohmann::json json_response=nlohmann::json::parse(raw_response.text, nullptr, false, ec);

    if(ec)
    {
        return ErrorCode::InvalidResponse;
    }

    // Extract the response text from the first choice
    if(!json_response.contains("choices")||
        json_response["choices"].empty()||
        !json_response["choices"][0].contains("message")||
        !json_response["choices"][0]["message"].contains("content"))
    {
        return ErrorCode::InvalidResponse;
    }

    response.text=json_response["choices"][0]["message"]["content"];
    response.provider="openai";

    if(json_response.contains("model"))
    {
        response.model=json_response["model"];
    }

    // Extract usage information if available
    if(json_response.contains("usage")&&
        json_response["usage"].contains("total_tokens"))
    {
        response.tokens_used=json_response["usage"]["total_tokens"];
    }

    return ErrorCode::Success;
}

} // namespace llm_hermes
