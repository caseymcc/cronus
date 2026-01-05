# Cronus Standalone App

A standalone Electron-based application that wraps the Cronus web client, allowing it to run outside of a browser or server environment.

## Features

- **Standalone Desktop App**: Run Cronus as a native desktop application
- **Auto-reconnection**: Automatically connects when server becomes available
- **Full Web Client Features**: All features from the web client, including:
  - Chat interface with AI assistant
  - File explorer and editor
  - Real-time log streaming
  - Multiple tabs and layouts
- **Professional UI**: Dark theme with modern interface
- **Cross-platform**: Built with Electron for Linux, Windows, and macOS

## Architecture

This app is an Electron wrapper around the web client:
- **Main Process** (`src/main.ts`): Electron window management and IPC handlers
- **Preload Script** (`src/preload.ts`): Secure bridge between Electron and web client
- **Renderer**: The compiled web client (from `clients/web`)

The web client detects when running in Electron and uses the preload API instead of direct HTTP/SSE connections.

## Building

From the `clients` directory:

```bash
./build-app.sh
```

Or manually:

```bash
# Build web client first
cd web
npm install
npm run build
cd ../app

# Build Electron wrapper
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

The default server URL is `http://localhost:9000`. You can change it by setting the `CRONUS_SERVER_URL` environment variable:

```bash
CRONUS_SERVER_URL=http://192.168.1.100:9000 npm start
```

## Package Structure

- `src/main.ts` - Main Electron process
- `src/preload.ts` - Preload script for secure IPC
- `renderer/` - Built web client (copied during build)
- `dist/` - Compiled TypeScript
- `dist-electron/` - Packaged application

## Development Notes

- The app automatically rebuilds the web client during build
- The preload API is exposed to the web client via `window.cronusAPI`
- Connection management is handled by the Electron main process
- Logs and events are forwarded from the main process to the renderer


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
