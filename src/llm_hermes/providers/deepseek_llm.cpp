#include "llm_hermes/providers/deepseek_llm.h"

namespace llm_hermes
{

DeepseekLLM::DeepseekLLM()=default;

ErrorCode DeepseekLLM::completion(const CompletionRequest &request,
    CompletionResponse &response)
{
    std::string apiKey;
    if(request.api_key.has_value())
    {
        apiKey=request.api_key.value();
    }
    else
    {
        auto result=getApiKey("deepseek", apiKey);
        if(result!=ErrorCode::Success)
        {
            return result;
        }
    }

    // Create request body and headers
    auto body=createRequestBody(request);
    auto headers=createHeaders(apiKey);

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
    return parseResponse(raw_response, response);
}

nlohmann::json DeepseekLLM::createRequestBody(const CompletionRequest &request)
{
    nlohmann::json body;
    body["model"]="deepseek-chat";  // Currently only supporting main chat model

    // Convert messages to Deepseek format
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

    // Add default values for required fields
    body["frequency_penalty"]=0;
    body["presence_penalty"]=0;
    body["response_format"]={ {"type", "text"} };
    body["stream"]=false;
    body["top_p"]=1;

    return body;
}

cpr::Header DeepseekLLM::createHeaders(const std::string &apiKey)
{
    return cpr::Header{
        {"Content-Type", "application/json"},
        {"Accept", "application/json"},
        {"Authorization", "Bearer "+apiKey}
    };
}

ErrorCode DeepseekLLM::parseResponse(const cpr::Response &rawResponse,
    CompletionResponse &response)
{
    try {
        nlohmann::json jsonResponse = nlohmann::json::parse(rawResponse.text);
    } catch(const nlohmann::json::parse_error&)
    {
        return ErrorCode::InvalidResponse;
    }

    // Extract the response text from the first choice
    if(!jsonResponse.contains("choices")||
        jsonResponse["choices"].empty()||
        !jsonResponse["choices"][0].contains("message")||
        !jsonResponse["choices"][0]["message"].contains("content"))
    {
        return ErrorCode::InvalidResponse;
    }

    response.text=jsonResponse["choices"][0]["message"]["content"];
    response.provider="deepseek";

    if(jsonResponse.contains("model"))
    {
        response.model=jsonResponse["model"];
    }

    // Extract usage information if available
    if(jsonResponse.contains("usage")&&
        jsonResponse["usage"].contains("total_tokens"))
    {
        response.tokens_used=jsonResponse["usage"]["total_tokens"];
    }

    return ErrorCode::Success;
}

} // namespace llm_hermes
