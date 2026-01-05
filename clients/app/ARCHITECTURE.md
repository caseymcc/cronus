# Cronus Standalone App Architecture

This document explains how the standalone Electron app works and how it integrates with the web client.

## Overview

The Cronus standalone app is an Electron wrapper around the web client that allows the web application to run as a native desktop application without requiring a web browser or separate server hosting.

## Architecture

```
┌─────────────────────────────────────┐
│   Electron Main Process             │
│   (src/main.ts)                     │
│                                     │
│   - Window management               │
│   - CronusClient instance           │
│   - IPC handlers                    │
└──────────────┬──────────────────────┘
               │ IPC Communication
               │
┌──────────────▼──────────────────────┐
│   Preload Script                    │
│   (src/preload.ts)                  │
│                                     │
│   - Exposes window.cronusAPI        │
│   - Secure IPC bridge               │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│   Web Client (Renderer Process)     │
│   (renderer/ - from clients/web)    │
│                                     │
│   - React application               │
│   - Detects Electron environment    │
│   - Uses cronusAPI when available   │
└─────────────────────────────────────┘
```

## Dual-Mode Support

The web client (`clients/web`) now supports two modes:

### Browser Mode
- Runs in a standard web browser
- Connects directly to Cronus server via HTTP/SSE
- Uses `fetch()` and `EventSource` APIs
- Requires separate server instance

### Electron Mode (Standalone)
- Runs in Electron desktop app
- Uses `window.cronusAPI` provided by preload script
- Main process manages server connection
- Events forwarded via IPC

## How It Works

### 1. Environment Detection

The web client detects the environment using `utils/electronBridge.js`:

```javascript
export const isElectronEnvironment = () => {
  return typeof window !== 'undefined' && window.cronusAPI !== undefined;
};
```

### 2. Connection Management

**In Browser Mode:**
- Web client creates SSE connection to server
- Direct HTTP requests to API endpoints

**In Electron Mode:**
- Main process creates `CronusClient` instance
- Connects to server and manages reconnection
- Forwards events to renderer via IPC

### 3. IPC Communication

The preload script exposes a secure API:

```typescript
window.cronusAPI = {
  connect: () => Promise<Result>,
  disconnect: () => Promise<Result>,
  getConnectionStatus: () => Promise<Status>,
  setServerUrl: (url: string) => Promise<Result>,
  getLogs: (limit?, levelFilter?) => Promise<Logs>,
  isElectron: () => Promise<boolean>,
  onConnected: (callback) => unsubscribe,
  onDisconnected: (callback) => unsubscribe,
  onError: (callback) => unsubscribe,
  onLog: (callback) => unsubscribe
}
```

### 4. Event Flow

**Connection Events:**
```
Server → CronusClient → Main Process → IPC → Preload → Web Client
```

**Log Events:**
```
Server → CronusClient → Main Process → IPC → Preload → Web Client → UI
```

**User Actions:**
```
UI → Web Client → cronusAPI → IPC → Main Process → CronusClient → Server
```

## Build Process

1. **Build Web Client**
   ```bash
   cd clients/web
   npm run build
   ```
   Creates production build in `clients/web/build/`

2. **Copy Web Build**
   ```bash
   cd clients/app
   npm run copy-web-build
   ```
   Copies web build to `clients/app/renderer/`

3. **Build Electron App**
   ```bash
   cd clients/app
   npm run build:electron-only
   ```
   Compiles TypeScript in `src/` to `dist/`

4. **Run App**
   ```bash
   npm start
   ```
   Loads `dist/main.js` and `renderer/index.html`

Or use the all-in-one script:
```bash
cd clients
./build-app.sh
```

## Files

### Electron App (`clients/app/`)
- `src/main.ts` - Main Electron process, window management, IPC handlers
- `src/preload.ts` - Secure bridge between main and renderer
- `package.json` - App dependencies and build scripts
- `renderer/` - Copied web client build (generated)
- `dist/` - Compiled TypeScript (generated)

### Web Client (`clients/web/`)
- `src/App.js` - Main React component with environment detection
- `src/utils/electronBridge.js` - Environment detection and API abstraction
- All other React components work in both modes

## Development

### Web Client Changes
When working on the web client:
1. Test in browser mode first: `cd clients/web && npm start`
2. Build and test in Electron: `cd clients/app && npm run build && npm run dev`

### Electron Wrapper Changes
When modifying the Electron wrapper:
1. Edit `src/main.ts` or `src/preload.ts`
2. Run `npm run build:electron-only` (or just `npm run build`)
3. Test with `npm run dev`

## Benefits

1. **Code Reuse**: Single web client codebase for both browser and desktop
2. **Feature Parity**: Same features in both modes
3. **Standalone**: Desktop app works independently of browser
4. **Auto-reconnection**: Built into Electron wrapper
5. **Native Feel**: Desktop app with native window management

## Future Enhancements

Potential additions:
- System tray integration
- Native notifications
- File system access for local projects
- Multiple window support
- Auto-updates via electron-updater
- Platform-specific features (menu bar, dock, etc.)
