# gRPC and REST API Documentation

## Overview

Cronus uses a dual API approach:
1. **gRPC API (LoreForge)**: High-performance, schema-driven service for code intelligence
2. **REST API (Cronus)**: HTTP-based API for web client communication

## LoreForge gRPC Server

### Service Definition

**File:** `server/loreforge/protos/loreforge.proto`

```protobuf
syntax = "proto3";
package loreforge;

service LoreForge {
    // Health monitoring
    rpc HealthCheck(HealthCheckRequest) returns (HealthCheckResponse) {}
    
    // Parse source code files
    rpc ParseFile(ParseFileRequest) returns (ParseFileResponse) {}
    
    // Query codebase with natural language
    rpc QueryCodebase(QueryRequest) returns (stream QueryResponse);
    
    // Index a repository for RAG
    rpc IndexRepository(IndexRepositoryRequest) returns (IndexRepositoryResponse);
}
```

### Message Definitions

#### HealthCheck

**Purpose:** Verify server availability and status

```protobuf
message HealthCheckRequest {}

message HealthCheckResponse {
    string status = 1;  // "ok" or error message
}
```

**Usage:**
```cpp
HealthCheckRequest request;
HealthCheckResponse response;
ClientContext context;

Status status = stub->HealthCheck(&context, &request, &response);
if (status.ok()) {
    std::cout << "Server status: " << response.status() << std::endl;
}
```

#### ParseFile

**Purpose:** Parse source code and validate syntax

```protobuf
message ParseFileRequest {
    string file_path = 1;  // File path for language detection
    string content = 2;    // Source code content
}

message ParseFileResponse {
    bool success = 1;      // Parse success/failure
}
```

**Current Implementation:**
- Detects language by file extension
- Uses tree-sitter for parsing
- Validates syntax
- Returns success status

**Planned Enhancements:**
```protobuf
message ParseFileResponse {
    bool success = 1;
    repeated SyntaxError errors = 2;
    repeated FunctionInfo functions = 3;
    repeated ClassInfo classes = 4;
    CallGraph call_graph = 5;
}
```

#### QueryCodebase

**Purpose:** Natural language queries against indexed codebase

```protobuf
message QueryRequest {
    string query = 1;            // Natural language query
    string model = 2;            // LLM model to use
    string codebase_name = 3;    // Target codebase identifier
}

message QueryResponse {
    string response_chunk = 1;   // Streaming response tokens
}
```

**Features:**
- **Streaming Response**: Tokens delivered in real-time
- **RAG-Enhanced**: Retrieves relevant code context
- **Multi-Model**: Supports different LLM backends

**Query Flow:**
```
1. Client sends QueryRequest
2. Server embeds query → Vector search
3. Retrieves relevant code chunks
4. Constructs prompt with context
5. Streams LLM response back to client
```

**Example Usage:**
```cpp
QueryRequest request;
request.set_query("How does the authentication system work?");
request.set_model("gpt-4");
request.set_codebase_name("my-project");

ClientContext context;
auto reader = stub->QueryCodebase(&context, &request);

QueryResponse response;
while (reader->Read(&response)) {
    std::cout << response.response_chunk() << std::flush;
}

Status status = reader->Finish();
```

#### IndexRepository

**Purpose:** Index a repository for semantic search

```protobuf
message IndexRepositoryRequest {
    string repository_path = 1;  // Path to repository
    string model = 2;            // Embedding model
    string codebase_name = 3;    // Unique codebase identifier
}

message IndexRepositoryResponse {
    bool success = 1;            // Indexing success
    string message = 2;          // Status message or error
}
```

**Indexing Process:**
```
1. Scan repository for source files
2. Parse each file with tree-sitter
3. Extract functions, classes, etc.
4. Generate embeddings for code chunks
5. Store in Faiss vector database
6. Save metadata mapping
```

**Example:**
```cpp
IndexRepositoryRequest request;
request.set_repository_path("/path/to/repo");
request.set_model("text-embedding-3-small");
request.set_codebase_name("my-project");

IndexRepositoryResponse response;
ClientContext context;

Status status = stub->IndexRepository(&context, &request, &response);
if (status.ok() && response.success()) {
    std::cout << "Repository indexed: " << response.message() << std::endl;
}
```

### Server Implementation

**File:** `server/loreforge/src/loreforge/main.cpp`

```cpp
class LoreForgeServiceImpl final : public LoreForge::Service {
public:
    Status HealthCheck(ServerContext* context,
                      const HealthCheckRequest* request,
                      HealthCheckResponse* response) override {
        response->set_status("ok");
        return Status::OK;
    }
    
    Status ParseFile(ServerContext* context,
                    const ParseFileRequest* request,
                    ParseFileResponse* response) override {
        // Language detection
        TSLanguage* language = detectLanguage(request->file_path());
        
        // Parse with tree-sitter
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, language);
        
        TSTree* tree = ts_parser_parse_string(
            parser, NULL,
            request->content().c_str(),
            request->content().length()
        );
        
        // Validate
        TSNode root = ts_tree_root_node(tree);
        bool success = validateAST(root);
        
        response->set_success(success);
        
        // Cleanup
        ts_tree_delete(tree);
        ts_parser_delete(parser);
        
        return Status::OK;
    }
    
    Status QueryCodebase(ServerContext* context,
                        const QueryRequest* request,
                        ServerWriter<QueryResponse>* writer) override {
        // Embed query
        auto queryEmbedding = embedQuery(request->query(), request->model());
        
        // Search vector database
        auto results = vectorSearch(queryEmbedding, request->codebase_name());
        
        // Construct context
        std::string context = buildContext(results);
        
        // Stream LLM response
        streamLLMResponse(request->query(), context, request->model(),
            [writer](const std::string& chunk) {
                QueryResponse response;
                response.set_response_chunk(chunk);
                writer->Write(response);
            });
        
        return Status::OK;
    }
};
```

### Server Configuration

**Launching Server:**

```cpp
int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50051");
    LoreForgeServiceImpl service;
    
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    
    server->Wait();
    return 0;
}
```

### Client Implementation

**Test Client:** `server/loreforge/tests/main.cpp`

```cpp
class LoreForgeTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::shared_ptr<Channel> channel = 
            grpc::CreateChannel("localhost:50051", 
                               grpc::InsecureChannelCredentials());
        stub = LoreForge::NewStub(channel);
    }
    
    std::unique_ptr<LoreForge::Stub> stub;
};

TEST_F(LoreForgeTest, HealthCheck) {
    HealthCheckRequest request;
    HealthCheckResponse response;
    ClientContext context;
    
    Status status = stub->HealthCheck(&context, &request, &response);
    
    ASSERT_TRUE(status.ok());
    EXPECT_EQ(response.status(), "ok");
}
```

## Cronus REST API

### Architecture

**File:** `server/cronus/restApi.cpp`

The REST API provides HTTP endpoints for:
- Web UI communication
- Real-time updates via Server-Sent Events (SSE)
- File operations
- Source code navigation

### Endpoints

#### POST /api/input

**Purpose:** Process user input and commands

**Request:**
```json
{
    "input": "add authentication to the login page",
    "context": {
        "currentFile": "src/login.js",
        "selectedText": "function login() { ... }"
    }
}
```

**Response:**
```json
{
    "status": "success",
    "response": "I'll add authentication to the login function...",
    "changes": [
        {
            "file": "src/login.js",
            "action": "modify",
            "diff": "..."
        }
    ]
}
```

**Implementation:**
```cpp
void RestApi::handleProcessInput(const httplib::Request& req, 
                                httplib::Response& res) {
    try {
        auto body = json::parse(req.body);
        std::string input = body["input"];
        
        // Queue task for processing
        m_cronus.queueTask([this, input]() {
            auto response = m_cronus.processInput(input);
            
            // Send response via SSE to all connected clients
            broadcastSSE("response", response);
        });
        
        json response = {
            {"status", "queued"},
            {"message", "Input is being processed"}
        };
        res.set_content(response.dump(), "application/json");
        
    } catch (const std::exception& e) {
        json error = {
            {"status", "error"},
            {"message", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}
```

#### GET /api/directory

**Purpose:** Get directory structure for file navigation

**Query Parameters:**
- `path`: Directory path (optional, defaults to project root)
- `depth`: Max depth to traverse (default: 3)

**Response:**
```json
{
    "path": "/project/src",
    "contents": [
        {
            "name": "components",
            "type": "directory",
            "children": [...]
        },
        {
            "name": "App.js",
            "type": "file",
            "size": 2048,
            "language": "javascript"
        }
    ]
}
```

**Implementation:**
```cpp
void RestApi::handleGetDirectoryContents(const httplib::Request& req,
                                        httplib::Response& res) {
    std::string path = req.get_param_value("path");
    if (path.empty()) {
        path = m_cronus.getWorkingDirectory();
    }
    
    auto contents = m_cronus.getDirectoryStructure(path);
    
    json response = {
        {"path", path},
        {"contents", contents}
    };
    
    res.set_content(response.dump(), "application/json");
}
```

#### GET /api/sourcemap

**Purpose:** Get source map data for code navigation

**Response:**
```json
{
    "files": [
        {
            "path": "src/App.js",
            "functions": ["App", "handleClick"],
            "classes": ["Component"],
            "imports": ["react", "./utils"]
        }
    ],
    "symbols": {
        "App": {
            "type": "function",
            "file": "src/App.js",
            "line": 10
        }
    }
}
```

#### GET /api/file

**Purpose:** Get file content

**Query Parameters:**
- `path`: File path (required)

**Response:**
```json
{
    "path": "src/App.js",
    "content": "import React from 'react';\n...",
    "language": "javascript",
    "size": 2048
}
```

#### GET /api/health

**Purpose:** Health check for REST API

**Response:**
```json
{
    "status": "ok",
    "message": "Cronus API is running"
}
```

### Server-Sent Events (SSE)

#### GET /api/events

**Purpose:** Real-time updates from server to client

**Event Types:**
- `response`: LLM responses
- `progress`: Long-running operation progress
- `file_change`: File system changes
- `error`: Error notifications

**Client Example (JavaScript):**
```javascript
const eventSource = new EventSource('http://localhost:8080/api/events');

eventSource.addEventListener('response', (event) => {
    const data = JSON.parse(event.data);
    console.log('Response:', data.message);
});

eventSource.addEventListener('progress', (event) => {
    const data = JSON.parse(event.data);
    updateProgressBar(data.percentage);
});

eventSource.onerror = (error) => {
    console.error('SSE error:', error);
    eventSource.close();
};
```

**Server Implementation:**
```cpp
void RestApi::handleSSEConnection(const httplib::Request& req,
                                 httplib::Response& res) {
    // Set SSE headers
    res.set_header("Content-Type", "text/event-stream");
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Connection", "keep-alive");
    
    // Create client
    auto client = std::make_shared<SSEClient>();
    client->id = generateClientId();
    client->connected = true;
    
    {
        std::lock_guard<std::mutex> lock(m_sseClientsMutex);
        m_sseClients.push_back(client);
    }
    
    // Keep connection alive
    res.set_content_provider(
        "text/event-stream",
        [client](size_t offset, httplib::DataSink& sink) {
            if (!client->connected) {
                return false;  // Close connection
            }
            
            // Check for messages
            std::string message;
            {
                std::lock_guard<std::mutex> lock(client->mutex);
                if (!client->messageQueue.empty()) {
                    message = client->messageQueue.front();
                    client->messageQueue.pop();
                }
            }
            
            if (!message.empty()) {
                sink.write(message.c_str(), message.length());
            }
            
            // Keep alive
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return true;
        }
    );
}

void RestApi::broadcastSSE(const std::string& eventType,
                          const std::string& data) {
    std::stringstream ss;
    ss << "event: " << eventType << "\n";
    ss << "data: " << data << "\n\n";
    std::string message = ss.str();
    
    std::lock_guard<std::mutex> lock(m_sseClientsMutex);
    for (auto& client : m_sseClients) {
        if (client->connected) {
            std::lock_guard<std::mutex> clientLock(client->mutex);
            client->messageQueue.push(message);
        }
    }
}
```

### CORS Configuration

```cpp
void RestApi::setupRoutes() {
    // Enable CORS for all routes
    m_server->set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
    });
    
    // Handle preflight requests
    m_server->Options("/(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;  // No content for OPTIONS
    });
}
```

### Error Handling

```cpp
// Standardized error response
json createErrorResponse(const std::string& message, int code = 400) {
    return {
        {"status", "error"},
        {"message", message},
        {"code", code}
    };
}

// Usage in handlers
void RestApi::handleProcessInput(const httplib::Request& req,
                                httplib::Response& res) {
    try {
        // Process request
        // ...
    } catch (const std::invalid_argument& e) {
        auto error = createErrorResponse(e.what(), 400);
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    } catch (const std::runtime_error& e) {
        auto error = createErrorResponse(e.what(), 500);
        res.set_content(error.dump(), "application/json");
        res.status = 500;
    } catch (const std::exception& e) {
        auto error = createErrorResponse("Internal server error", 500);
        res.set_content(error.dump(), "application/json");
        res.status = 500;
    }
}
```

## Integration Between APIs

### Cronus Main Application

**File:** `server/cronus/cronus.cpp`

```cpp
class Cronus {
public:
    void run(const std::string& resourcePath) {
        // Start REST API for web UI
        startRestApi(8080);
        
        // Start worker thread for processing
        m_workerThread = std::thread(&Cronus::workerLoop, this);
        
        // Initialize components
        m_sourceMap = std::make_shared<SourceMap>(m_workingDir);
        m_model = std::make_shared<Model>(config.getModel(), config.getProvider());
        m_coder = std::make_shared<agents::Coder>(m_sourceMap, m_model);
    }
    
    void startRestApi(int port) {
        m_restApi = std::make_unique<RestApi>(*this, port);
        m_restApi->start();
    }
    
    void stopRestApi() {
        if (m_restApi) {
            m_restApi->stop();
        }
    }
};
```

### Communication Flow

```
Web Client
    ↓ (HTTP/REST)
REST API Server (Cronus)
    ↓ (Task Queue)
Worker Thread
    ↓ (gRPC)
LoreForge Server
    ↓ (Vector Search + LLM)
Response
    ↓ (SSE)
Web Client
```

## Performance Considerations

### gRPC Optimization

**Connection Pooling:**
```cpp
// Reuse channels across requests
class LoreForgeClient {
    std::shared_ptr<Channel> m_channel;
    std::unique_ptr<LoreForge::Stub> m_stub;
    
public:
    LoreForgeClient(const std::string& address) {
        m_channel = grpc::CreateChannel(address, 
                                       grpc::InsecureChannelCredentials());
        m_stub = LoreForge::NewStub(m_channel);
    }
};
```

**Compression:**
```cpp
// Enable compression for large payloads
ClientContext context;
context.set_compression_algorithm(GRPC_COMPRESS_GZIP);
```

**Deadline:**
```cpp
// Set request timeout
ClientContext context;
context.set_deadline(std::chrono::system_clock::now() + 
                    std::chrono::seconds(30));
```

### REST API Optimization

**Connection Keep-Alive:**
```cpp
m_server->set_keep_alive_max_count(100);
m_server->set_keep_alive_timeout_sec(5);
```

**Response Compression:**
```cpp
// Compress large responses
m_server->set_payload_max_length(10 * 1024 * 1024);  // 10MB
```

**Thread Pool:**
```cpp
// Use thread pool for request handling
m_server->new_task_queue = [] {
    return new httplib::ThreadPool(8);
};
```

## Security Considerations

### gRPC Security

**TLS/SSL:**
```cpp
// Production: Use SSL credentials
auto ssl_opts = grpc::SslServerCredentialsOptions();
ssl_opts.pem_root_certs = ReadFile("ca.pem");
ssl_opts.pem_key_cert_pairs.push_back({
    ReadFile("server-key.pem"),
    ReadFile("server-cert.pem")
});

auto creds = grpc::SslServerCredentials(ssl_opts);
builder.AddListeningPort(server_address, creds);
```

**Authentication:**
```cpp
// API key authentication
class AuthMetadataProcessor : public grpc::AuthMetadataProcessor {
    Status Process(const InputMetadata& auth_metadata,
                  grpc::AuthContext* context,
                  OutputMetadata* consumed_auth_metadata,
                  OutputMetadata* response_metadata) override {
        auto api_key = auth_metadata.find("x-api-key");
        if (api_key == auth_metadata.end()) {
            return Status(StatusCode::UNAUTHENTICATED, "Missing API key");
        }
        
        if (!validateApiKey(api_key->second.data())) {
            return Status(StatusCode::UNAUTHENTICATED, "Invalid API key");
        }
        
        return Status::OK;
    }
};
```

### REST API Security

**HTTPS:**
```cpp
// Production: Use HTTPS
m_server = std::make_unique<httplib::SSLServer>(
    "server-cert.pem",
    "server-key.pem"
);
```

**Rate Limiting:**
```cpp
// Simple rate limiter
class RateLimiter {
    std::unordered_map<std::string, int> m_requests;
    std::mutex m_mutex;
    
public:
    bool allowRequest(const std::string& clientIp) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& count = m_requests[clientIp];
        if (count >= 100) {  // 100 requests per minute
            return false;
        }
        count++;
        return true;
    }
};
```

## Testing

### gRPC Tests

```cpp
TEST_F(LoreForgeTest, ParseFile_ValidCpp) {
    ParseFileRequest request;
    request.set_file_path("test.cpp");
    request.set_content("int main() { return 0; }");
    
    ParseFileResponse response;
    ClientContext context;
    
    Status status = stub->ParseFile(&context, &request, &response);
    
    ASSERT_TRUE(status.ok());
    EXPECT_TRUE(response.success());
}

TEST_F(LoreForgeTest, QueryCodebase_Streaming) {
    QueryRequest request;
    request.set_query("Explain the authentication flow");
    request.set_model("gpt-4");
    request.set_codebase_name("test-project");
    
    ClientContext context;
    auto reader = stub->QueryCodebase(&context, &request);
    
    std::string fullResponse;
    QueryResponse response;
    while (reader->Read(&response)) {
        fullResponse += response.response_chunk();
    }
    
    ASSERT_TRUE(reader->Finish().ok());
    EXPECT_FALSE(fullResponse.empty());
}
```

### REST API Tests

```cpp
TEST(RestApi, ProcessInput) {
    httplib::Client client("localhost", 8080);
    
    json body = {
        {"input", "add tests"},
        {"context", {{"currentFile", "src/app.js"}}}
    };
    
    auto res = client.Post("/api/input", body.dump(), "application/json");
    
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    
    auto response = json::parse(res->body);
    EXPECT_EQ(response["status"], "queued");
}
```

This dual-API architecture provides both high-performance, strongly-typed gRPC communication for code intelligence and flexible, web-friendly REST endpoints for user interfaces.
