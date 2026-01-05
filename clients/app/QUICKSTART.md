# Cronus Standalone Debug Client - Quick Start

## What is it?

A standalone Electron application for monitoring Cronus servers in real-time. It provides:
- Real-time log streaming
- Auto-reconnection when server starts/restarts
- Log filtering and search
- Professional dark theme UI
- No VSCode required!

## Installation & Building

### 1. Build all components

**Using the convenience script:**
```bash
cd /home/caseymcc/projects/cronus
./run_local.sh -d
```

This will:
- Build all shared libraries in Docker
- Install dependencies on the host
- Launch the debug client

**Or manually:**
```bash
cd /home/caseymcc/projects/cronus/clients
./build-debug-client.sh
```

This will:
- Install all dependencies
- Build `@cronus/shared` library
- Build `@cronus/shared-ui` components
- Build the Electron debug client

### 2. Start Cronus server

In Docker (recommended):
```bash
cd /home/caseymcc/projects/cronus
./run_local.sh ./build/linux_x64_debug/server/cronus/cronus
```

### 3. Launch debug client

**Quick start (recommended):**
```bash
cd /home/caseymcc/projects/cronus
./run_local.sh -d
```

**Or manually:**
```bash
cd /home/caseymcc/projects/cronus/clients/debug
npm start
```

Or with DevTools for development:
```bash
npm run dev
```

## Using the Debug Client

### Interface Overview

```
┌──────────────────────────────────────────────────────┐
│ Cronus Debug Client    [●] Connected  [URL] [Connect]│
├──────────┬───────────────────────────────────────────┤
│ FILTERS  │ LOGS                                      │
│          │                                           │
│ Levels   │ [timestamp] [INFO] Loading config...     │
│ [D][I]   │ [timestamp] [DEBUG] Processing request   │
│ [W][E]   │ [timestamp] [ERROR] Connection failed    │
│          │                                           │
│ Search   │                                           │
│ [____]   │                                           │
│          │                                           │
│ [Clear]  │                                           │
│          │                                           │
│ Stats    │                                           │
│ Total: 0 │                                           │
│ Shown: 0 │                                           │
└──────────┴───────────────────────────────────────────┘
```

### Features

1. **Connection Status** (top right)
   - Green dot = Connected
   - Red dot = Disconnected
   - Auto-reconnects when server available

2. **Server URL Input**
   - Default: `http://localhost:9000`
   - Change to connect to remote servers
   - Press "Connect" to apply

3. **Level Filters** (sidebar)
   - D = Debug (gray)
   - I = Info (cyan/green)
   - W = Warning (yellow)
   - E = Error (red)
   - Click to toggle visibility

4. **Search Filter**
   - Type to filter logs by text
   - Case-insensitive
   - Real-time filtering

5. **Clear Logs**
   - Removes all logs from display
   - New logs continue streaming

6. **Auto-scroll**
   - Enabled by default
   - Automatically scrolls to newest logs
   - Uncheck to manually scroll

## Configuration

### Environment Variables

```bash
# Set custom server URL
export CRONUS_SERVER_URL=http://192.168.1.100:9000
npm start
```

### Server URL

You can change the server URL in the UI:
1. Enter URL in the text input (top right)
2. Click "Connect"
3. Client will disconnect from old server and connect to new one

## Keyboard Shortcuts (Planned)

- `Ctrl+F`: Focus search
- `Ctrl+L`: Clear logs
- `Ctrl+Shift+I`: Toggle DevTools (dev mode only)

## Troubleshooting

### Client won't connect

1. Check server is running:
   ```bash
   curl http://localhost:9000/api/health
   ```

2. Check server URL is correct in UI

3. Check network firewall settings

### No logs showing

1. Verify connection status shows "Connected"
2. Check level filters - make sure at least one is active
3. Clear search filter if active
4. Click "Clear Logs" to reset

### Build errors

1. Make sure you're in the correct directory:
   ```bash
   cd /home/caseymcc/projects/cronus/clients
   ```

2. Clean and rebuild:
   ```bash
   rm -rf node_modules package-lock.json
   rm -rf */node_modules */package-lock.json
   ./build-debug-client.sh
   ```

3. Check Node.js version (requires Node 18+):
   ```bash
   node --version
   ```

## Development

### Project Structure

```
clients/debug/
├── src/
│   ├── main.ts          # Electron main process
│   └── preload.ts       # Secure IPC bridge
├── renderer/
│   ├── index.html       # UI markup
│   ├── styles.css       # UI styling
│   └── renderer.js      # UI logic
├── package.json
├── tsconfig.json
└── README.md
```

### Running in Development

```bash
npm run dev
```

This:
- Builds TypeScript
- Starts Electron
- Opens DevTools automatically
- Enables hot reload (manual restart required)

### Building for Distribution

```bash
npm run package
```

Creates installers in `dist-electron/`:
- Linux: `.AppImage` and `.deb`
- Can be configured for Windows/Mac in `package.json`

## Next Steps

1. **Add More Panels**: Config viewer, agent status, file watcher
2. **Keyboard Shortcuts**: Implement shortcuts for common actions
3. **Log Export**: Add ability to export logs to file
4. **Themes**: Add light theme option
5. **Settings Panel**: Persistent configuration
6. **Multi-server**: Connect to multiple servers simultaneously

## Related Documentation

- [Full Implementation Guide](../../docs/DEBUG-CLIENT-COMPLETE.md)
- [Architecture Overview](../../docs/architecture.md)
- [Client Development Guide](../../docs/client-development.md)
- [API Documentation](../../docs/api-documentation.md)

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review the full documentation
3. Check server logs for errors
4. Open an issue on GitHub
