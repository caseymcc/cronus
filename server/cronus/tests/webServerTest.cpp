#include "cronus/webServer.h"
#include "cronus/cronus.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

using json = nlohmann::json;
using namespace cronus;

class WebServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a Cronus instance
        cronus = std::make_unique<Cronus>();
        
        // Create WebServer on a specific port for testing
        webServer = std::make_unique<WebServer>(*cronus, 19000);
        
        // Give the server a moment to initialize
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void TearDown() override {
        if (webServer && webServer->isRunning()) {
            webServer->stop();
        }
        cronus.reset();
        webServer.reset();
    }

    std::unique_ptr<Cronus> cronus;
    std::unique_ptr<WebServer> webServer;
};

// Test: WebServer starts and stops correctly
TEST_F(WebServerTest, StartAndStop) {
    EXPECT_FALSE(webServer->isRunning());
    
    EXPECT_TRUE(webServer->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(webServer->isRunning());
    
    webServer->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(webServer->isRunning());
}

// Test: Multiple start calls don't crash
TEST_F(WebServerTest, MultipleStartCalls) {
    EXPECT_TRUE(webServer->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Second start should return true but not crash
    EXPECT_TRUE(webServer->start());
    EXPECT_TRUE(webServer->isRunning());
    
    webServer->stop();
}

// Test: Stop before start doesn't crash
TEST_F(WebServerTest, StopBeforeStart) {
    EXPECT_FALSE(webServer->isRunning());
    webServer->stop();
    EXPECT_FALSE(webServer->isRunning());
}

// Test: Multiple stop calls don't crash
TEST_F(WebServerTest, MultipleStopCalls) {
    webServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    webServer->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Second stop should not crash
    webServer->stop();
    EXPECT_FALSE(webServer->isRunning());
}

// Test: Base URL is correct format
TEST_F(WebServerTest, BaseUrl) {
    std::string baseUrl = webServer->getBaseUrl();
    EXPECT_TRUE(baseUrl.find("http://localhost:") != std::string::npos);
    EXPECT_TRUE(baseUrl.find("19000") != std::string::npos);
}

// Test: Broadcast methods don't crash when no clients connected
TEST_F(WebServerTest, BroadcastWithNoClients) {
    webServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // These should not crash even with no WebSocket clients
    EXPECT_NO_THROW(webServer->broadcastMessage("user", "Test message"));
    EXPECT_NO_THROW(webServer->broadcastLog("info", "Test log message"));
    EXPECT_NO_THROW(webServer->broadcastDirectoryUpdate());
    
    json testParams = {{"key", "value"}};
    EXPECT_NO_THROW(webServer->broadcastNotification("testMethod", testParams));
}

// Test: Broadcast message with different roles
TEST_F(WebServerTest, BroadcastMessageRoles) {
    webServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_NO_THROW(webServer->broadcastMessage("user", "User message"));
    EXPECT_NO_THROW(webServer->broadcastMessage("agent", "Agent response"));
    EXPECT_NO_THROW(webServer->broadcastMessage("system", "System notification"));
}

// Test: Broadcast log with different levels
TEST_F(WebServerTest, BroadcastLogLevels) {
    webServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_NO_THROW(webServer->broadcastLog("debug", "Debug message"));
    EXPECT_NO_THROW(webServer->broadcastLog("info", "Info message"));
    EXPECT_NO_THROW(webServer->broadcastLog("warning", "Warning message"));
    EXPECT_NO_THROW(webServer->broadcastLog("error", "Error message"));
}

// Test: Server handles rapid start/stop cycles
TEST_F(WebServerTest, RapidStartStop) {
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(webServer->start());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(webServer->isRunning());
        
        webServer->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_FALSE(webServer->isRunning());
    }
}

// Test: Server can be created on different ports
TEST(WebServerPortTest, DifferentPorts) {
    Cronus cronus1;
    Cronus cronus2;
    
    auto server1 = std::make_unique<WebServer>(cronus1, 19001);
    auto server2 = std::make_unique<WebServer>(cronus2, 19002);
    
    EXPECT_TRUE(server1->start());
    EXPECT_TRUE(server2->start());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_TRUE(server1->isRunning());
    EXPECT_TRUE(server2->isRunning());
    
    std::string url1 = server1->getBaseUrl();
    std::string url2 = server2->getBaseUrl();
    
    EXPECT_NE(url1, url2);
    EXPECT_TRUE(url1.find("19001") != std::string::npos);
    EXPECT_TRUE(url2.find("19002") != std::string::npos);
    
    server1->stop();
    server2->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// Test: JSON-RPC request structure validation
TEST(JsonRpcTest, ValidRequestStructure) {
    // Valid JSON-RPC 2.0 request
    json validRequest = {
        {"jsonrpc", "2.0"},
        {"method", "getSourceMap"},
        {"params", json::object()},
        {"id", 1}
    };
    
    EXPECT_EQ(validRequest["jsonrpc"], "2.0");
    EXPECT_EQ(validRequest["method"], "getSourceMap");
    EXPECT_TRUE(validRequest.contains("id"));
    EXPECT_TRUE(validRequest.contains("params"));
}

// Test: JSON-RPC error response structure
TEST(JsonRpcTest, ErrorResponseStructure) {
    // Standard JSON-RPC 2.0 error response
    json errorResponse = {
        {"jsonrpc", "2.0"},
        {"error", {
            {"code", -32600},
            {"message", "Invalid Request"}
        }},
        {"id", nullptr}
    };
    
    EXPECT_EQ(errorResponse["jsonrpc"], "2.0");
    EXPECT_TRUE(errorResponse.contains("error"));
    EXPECT_EQ(errorResponse["error"]["code"], -32600);
    EXPECT_TRUE(errorResponse["error"].contains("message"));
}

// Test: JSON-RPC notification structure (no id)
TEST(JsonRpcTest, NotificationStructure) {
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "directoryUpdate"},
        {"params", {
            {"tree", json::array()}
        }}
    };
    
    EXPECT_EQ(notification["jsonrpc"], "2.0");
    EXPECT_TRUE(notification.contains("method"));
    EXPECT_TRUE(notification.contains("params"));
    EXPECT_FALSE(notification.contains("id")); // Notifications don't have id
}

// Test: Concurrent broadcast calls
TEST_F(WebServerTest, ConcurrentBroadcasts) {
    webServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::vector<std::thread> threads;
    
    // Launch multiple threads broadcasting simultaneously
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, i]() {
            webServer->broadcastMessage("user", "Message " + std::to_string(i));
            webServer->broadcastLog("info", "Log " + std::to_string(i));
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Server should still be running
    EXPECT_TRUE(webServer->isRunning());
}

