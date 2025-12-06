# LLM Integration (ArbiterAI)

## Overview

ArbiterAI is a comprehensive C++ library that provides unified access to multiple Large Language Model (LLM) providers. It abstracts the differences between various AI services, allowing developers to switch between providers seamlessly while maintaining a consistent API.

## Architecture

### Component Structure

```
┌─────────────────────────────────────────────┐
│         Client Application                   │
│         (Cronus/LoreForge)                  │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│         ArbiterAI Unified API               │
│  - completion()                             │
│  - streamingCompletion()                    │
│  - embedding()                              │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│           ModelManager                      │
│  - Model discovery                          │
│  - Provider selection                       │
│  - Configuration management                 │
│  - Version compatibility                    │
└─────────────────────────────────────────────┘
                    ↓
        ┌───────────┴───────────┬────────────┐
        ↓                       ↓            ↓
┌──────────────┐   ┌──────────────┐   ┌──────────────┐
│   OpenAI     │   │  Anthropic   │   │    Google    │
│   Provider   │   │   Provider   │   │   Provider   │
└──────────────┘   └──────────────┘   └──────────────┘
        ↓                       ↓            ↓
┌──────────────┐   ┌──────────────┐   ┌──────────────┐
│ Local LLMs   │   │   Custom     │   │   Future     │
│ (llama.cpp)  │   │  Providers   │   │  Providers   │
└──────────────┘   └──────────────┘   └──────────────┘
```

## Model Manager

### Purpose

The ModelManager is the central component responsible for:
- Loading model configurations from multiple sources
- Managing provider-model mappings
- Handling version compatibility
- Ranking and selecting optimal models
- Dynamic model discovery

### Configuration System

**File Location:** `server/arbiterAI/src/arbiterAI/modelManager.h`

#### ModelInfo Structure

```cpp
struct ModelInfo {
    std::string model;                          // Model identifier (e.g., "gpt-4")
    std::string provider;                       // Provider name (e.g., "openai")
    std::string mode{"chat"};                   // Mode: chat, completion, embedding
    std::string configVersion{"1.1.0"};         // Schema version
    std::string minSchemaVersion{"1.0.0"};      // Minimum compatible schema
    int ranking{50};                            // Priority (0-100)
    std::optional<std::string> apiBase;         // Custom API endpoint
    std::optional<std::string> filePath;        // Local model path
    std::optional<std::string> apiKey;          // API key override
    std::optional<DownloadMetadata> download;   // Download information
    std::optional<std::string> minClientVersion; // Minimum client version
    std::optional<std::string> maxClientVersion; // Maximum client version
    bool examplesAsSysMsg{false};               // Few-shot example handling
    int contextWindow{4096};                    // Maximum context size
    int maxTokens{2048};                        // Default max tokens
    int maxInputTokens{3072};                   // Maximum input tokens
    int maxOutputTokens{1024};                  // Maximum output tokens
    Pricing pricing;                            // Cost information
};
```

#### Pricing Information

```cpp
struct Pricing {
    double prompt_token_cost = 0.0;      // Cost per 1M prompt tokens
    double completion_token_cost = 0.0;  // Cost per 1M completion tokens
};
```

### Initialization

**File:** `server/arbiterAI/src/arbiterAI/modelManager.cpp`

```cpp
bool ModelManager::initialize(
    const std::vector<std::filesystem::path>& configPaths,
    const std::filesystem::path& localOverridePath)
{
    // 1. Download configurations from remote repository
    const std::string remoteUrl = "https://github.com/caseymcc/arbiterAI_config.git";
    std::filesystem::path localPath = std::filesystem::temp_directory_path() / "arbiterAI_config";
    
    m_configDownloader.initialize(remoteUrl, localPath);
    
    // 2. Load models from remote config
    auto remoteModelsPath = m_configDownloader.getLocalPath() / "models";
    if (std::filesystem::exists(remoteModelsPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(remoteModelsPath)) {
            if (entry.path().extension() == ".json") {
                loadModelFile(entry.path());
            }
        }
    }
    
    // 3. Load models from local config paths
    for (const auto& configPath : configPaths) {
        auto modelsPath = configPath / "models";
        if (std::filesystem::exists(modelsPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(modelsPath)) {
                if (entry.path().extension() == ".json") {
                    loadModelFile(entry.path());
                }
            }
        }
    }
    
    // 4. Load local overrides (highest priority)
    if (!localOverridePath.empty() && std::filesystem::exists(localOverridePath)) {
        for (const auto& entry : std::filesystem::directory_iterator(localOverridePath)) {
            if (entry.path().extension() == ".json") {
                loadModelFile(entry.path());
            }
        }
    }
    
    return true;
}
```

### Configuration Sources (Priority Order)

1. **Local Overrides**: User-specific configurations (highest priority)
2. **Config Paths**: Application-provided configurations
3. **Remote Repository**: Centralized model definitions (lowest priority)

### Model Selection

**By Model Name:**

```cpp
std::optional<std::string> ModelManager::getProvider(const std::string& model) const {
    auto it = m_modelProviderMap.find(model);
    if (it != m_modelProviderMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<ModelInfo> ModelManager::getModelInfo(const std::string& model) const {
    for (const auto& info : m_models) {
        if (info.model == model) {
            return info;
        }
    }
    return std::nullopt;
}
```

**By Provider:**

```cpp
std::vector<ModelInfo> ModelManager::getModels(const std::string& provider) const {
    std::vector<ModelInfo> result;
    for (const auto& info : m_models) {
        if (info.provider == provider) {
            result.push_back(info);
        }
    }
    return result;
}
```

**By Ranking:**

```cpp
std::vector<ModelInfo> ModelManager::getModelsByRanking() const {
    std::vector<ModelInfo> sorted = m_models;
    std::sort(sorted.begin(), sorted.end(),
        [](const ModelInfo& a, const ModelInfo& b) {
            return a.ranking > b.ranking;  // Higher ranking first
        });
    return sorted;
}
```

### Version Compatibility

```cpp
bool ModelInfo::isCompatible(const std::string& clientVersion) const {
    if (minClientVersion.has_value() && 
        ModelManager::compareVersions(clientVersion, minClientVersion.value()) < 0) {
        return false;
    }
    if (maxClientVersion.has_value() && 
        ModelManager::compareVersions(clientVersion, maxClientVersion.value()) > 0) {
        return false;
    }
    return true;
}

bool ModelInfo::isSchemaCompatible(const std::string& schemaVersion) const {
    if (minSchemaVersion.empty() || schemaVersion.empty()) {
        return true;
    }
    return ModelManager::compareVersions(schemaVersion, minSchemaVersion) >= 0 &&
           ModelManager::compareVersions(schemaVersion, configVersion) <= 0;
}
```

**Version Comparison:**

```cpp
int ModelManager::compareVersions(const std::string& v1, const std::string& v2) {
    // Semantic versioning comparison: "1.2.3" vs "1.2.4"
    // Returns: -1 if v1 < v2, 0 if equal, 1 if v1 > v2
}
```

## Provider Implementations

### Supported Providers

1. **OpenAI**: GPT-3.5, GPT-4, GPT-4-turbo, embeddings
2. **Anthropic**: Claude 2, Claude 3 (Opus, Sonnet, Haiku)
3. **Google**: Gemini Pro, Gemini Ultra
4. **Local Models**: llama.cpp integration for local inference

### Provider Interface

Each provider implements a common interface:

```cpp
class LLMProvider {
public:
    virtual ~LLMProvider() = default;
    
    // Synchronous completion
    virtual std::string completion(
        const std::string& prompt,
        const CompletionParams& params
    ) = 0;
    
    // Streaming completion (token-by-token)
    virtual void streamingCompletion(
        const std::string& prompt,
        const CompletionParams& params,
        std::function<void(const std::string&)> callback
    ) = 0;
    
    // Generate embeddings
    virtual std::vector<float> embedding(
        const std::string& text,
        const EmbeddingParams& params
    ) = 0;
    
    // Token counting
    virtual int countTokens(const std::string& text) = 0;
};
```

### OpenAI Provider

**Features:**
- Chat completion API
- Streaming responses
- Function calling support
- Embeddings (text-embedding-ada-002, text-embedding-3-small, etc.)
- Vision capabilities (GPT-4V)

**Configuration Example:**

```json
{
    "model": "gpt-4-turbo",
    "provider": "openai",
    "mode": "chat",
    "configVersion": "1.1.0",
    "ranking": 90,
    "contextWindow": 128000,
    "maxTokens": 4096,
    "maxInputTokens": 124000,
    "maxOutputTokens": 4096,
    "pricing": {
        "prompt_token_cost": 10.0,
        "completion_token_cost": 30.0
    }
}
```

### Anthropic Provider

**Features:**
- Claude 2 and Claude 3 models
- Large context windows (up to 200K tokens)
- Streaming responses
- Multi-modal support (Claude 3)

**Configuration Example:**

```json
{
    "model": "claude-3-opus-20240229",
    "provider": "anthropic",
    "mode": "chat",
    "ranking": 95,
    "contextWindow": 200000,
    "maxTokens": 4096,
    "maxOutputTokens": 4096,
    "pricing": {
        "prompt_token_cost": 15.0,
        "completion_token_cost": 75.0
    }
}
```

### Google Provider

**Features:**
- Gemini Pro and Ultra models
- Long context support
- Multi-modal capabilities
- Grounding with Google Search

**Configuration Example:**

```json
{
    "model": "gemini-1.5-pro",
    "provider": "google",
    "mode": "chat",
    "ranking": 85,
    "contextWindow": 1000000,
    "maxTokens": 8192,
    "pricing": {
        "prompt_token_cost": 3.5,
        "completion_token_cost": 10.5
    }
}
```

### Local Models (llama.cpp)

**Features:**
- Run models locally without API calls
- Support for GGUF format models
- GPU acceleration
- Lower cost (no API fees)

**Configuration Example:**

```json
{
    "model": "llama-3-8b",
    "provider": "local",
    "mode": "chat",
    "filePath": "/models/llama-3-8b.gguf",
    "ranking": 60,
    "contextWindow": 8192,
    "maxTokens": 2048,
    "pricing": {
        "prompt_token_cost": 0.0,
        "completion_token_cost": 0.0
    }
}
```

## API Usage

### Completion Requests

**Synchronous:**

```cpp
#include <arbiterAI/modelManager.h>

// Initialize model manager
arbiterAI::ModelManager::instance().initialize();

// Get model info
auto modelInfo = arbiterAI::ModelManager::instance().getModelInfo("gpt-4");
if (modelInfo) {
    std::cout << "Using model: " << modelInfo->model << std::endl;
    std::cout << "Provider: " << modelInfo->provider << std::endl;
    std::cout << "Context window: " << modelInfo->contextWindow << std::endl;
}

// Get provider for model
auto provider = arbiterAI::ModelManager::instance().getProvider("gpt-4");
if (provider) {
    std::cout << "Provider: " << provider.value() << std::endl;
}
```

**Streaming:**

```cpp
// Streaming completion with callback
void streamCompletion(const std::string& prompt) {
    auto modelInfo = arbiterAI::ModelManager::instance().getModelInfo("gpt-4");
    
    // Callback receives tokens as they arrive
    auto callback = [](const std::string& token) {
        std::cout << token << std::flush;
    };
    
    // Stream tokens in real-time
    provider->streamingCompletion(prompt, params, callback);
}
```

### Model Discovery

**List All Models:**

```cpp
// Get all available models
auto allModels = arbiterAI::ModelManager::instance().getModelsByRanking();
for (const auto& model : allModels) {
    std::cout << "Model: " << model.model 
              << " (Provider: " << model.provider
              << ", Ranking: " << model.ranking << ")" << std::endl;
}
```

**Filter by Provider:**

```cpp
// Get all OpenAI models
auto openaiModels = arbiterAI::ModelManager::instance().getModels("openai");
for (const auto& model : openaiModels) {
    std::cout << "OpenAI Model: " << model.model << std::endl;
}
```

**Select Best Model:**

```cpp
// Get highest-ranked model
auto models = arbiterAI::ModelManager::instance().getModelsByRanking();
if (!models.empty()) {
    const auto& bestModel = models[0];
    std::cout << "Best model: " << bestModel.model 
              << " (Ranking: " << bestModel.ranking << ")" << std::endl;
}
```

## Configuration Management

### Configuration Downloader

**Purpose:** Automatically fetch and update model configurations from a Git repository.

**Features:**
- Git repository cloning
- Automatic updates
- Version tracking
- Local caching

**Implementation:**

```cpp
class ConfigDownloader {
public:
    void initialize(const std::string& remoteUrl, 
                   const std::filesystem::path& localPath);
    std::filesystem::path getLocalPath() const;
    bool update();  // Pull latest changes
private:
    std::string m_remoteUrl;
    std::filesystem::path m_localPath;
};
```

### Configuration File Format

**JSON Schema:**

```json
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "required": ["model", "provider", "mode"],
    "properties": {
        "model": {"type": "string"},
        "provider": {"type": "string"},
        "mode": {"type": "string", "enum": ["chat", "completion", "embedding"]},
        "configVersion": {"type": "string"},
        "minSchemaVersion": {"type": "string"},
        "ranking": {"type": "integer", "minimum": 0, "maximum": 100},
        "apiBase": {"type": "string"},
        "filePath": {"type": "string"},
        "apiKey": {"type": "string"},
        "contextWindow": {"type": "integer"},
        "maxTokens": {"type": "integer"},
        "maxInputTokens": {"type": "integer"},
        "maxOutputTokens": {"type": "integer"},
        "pricing": {
            "type": "object",
            "properties": {
                "prompt_token_cost": {"type": "number"},
                "completion_token_cost": {"type": "number"}
            }
        }
    }
}
```

### Custom Model Configuration

**Creating a custom model:**

```json
{
    "model": "custom-gpt",
    "provider": "openai",
    "mode": "chat",
    "apiBase": "https://custom-endpoint.com/v1",
    "apiKey": "custom-api-key",
    "ranking": 80,
    "contextWindow": 16384,
    "maxTokens": 4096,
    "pricing": {
        "prompt_token_cost": 5.0,
        "completion_token_cost": 15.0
    }
}
```

**Loading custom configuration:**

```cpp
// Specify local override directory
std::filesystem::path overridePath = "/path/to/custom/models";
arbiterAI::ModelManager::instance().initialize({}, overridePath);
```

## Integration with Cronus

### Usage in Cronus Main Application

**File:** `server/cronus/main.cpp`

```cpp
#include "cronus/cronus.h"
#include "cronus/config.h"

int main(int argc, char* argv[]) {
    // Load configuration (includes ArbiterAI initialization)
    cronus::Config::instance().load(resourcePath);
    
    // Create Cronus application
    cronus::Cronus cronusApp;
    
    // Run application (uses ArbiterAI for LLM calls)
    cronusApp.run(resourcePath);
    
    return 0;
}
```

### Usage in LoreForge

LoreForge uses ArbiterAI for:
1. **Embedding Generation**: Convert code chunks to vectors
2. **Query Understanding**: Process natural language queries
3. **Context Augmentation**: Generate responses with retrieved code context

**Planned Integration:**

```cpp
// Generate embedding for code chunk
std::vector<float> embedCode(const std::string& code) {
    auto modelInfo = arbiterAI::ModelManager::instance()
        .getModelInfo("text-embedding-3-small");
    
    if (modelInfo && modelInfo->mode == "embedding") {
        // Generate embedding using configured model
        return provider->embedding(code, params);
    }
    return {};
}

// Process query with LLM
std::string queryCodebase(const std::string& query, 
                          const std::vector<std::string>& context) {
    // Get best chat model
    auto models = arbiterAI::ModelManager::instance().getModelsByRanking();
    auto chatModel = std::find_if(models.begin(), models.end(),
        [](const auto& m) { return m.mode == "chat"; });
    
    if (chatModel != models.end()) {
        // Construct prompt with context
        std::string prompt = buildPrompt(query, context);
        
        // Get completion
        return provider->completion(prompt, params);
    }
    return "";
}
```

## Error Handling

### Provider Errors

```cpp
try {
    auto result = provider->completion(prompt, params);
} catch (const arbiterAI::ProviderException& e) {
    std::cerr << "Provider error: " << e.what() << std::endl;
    // Fallback to alternative provider
}
```

### Configuration Errors

```cpp
if (!arbiterAI::ModelManager::instance().initialize()) {
    std::cerr << "Failed to initialize ModelManager" << std::endl;
    // Use default configurations
}
```

### Version Incompatibility

```cpp
auto modelInfo = arbiterAI::ModelManager::instance().getModelInfo("gpt-4");
if (modelInfo && !modelInfo->isCompatible(CLIENT_VERSION)) {
    std::cerr << "Model incompatible with client version" << std::endl;
    // Select alternative model
}
```

## Performance Considerations

### Caching

**Model Configuration Caching:**
- Configurations cached in memory after loading
- Singleton pattern prevents reloading
- Git repository cached locally

**Response Caching (Planned):**
```cpp
class ResponseCache {
    std::unordered_map<std::string, std::string> cache;
    
public:
    std::optional<std::string> get(const std::string& promptHash);
    void set(const std::string& promptHash, const std::string& response);
};
```

### Token Management

**Token Counting:**
```cpp
int estimateTokens(const std::string& text) {
    // Rough estimation: ~4 characters per token for English
    return text.length() / 4;
}

bool fitsInContext(const std::string& prompt, const ModelInfo& model) {
    int tokens = estimateTokens(prompt);
    return tokens <= model.maxInputTokens;
}
```

### Cost Optimization

**Calculate Request Cost:**

```cpp
double calculateCost(const ModelInfo& model, int promptTokens, int completionTokens) {
    double promptCost = (promptTokens / 1000000.0) * model.pricing.prompt_token_cost;
    double completionCost = (completionTokens / 1000000.0) * model.pricing.completion_token_cost;
    return promptCost + completionCost;
}
```

**Select Cost-Effective Model:**

```cpp
ModelInfo selectByCost(const std::vector<ModelInfo>& models, int estimatedTokens) {
    return *std::min_element(models.begin(), models.end(),
        [estimatedTokens](const ModelInfo& a, const ModelInfo& b) {
            double costA = calculateCost(a, estimatedTokens, estimatedTokens);
            double costB = calculateCost(b, estimatedTokens, estimatedTokens);
            return costA < costB;
        });
}
```

## Future Enhancements

### Planned Features

1. **Automatic Fallback**: Try alternative providers on failure
2. **Load Balancing**: Distribute requests across multiple API keys
3. **Rate Limiting**: Respect provider rate limits
4. **Usage Tracking**: Monitor costs and token usage
5. **Model Benchmarking**: Compare model quality and speed
6. **Custom Providers**: Plugin system for new providers
7. **Async API**: Non-blocking API calls
8. **Batch Processing**: Batch multiple requests

### Advanced Configuration

**Retry Logic:**
```cpp
struct RetryConfig {
    int maxRetries = 3;
    int backoffMs = 1000;
    bool exponentialBackoff = true;
};
```

**Provider Preferences:**
```cpp
struct ProviderPreferences {
    std::vector<std::string> preferredProviders;
    bool allowFallback = true;
    double maxCostPerRequest = 0.10;
};
```

## Testing

### Unit Tests

```cpp
TEST(ModelManager, Initialize) {
    arbiterAI::ModelManager::instance().reset();
    ASSERT_TRUE(arbiterAI::ModelManager::instance().initialize());
}

TEST(ModelManager, GetProvider) {
    auto provider = arbiterAI::ModelManager::instance().getProvider("gpt-4");
    ASSERT_TRUE(provider.has_value());
    EXPECT_EQ(provider.value(), "openai");
}

TEST(ModelManager, GetModelsByRanking) {
    auto models = arbiterAI::ModelManager::instance().getModelsByRanking();
    ASSERT_FALSE(models.empty());
    
    // Verify sorted by ranking
    for (size_t i = 1; i < models.size(); ++i) {
        EXPECT_GE(models[i-1].ranking, models[i].ranking);
    }
}
```

### Integration Tests

```cpp
TEST(Integration, CompletionRequest) {
    auto modelInfo = arbiterAI::ModelManager::instance().getModelInfo("gpt-3.5-turbo");
    ASSERT_TRUE(modelInfo.has_value());
    
    // Test completion (requires API key)
    // auto result = provider->completion("Hello, world!", params);
    // EXPECT_FALSE(result.empty());
}
```

This LLM integration system provides a robust foundation for incorporating AI capabilities into Cronus and LoreForge while maintaining flexibility, cost-awareness, and ease of use.
