# @cronus/shared

Shared core library for Cronus clients. This package provides a unified API client, data models, and utilities used across all Cronus client implementations (web, VSCode, debug, CLI).

## Features

- **CronusClient**: Unified API client with automatic reconnection
- **SSEManager**: Server-Sent Events manager with exponential backoff
- **Type Definitions**: Shared TypeScript types for all data models
- **Logger**: Configurable logging utility

## Installation

```bash
npm install @cronus/shared
```

## Usage

### Basic Client Setup

```typescript
import { CronusClient } from '@cronus/shared';

const client = new CronusClient({
  baseUrl: 'http://localhost:9000',
  reconnectInterval: 2000,
  maxReconnectAttempts: 0, // Infinite
});

// Start the client
client.start();

// Listen for events
client.on('connected', () => {
  console.log('Connected to Cronus server');
});

client.on('message', (msg) => {
  console.log('Received message:', msg);
});

client.on('directory-update', (tree) => {
  console.log('Directory updated:', tree);
});

// Send a message
await client.sendMessage('Hello, Cronus!');

// Fetch file tree
const fileTree = await client.fetchSourceMap();

// Fetch file content
const file = await client.fetchFileContent('/path/to/file.js');

// Stop the client
client.stop();
```

### Direct SSE Management

```typescript
import { SSEManager } from '@cronus/shared';

const sseManager = new SSEManager({
  baseUrl: 'http://localhost:9000',
  reconnectInterval: 2000,
  reconnectBackoff: true,
});

sseManager.on('connected', () => {
  console.log('SSE connected');
});

sseManager.on('event', (event) => {
  console.log('SSE event:', event.type, event.data);
});

sseManager.connect();
```

### Logging

```typescript
import { Logger } from '@cronus/shared';

const logger = new Logger({
  level: 'debug',
  prefix: 'MyApp',
  handler: (entry) => {
    // Custom log handling
    myCustomLogger(entry);
  },
});

logger.debug('Debug message', { extra: 'data' });
logger.info('Info message');
logger.warning('Warning message');
logger.error('Error message');
```

## API Reference

### CronusClient

#### Constructor

```typescript
new CronusClient(config: CronusClientConfig)
```

**Config Options:**
- `baseUrl`: Server base URL (required)
- `reconnectInterval`: Milliseconds between reconnect attempts (default: 2000)
- `maxReconnectAttempts`: Maximum reconnection attempts, 0 = infinite (default: 0)
- `healthCheckInterval`: Milliseconds between health checks (default: 30000)

#### Methods

- `start()`: Start the client (connect SSE and health checks)
- `stop()`: Stop the client
- `isConnected()`: Check connection status
- `getConnectionStatus()`: Get detailed connection status
- `fetchSourceMap()`: Fetch file tree
- `fetchFileContent(path)`: Fetch file content
- `sendMessage(message, sessionId?)`: Send message to server
- `checkHealth()`: Check server health

#### Events

- `connected`: Server connection established
- `disconnected`: Server connection lost
- `reconnecting`: Attempting to reconnect
- `connection-status-change`: Connection status changed
- `message`: New message received
- `directory-update`: File tree updated
- `log`: Log entry received
- `agent-status`: Agent status update
- `error`: Error occurred

### SSEManager

#### Constructor

```typescript
new SSEManager(config: SSEManagerConfig)
```

**Config Options:**
- `baseUrl`: Server base URL (required)
- `reconnectInterval`: Milliseconds between reconnect attempts (default: 2000)
- `maxReconnectAttempts`: Maximum reconnection attempts, 0 = infinite (default: 0)
- `reconnectBackoff`: Use exponential backoff (default: true)

#### Methods

- `connect()`: Connect to SSE endpoint
- `disconnect()`: Disconnect from SSE endpoint
- `isConnected()`: Check connection status
- `getReconnectAttempts()`: Get current reconnect attempt count

#### Events

- `connected`: Connection established
- `disconnected`: Connection closed
- `reconnecting`: Attempting to reconnect
- `status`: Connection status changed
- `event`: Generic SSE event received
- `error`: Error occurred
- `{eventType}`: Specific event types (e.g., 'message', 'directory_update')

## Development

### Build

```bash
npm run build
```

### Watch Mode

```bash
npm run watch
```

### Clean

```bash
npm run clean
```

## License

MIT
