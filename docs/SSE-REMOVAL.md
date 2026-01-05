# SSE Bridge Removal - Architecture Update

## Summary

The Cronus application has been updated to use WebSocket + JSON-RPC 2.0 exclusively for client-server communication, removing the duplicate RestApi implementation that used Server-Sent Events (SSE).

## Changes Made

### 1. Server Side
- ✅ **Removed**: `RestApi` class (used cpp-httplib with SSE)
- ✅ **Kept**: `WebServer` class (uses Crow with WebSocket + JSON-RPC 2.0)
- ✅ **Removed**: All SSE endpoint handlers (`/api/events`)
- ✅ **Primary Communication**: WebSocket endpoint at `/ws`

### 2. Shared Client Library (`clients/shared`)
- ✅ **Removed**: `SSEManager.ts` - SSE connection manager
- ✅ **Removed**: `SSEEvent` type from `types.ts`
- ✅ **Updated**: `CronusClient.ts` - Uses WebSocket exclusively
- ✅ **Exports**: Removed SSEManager and SSEManagerConfig from index.ts

### 3. Web Client (`clients/web`)
- ✅ **Added**: `@cronus/shared` dependency
- ✅ **Added**: `useCronusClient` hook for WebSocket communication
- ✅ **Removed**: Direct EventSource usage for SSE
- ✅ **Updated**: `App.js` to use CronusClient instead of SSE
- ✅ **Kept**: `notification-listener.js` for startup notifications only

### 4. Build Configuration
- ✅ **Verified**: No RestApi references in CMakeLists.txt

## Architecture After Changes

```
┌─────────────────────────────────────────────────┐
│   Cronus Server (C++)                           │
│                                                 │
│   WebServer (Crow)                              │
│   - WebSocket endpoint: /ws                     │
│   - JSON-RPC 2.0 protocol                       │
│   - Bidirectional real-time communication       │
└────────────────┬────────────────────────────────┘
                 │
                 │ WebSocket
                 │
    ┌────────────┴───────────┬────────────────┐
    │                        │                │
    ▼                        ▼                ▼
┌───────────────┐  ┌──────────────────┐  ┌─────────────┐
│ Web Client    │  │ Electron App     │  │ VS Code Ext │
│               │  │                  │  │             │
│ CronusClient  │  │  CronusClient    │  │ (Future)    │
│ (@cronus/     │  │  (@cronus/       │  │             │
│  shared)      │  │   shared)        │  │             │
│               │  │                  │  │             │
│ WebSocket     │  │  WebSocket       │  │  WebSocket  │
└───────────────┘  └──────────────────┘  └─────────────┘
```

## notification-listener.js Purpose

**Important**: The `notification-listener.js` file is **NOT** removed because it serves a different purpose:

### What It Does
- Listens for UDP startup notifications from Cronus server
- Converts UDP notifications to SSE for browser clients
- Used **only** for server discovery in browser development mode

### Why It's Still Needed
1. **Browser Limitation**: Browsers cannot listen to UDP packets directly (security restriction)
2. **Development Workflow**: Allows web client to auto-discover when server starts
3. **Not for Communication**: Only for initial server discovery, not ongoing communication
4. **Electron Doesn't Need It**: Electron can listen to UDP directly

### Usage
```bash
# Start web client with notification listener in development
cd clients/web
npm run dev  # Runs both notification-listener and React dev server
```

## Communication Patterns

### Before (RestApi + SSE)
- REST endpoints for requests (`POST /api/input`, `GET /api/file`, etc.)
- SSE stream (`/api/events`) for real-time updates
- One-way server-to-client push for events
- Polling required for some data

### After (WebSocket + JSON-RPC 2.0)
- Single WebSocket connection (`/ws`)
- JSON-RPC 2.0 for requests/responses
- Bidirectional real-time communication
- Server can push notifications at any time
- No polling needed

## Benefits of WebSocket-Only Approach

1. **Single Connection**: One persistent connection instead of HTTP + SSE
2. **Bidirectional**: Server and client can both initiate messages
3. **Lower Latency**: No HTTP overhead for each message
4. **Standard Protocol**: JSON-RPC 2.0 is well-defined and widely used
5. **Simplified Architecture**: One server implementation instead of two
6. **Better Resource Usage**: Fewer connections and less overhead

## Migration Guide for Future Clients

When adding new clients (CLI, VS Code extension, etc.):

1. **Use `@cronus/shared`**: Import CronusClient from the shared library
2. **No SSE**: Don't implement SSE connections
3. **WebSocket Only**: Connect to `ws://localhost:9000/ws`
4. **JSON-RPC**: Use JSON-RPC 2.0 message format
5. **Event Handling**: Subscribe to CronusClient events for notifications

Example:
```typescript
import { CronusClient } from '@cronus/shared';

const client = new CronusClient({
  baseUrl: 'http://localhost:9000',
  reconnectInterval: 2000,
});

client.on('connected', () => console.log('Connected'));
client.on('message', (msg) => handleMessage(msg));
client.on('log', (log) => handleLog(log));

client.start();
```

## Documentation Updates Needed

- ✅ This architecture update document
- ⚠️ Update `docs/architecture.md` to remove RestApi references
- ⚠️ Update `docs/current_state.md` to reflect WebSocket-only
- ⚠️ Update `docs/api-documentation.md` to focus on WebSocket/JSON-RPC
- ⚠️ Update `docs/client-development.md` to show CronusClient usage
- ⚠️ Update `clients/README.md` to remove SSEManager references

## Testing Recommendations

1. **Web Client**: Test WebSocket connection and reconnection
2. **Electron App**: Verify it still works with WebSocket
3. **Server**: Ensure WebServer handles all client requests
4. **Startup Notifications**: Test notification-listener.js still works
5. **Reconnection**: Test automatic reconnection on disconnection
6. **Error Handling**: Test connection failures and error recovery

## Rollback Plan (If Needed)

If issues are found, the old code is available in git history:
```bash
git log --all --full-history -- "**/restApi.*"
git log --all --full-history -- "**/SSEManager.ts"
```

However, rolling back is **not recommended** as:
- WebSocket is more modern and efficient
- Duplication was causing maintenance issues
- All clients should standardize on one protocol
