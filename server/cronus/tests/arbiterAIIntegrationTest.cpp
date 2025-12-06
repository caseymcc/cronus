// Integration test for ArbiterAI using an OpenAI-compatible local LLM endpoint.
// The test expects a server exposing the OpenAI /v1/chat/completions endpoint
// at the IP provided (default: http://192.168.2.106). You can override with
// OPENAI_BASE_URL environment variable.

#include <gtest/gtest.h>
#include "arbiterAI/arbiterAI.h"
#include "arbiterAI/modelManager.h"
#include <fstream>
#include <cstdlib>
#include <filesystem>

namespace
{

std::filesystem::path createTempModelConfigDir()
{
    // Create a temporary config directory with required models subdirectory
    auto baseDir=std::filesystem::temp_directory_path() / "arbiterai_gpt120_test_cfg";
    std::filesystem::create_directories(baseDir / "models");
    std::string jsonConfig=R"({
        "schema_version": "1.0.0",
        "models": [
            { "model": "gpt-120", "provider": "openai", "ranking": 10 }
        ]
    })";
    auto modelFile=baseDir / "models" / "gpt120.json";
    std::ofstream ofs(modelFile);
    ofs << jsonConfig;
    ofs.close();
    return baseDir;
}

}

class ArbiterAIIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Point to local OpenAI-compatible server if not already set
        if(!std::getenv("OPENAI_BASE_URL"))
        {
            setenv("OPENAI_BASE_URL", "http://192.168.2.106:8000/v1", 1);
        }
        // For local servers that don't require auth, leaving OPENAI_API_KEY unset is fine.
        // If the server requires a key, export OPENAI_API_KEY before running.

        // Provide dummy API key to satisfy provider requirements
        if(!std::getenv("OPENAI_API_KEY"))
        {
            setenv("OPENAI_API_KEY", "dummy", 1);
        }
        auto cfgDir=createTempModelConfigDir();
        auto ec=arbiterAI::ArbiterAI::instance().initialize({ cfgDir });
        ASSERT_EQ(ec, arbiterAI::ErrorCode::Success) << "Failed to initialize ArbiterAI";
        ASSERT_TRUE(arbiterAI::ArbiterAI::instance().initialized) << "ArbiterAI should be marked initialized";
    }
};

TEST_F(ArbiterAIIntegrationTest, CompletionBasic)
{
    arbiterAI::CompletionRequest req;
    
    req.model="gpt-120";
    req.messages={
        {"system", "You are a concise assistant used for integration testing."},
        {"user", "Reply with the single word: pong"}
    };
    req.max_tokens=32;

    arbiterAI::CompletionResponse resp;
    auto ec=arbiterAI::ArbiterAI::instance().completion(req, resp);

    if(ec == arbiterAI::ErrorCode::NetworkError)
    {
        GTEST_SKIP() << "Network unreachable for local OpenAI-compatible endpoint";
    }

    ASSERT_EQ(ec, arbiterAI::ErrorCode::Success) << "Completion call failed (ErrorCode=" << static_cast<int>(ec) << ")";
    ASSERT_FALSE(resp.text.empty()) << "Response text should not be empty";
    // Basic sanity: ensure expected word present (may vary slightly depending on local server behavior)
    // We don't enforce exact equality to reduce flakiness.
    EXPECT_NE(resp.text.find("pong"), std::string::npos) << "Model response does not contain expected keyword";
}

TEST_F(ArbiterAIIntegrationTest, UnknownModelFails)
{
    arbiterAI::CompletionRequest req;
    req.model="nonexistent-model"; // Not in config
    req.messages={ {"user", "Hello"} };
    arbiterAI::CompletionResponse resp;
    auto ec=arbiterAI::ArbiterAI::instance().completion(req, resp);
    EXPECT_EQ(ec, arbiterAI::ErrorCode::UnknownModel);
}


