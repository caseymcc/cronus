# @cronus/shared-ui

Shared React UI components and hooks for Cronus clients. This package provides reusable React components and custom hooks that work across web, VSCode webview, and Electron clients.

## Features

- **React Hooks**: Custom hooks for Cronus client integration
- **UI Components**: Reusable components for common UI patterns
- **Type Safety**: Full TypeScript support
- **Platform Agnostic**: Works in browser, VSCode webview, and Electron

## Installation

```bash
npm install @cronus/shared-ui @cronus/shared react react-dom
```

## Usage

### useCronusConnection Hook

Connect to a Cronus server with automatic reconnection:

```typescript
import { useCronusConnection } from '@cronus/shared-ui';

function App() {
  const { client, connected, reconnecting, connectionStatus } = useCronusConnection({
    baseUrl: 'http://localhost:9000',
    autoConnect: true,
  });

  return (
    <div>
      <p>Status: {connected ? 'Connected' : 'Disconnected'}</p>
      {reconnecting && <p>Reconnecting...</p>}
    </div>
  );
}
```

### useFileTree Hook

Manage file tree data with automatic updates:

```typescript
import { useFileTree, useCronusConnection } from '@cronus/shared-ui';

function FileExplorer() {
  const { client } = useCronusConnection({ baseUrl: 'http://localhost:9000' });
  const { fileTree, loading, error, refresh } = useFileTree({
    client,
    autoFetch: true,
  });

  if (loading) return <div>Loading...</div>;
  if (error) return <div>Error: {error.message}</div>;

  return (
    <div>
      <button onClick={refresh}>Refresh</button>
      {/* Render file tree */}
    </div>
  );
}
```

### useMessages Hook

Handle chat messages:

```typescript
import { useMessages, useCronusConnection } from '@cronus/shared-ui';

function Chat() {
  const { client } = useCronusConnection({ baseUrl: 'http://localhost:9000' });
  const { messages, sendMessage, loading } = useMessages({
    client,
    sessionId: 'my-session',
  });

  const handleSend = async () => {
    await sendMessage('Hello, Cronus!');
  };

  return (
    <div>
      {messages.map((msg) => (
        <div key={msg.id}>
          <strong>{msg.role}:</strong> {msg.content}
        </div>
      ))}
      <button onClick={handleSend} disabled={loading}>
        Send
      </button>
    </div>
  );
}
```

### ConnectionStatus Component

Display connection status:

```typescript
import { ConnectionStatus, useCronusConnection } from '@cronus/shared-ui';

function App() {
  const { connectionStatus, reconnecting } = useCronusConnection({
    baseUrl: 'http://localhost:9000',
  });

  return (
    <ConnectionStatus
      status={connectionStatus}
      reconnecting={reconnecting}
    />
  );
}
```

## API Reference

### Hooks

#### useCronusConnection(options)

**Options:**
- `baseUrl`: Server base URL (required)
- `autoConnect`: Auto-connect on mount (default: true)

**Returns:**
- `client`: CronusClient instance
- `connected`: Connection status boolean
- `connectionStatus`: Detailed connection status
- `reconnecting`: Reconnection status boolean
- `connect()`: Manually connect
- `disconnect()`: Manually disconnect

#### useFileTree(options)

**Options:**
- `client`: CronusClient instance
- `autoFetch`: Auto-fetch on mount (default: true)

**Returns:**
- `fileTree`: Current file tree data
- `loading`: Loading status
- `error`: Error object if failed
- `refresh()`: Manually refresh file tree

#### useMessages(options)

**Options:**
- `client`: CronusClient instance
- `sessionId`: Optional session identifier

**Returns:**
- `messages`: Array of messages
- `sendMessage(content)`: Send a message
- `clearMessages()`: Clear all messages
- `loading`: Loading status

### Components

#### ConnectionStatus

Display connection status with visual indicators.

**Props:**
- `status`: ConnectionStatus object
- `reconnecting`: Boolean indicating reconnection state
- `className`: Optional CSS class name

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
