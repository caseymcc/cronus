# Cronus Server Startup Notification System

This document describes the startup notification system that allows web clients and the standalone app to automatically discover and connect to the Cronus server when it starts.

## Overview

When the Cronus server starts, it sends a UDP notification to a configurable host and port (default: `localhost:8999`). Web clients and the Electron app listen on this port and automatically connect to the server when they receive the notification.

## Architecture

```
┌─────────────────────────────┐
│   Cronus Server             │
│   (port 9000)               │
│                             │
│   On startup:               │
│   - Sends UDP notification  │
│     to localhost:8999       │
│   - Contains server URL     │
└──────────────┬──────────────┘
               │ UDP Packet
               │ {type: "server_startup",
               │  serverUrl: "http://localhost:9000"}
               │
    ┌──────────┴──────────────┬───────────────┐
    │                         │               │
    ▼                         ▼               ▼
┌────────────────┐  ┌─────────────────┐  ┌──────────────┐
│ Web Client     │  │ Notification    │  │ Electron App │
│ (Browser)      │  │ Listener        │  │ (Standalone) │
│                │  │ (Node.js)       │  │              │
│                │  │                 │  │              │
│ Connects via   │  │ WS Port: 8998   │  │ UDP: 8999    │
│ WebSocket to   │◄─┤ UDP: 8999       │  │ Built-in     │
│ localhost:8998 │  │ Forwards via WS │  │ listener     │
└────────────────┘  └─────────────────┘  └──────────────┘
```

## Components

### 1. Server-Side (C++)

**Files:**
- `server/cronus/startupNotifier.h` - Notification sender interface
- `server/cronus/startupNotifier.cpp` - UDP notification implementation
- `server/cronus/config.h` - Configuration for notification host/port
- `server/cronus/main.cpp` - Sends notification after web server starts

**Configuration:**
```cpp
// In Config class (defaults)
std::string m_notificationHost{ "localhost" };
int m_notificationPort{ 8999 };
```

**Notification Format:**
```json
{
  "type": "server_startup",
  "serverUrl": "http://localhost:9000",
  "timestamp": 1704499200
}
```

### 2. Web Client (Browser Mode)

**Files:**
- `clients/web/notification-listener.js` - Node.js UDP listener and WebSocket bridge
- `clients/web/src/hooks/useServerNotifications.js` - React hook for receiving notifications
- `clients/web/src/App.js` - Integration and auto-connection logic

**How It Works:**
1. Run `npm run dev` to start both React dev server and notification listener
2. Notification listener receives UDP packets on port 8999
3. Converts to WebSocket and broadcasts to web clients on port 8998
4. Web client connects to WebSocket endpoint and waits for notifications
5. When notification received, web client reloads to connect to new server

**Scripts:**
```bash
# Start web client with notification listener
cd clients/web
npm run dev

# Or manually:
npm run notification-listener  # Terminal 1
npm start                       # Terminal 2
```

### 3. Electron App (Standalone Mode)

**Files:**
- `clients/app/src/main.ts` - Built-in UDP listener in Electron main process

**How It Works:**
1. On app start, creates UDP socket listening on port 8999
2. Receives server startup notifications directly
3. Automatically updates CronusClient to connect to new server URL
4. Notifies renderer process of server discovery

**Benefits:**
- No external processes needed
- Native UDP support in Node.js
- Automatic reconnection to server

## Configuration

### Server Configuration

Set notification target in Cronus config:

```cpp
// In config or via environment
Config::instance().setNotificationHost("localhost");
Config::instance().setNotificationPort(8999);
```

### Web Client Configuration

Set notification listener port:

```bash
# Environment variable
export NOTIFICATION_PORT=8999
export HTTP_PORT=8998

# Then run
npm run notification-listener
```

### Electron App Configuration

Set notification port:

```bash
# Environment variable
export NOTIFICATION_PORT=8999

# Then run
npm start
```

## Usage Scenarios

### Scenario 1: Start Server First

```bash
# Terminal 1: Start Cronus server
./run_local.sh ./build/linux_x64_debug/server/cronus/cronus --web --port 9000

# Server sends startup notification to localhost:8999

# Terminal 2: Start web client
cd clients/web
npm run dev

# Client connects to notification listener and waits...
# When notification received, client auto-connects to server
```

### Scenario 2: Start Client First

```bash
# Terminal 1: Start web client
cd clients/web
npm run dev

# Shows "Waiting for server..." status

# Terminal 2: Start Cronus server
./run_local.sh ./build/linux_x64_debug/server/cronus/cronus --web --port 9000

# Server sends notification
# Client receives it and auto-connects
```

### Scenario 3: Electron App

```bash
# Terminal 1: Start Electron app
cd clients/app
npm start

# Shows "Server: Disconnected" and listens on UDP 8999

# Terminal 2: Start Cronus server
./run_local.sh ./build/linux_x64_debug/server/cronus/cronus --web --port 9000

# App receives notification and auto-connects
```

## Advantages

1. **Zero Configuration**: Works out of the box with defaults
2. **Automatic Discovery**: Clients don't need to know server URL in advance
3. **Fast Reconnection**: Server restarts are detected immediately
4. **Simple Protocol**: UDP broadcast is fire-and-forget, no complex handshake
5. **Dual Mode**: Works in both browser and Electron

## Network Ports

| Port | Service | Protocol | Purpose |
|------|---------|----------|---------|
| 9000 | Cronus Server | HTTP/WS | Main server API and WebSocket |
| 8999 | Notification Listener | UDP | Receives server startup notifications |
| 8998 | Notification Bridge | WebSocket | Web client WebSocket connection (browser mode only) |
| 3000 | React Dev Server | HTTP | Web client development server |

## Troubleshooting

### Notifications Not Received

**Check UDP listener is running:**
```bash
# For web client
lsof -i UDP:8999

# Should show node process (browser mode) or electron process (standalone)
```

**Test notification manually:**
```bash
# Send test notification
echo '{"type":"server_startup","serverUrl":"http://localhost:9000"}' | nc -u localhost 8999
```

**Check firewall:**
```bash
# Ensure UDP port 8999 is not blocked
sudo ufw allow 8999/udp
```

### Web Client Not Connecting

**Check notification listener:**
```bash
curl http://localhost:8998/health

# Should return:
# {"status":"ok","udpPort":8999,"wsPort":8998,"clients":1}
```

**Check WebSocket connection:**
Open browser console and look for:
```
Connected to notification listener
```

### Electron App Not Connecting

**Check console output:**
```
UDP notification listener active on 0.0.0.0:8999
```

**Check environment:**
```bash
echo $NOTIFICATION_PORT  # Should be 8999 or empty (uses default)
```

## Future Enhancements

Potential improvements:

1. **mDNS/Bonjour**: Use service discovery for network-wide announcements
2. **Multiple Servers**: Support connecting to multiple Cronus instances
3. **Server Health**: Periodic heartbeat notifications
4. **Encryption**: Sign/encrypt notifications for security
5. **Custom Ports**: UI for changing notification port
6. **Discovery History**: Remember previously discovered servers
