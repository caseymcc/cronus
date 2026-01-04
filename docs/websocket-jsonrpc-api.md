# WebSocket + JSON-RPC API Specification

## Overview

Cronus uses WebSocket connections with JSON-RPC 2.0 for all client-server communication. This provides:
- Real-time bidirectional communication
- Structured request/response with JSON-RPC 2.0
- Event notifications from server to clients
- Single persistent connection (no polling needed)

## Connection

**Endpoint:** `ws://localhost:<port>/ws`

**Protocol:** WebSocket with JSON-RPC 2.0 messages

## JSON-RPC 2.0 Message Format

### Request
```json
{
  "jsonrpc": "2.0",
  "method": "methodName",
  "params": { ... },
  "id": "unique-request-id"
}
```

### Response (Success)
```json
{
  "jsonrpc": "2.0",
  "result": { ... },
  "id": "unique-request-id"
}
```

### Response (Error)
```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32600,
    "message": "Error description"
  },
  "id": "unique-request-id"
}
```

### Notification (Server → Client, no response expected)
```json
{
  "jsonrpc": "2.0",
  "method": "notificationName",
  "params": { ... }
}
```

## Methods

### 1. input
Send user input to the agent for processing.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "input",
  "params": {
    "text": "Create a hello world function"
  },
  "id": "1"
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "success": true,
    "message": "Input received and processing"
  },
  "id": "1"
}
```

### 2. getSourceMap
Get the current source map (file tree structure).

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getSourceMap",
  "params": {},
  "id": "2"
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "fileTree": {
      "name": "root",
      "path": "/path/to/project",
      "type": "directory",
      "children": [...]
    }
  },
  "id": "2"
}
```

### 3. getFile
Get content of a specific file.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getFile",
  "params": {
    "path": "/path/to/file.cpp"
  },
  "id": "3"
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "path": "/path/to/file.cpp",
    "content": "file contents...",
    "language": "cpp"
  },
  "id": "3"
}
```

### 4. getDirectory
Get contents of a directory.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getDirectory",
  "params": {
    "path": "/path/to/dir"
  },
  "id": "4"
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "path": "/path/to/dir",
    "entries": [
      {"name": "file1.cpp", "type": "file"},
      {"name": "subdir", "type": "directory"}
    ]
  },
  "id": "4"
}
```

### 5. getLogs
Get recent log messages.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "getLogs",
  "params": {
    "limit": 100
  },
  "id": "5"
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "logs": [
      {"timestamp": "2026-01-04T...", "level": "info", "message": "..."}
    ]
  },
  "id": "5"
}
```

## Notifications (Server → Client)

### message
Agent response or system message.

```json
{
  "jsonrpc": "2.0",
  "method": "message",
  "params": {
    "role": "assistant",
    "content": "Here's the code...",
    "timestamp": "2026-01-04T..."
  }
}
```

### log
Log message from server.

```json
{
  "jsonrpc": "2.0",
  "method": "log",
  "params": {
    "level": "info",
    "message": "Server started",
    "timestamp": "2026-01-04T..."
  }
}
```

### directoryUpdate
File system changes.

```json
{
  "jsonrpc": "2.0",
  "method": "directoryUpdate",
  "params": {
    "fileTree": { ... }
  }
}
```

### response
Agent response (for streaming/async responses).

```json
{
  "jsonrpc": "2.0",
  "method": "response",
  "params": {
    "message": "Generated code or response",
    "complete": true
  }
}
```

## Error Codes

- `-32700`: Parse error
- `-32600`: Invalid Request
- `-32601`: Method not found
- `-32602`: Invalid params
- `-32603`: Internal error
- `-32000` to `-32099`: Server-defined errors

## Connection Lifecycle

1. Client connects to `/ws`
2. Server sends initial `directoryUpdate` notification with file tree
3. Client sends JSON-RPC requests as needed
4. Server sends notifications for events (logs, messages, updates)
5. Client can close connection when done

## Example Session

```
Client → Server: (WebSocket CONNECT /ws)
Server → Client: {"jsonrpc":"2.0","method":"directoryUpdate","params":{...}}

Client → Server: {"jsonrpc":"2.0","method":"input","params":{"text":"hello"},"id":"1"}
Server → Client: {"jsonrpc":"2.0","result":{"success":true},"id":"1"}

Server → Client: {"jsonrpc":"2.0","method":"message","params":{"role":"assistant","content":"..."}}
Server → Client: {"jsonrpc":"2.0","method":"response","params":{"message":"...","complete":true}}
```
