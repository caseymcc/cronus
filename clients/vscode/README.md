# Cronus VSCode Extension

Debug and monitor your Cronus AI assistant server directly from Visual Studio Code.

## Features

- **Auto-Reconnection**: Automatically reconnects to Cronus server when it starts
- **Real-time Monitoring**: View server status, messages, and logs in real-time
- **Debug Panel**: Dedicated webview panel for debugging
- **Server-Sent Events**: Live updates via SSE connection
- **Configurable**: Customize server URL, reconnection settings, and more

## Installation

### From Source

1. Navigate to the extension directory:
```bash
cd clients/vscode
```

2. Install dependencies:
```bash
npm install
```

3. Compile the extension:
```bash
npm run compile
```

4. Press `F5` in VSCode to open a new window with the extension loaded

### From VSIX

1. Package the extension:
```bash
npm run package
```

2. Install the `.vsix` file in VSCode:
   - Open VSCode
   - Go to Extensions view
   - Click "..." menu → "Install from VSIX..."
   - Select the generated `.vsix` file

## Usage

### Opening the Debug Panel

- **Command Palette**: `Cronus: Open Debug Panel`
- **Activity Bar**: Click the Cronus icon in the activity bar

### Commands

- `Cronus: Open Debug Panel` - Open the debug panel
- `Cronus: Reconnect to Server` - Manually reconnect to the server
- `Cronus: Disconnect from Server` - Disconnect from the server
- `Cronus: Clear Logs` - Clear the debug logs

### Configuration

Settings are available in VSCode settings (`Ctrl+,` or `Cmd+,`):

```json
{
  "cronus.serverUrl": "http://localhost:9000",
  "cronus.autoConnect": true,
  "cronus.reconnectInterval": 2000,
  "cronus.logLevel": "info"
}
```

**Settings:**

- `cronus.serverUrl` - Cronus server base URL (default: `http://localhost:9000`)
- `cronus.autoConnect` - Automatically connect on startup (default: `true`)
- `cronus.reconnectInterval` - Reconnection interval in milliseconds (default: `2000`)
- `cronus.logLevel` - Logging level: `debug`, `info`, `warning`, `error` (default: `info`)

## Features

### Connection Status

The debug panel shows:
- Current connection status (Connected/Disconnected)
- Server latency
- Reconnection attempts

### Log Viewer

View real-time logs from the Cronus server:
- Message logs
- Agent status updates
- System events
- Errors and warnings

### Auto-Reconnection

The extension automatically:
- Connects to the server on startup (if configured)
- Reconnects when the server becomes available
- Shows notifications on connection state changes

## Development

### Structure

```
clients/vscode/
├── src/
│   ├── extension.ts          # Extension entry point
│   └── panels/
│       └── CronusDebugPanel.ts # Webview panel
├── resources/
│   └── icon.svg              # Extension icon
├── package.json              # Extension manifest
└── tsconfig.json             # TypeScript config
```

### Building

```bash
# Compile TypeScript
npm run compile

# Watch mode (auto-recompile on changes)
npm run watch

# Package for distribution
npm run package
```

### Testing

Press `F5` in VSCode to launch the Extension Development Host with your extension loaded.

## Shared Code

This extension uses shared libraries from the Cronus monorepo:
- `@cronus/shared` - API client and core utilities

This ensures consistency across all Cronus clients (web, VSCode, CLI).

## Troubleshooting

### Extension Not Connecting

1. Check server URL in settings (`cronus.serverUrl`)
2. Ensure Cronus server is running on the specified port
3. Check server logs for connection errors
4. Try manually reconnecting with `Cronus: Reconnect to Server`

### Webview Not Loading

1. Reload the VSCode window (`Developer: Reload Window`)
2. Check the Output panel (`Cronus Debug` channel) for errors
3. Ensure the extension is properly compiled (`npm run compile`)

## License

MIT
