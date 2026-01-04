# Cronus Standalone Debug Client

A standalone Electron-based debug client for monitoring Cronus servers.

## Features

- **Auto-reconnection**: Automatically connects when server becomes available
- **Real-time log streaming**: Live log updates via Server-Sent Events (SSE)
- **Log filtering**: Filter by level (Debug, Info, Warning, Error) and search text
- **Standalone application**: No VSCode required
- **Configurable server URL**: Connect to any Cronus server instance
- **Professional UI**: Dark theme with color-coded log levels

## Building

From the `clients` directory:

```bash
./build-debug-client.sh
```

Or manually:

```bash
cd debug
npm install
npm run build
```

## Running

### Development Mode (with DevTools)

```bash
npm run dev
```

### Production Mode

```bash
npm start
```

## Configuration

The default server URL is `http://localhost:9000`. You can change it in the UI or set the `CRONUS_SERVER_URL` environment variable:

```bash
CRONUS_SERVER_URL=http://192.168.1.100:9000 npm start
```

## Packaging

To create distributable packages:

```bash
npm run package
```

This will create platform-specific installers in the `dist-electron` directory.

## Architecture

- **Main Process** (`src/main.ts`): Electron main process, manages window and Cronus client
- **Preload Script** (`src/preload.ts`): Secure bridge between main and renderer processes
- **Renderer** (`renderer/`): HTML/CSS/JS UI for displaying logs
- **Shared Libraries**: Uses `@cronus/shared` and `@cronus/shared-ui` from workspace

## API

The debug client exposes a secure API to the renderer via `window.cronusAPI`:

- `connect()`: Connect to Cronus server
- `disconnect()`: Disconnect from server
- `getConnectionStatus()`: Get current connection state
- `setServerUrl(url)`: Change server URL
- `getLogs(limit?, levelFilter?)`: Fetch historical logs
- Event listeners: `onConnected()`, `onDisconnected()`, `onError()`, `onLog()`

## Keyboard Shortcuts

- `Ctrl+Shift+I`: Toggle DevTools (development mode only)
- `Ctrl+F`: Focus search input
- `Ctrl+L`: Clear logs

## License

MIT
