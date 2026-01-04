# Cronus Client Setup - Quick Start Guide

**Last Updated:** January 04, 2026

## Quick Setup (5 minutes)

### 1. Install Dependencies

```bash
cd /home/caseymcc/projects/cronus/clients
npm install
```

This installs all dependencies for all packages using NPM workspaces.

### 2. Build Shared Libraries

```bash
npm run build:shared
npm run build:shared-ui
```

This compiles the TypeScript shared libraries that all clients depend on.

### 3. Choose Your Client

#### Option A: Web Client

```bash
npm run dev:web
```

Open browser to: http://localhost:3000

#### Option B: VSCode Extension

```bash
cd vscode
npm run compile
```

Then in VSCode:
1. Open the `clients/vscode` folder
2. Press `F5` to debug
3. In the Extension Development Host window, run: `Cronus: Open Debug Panel`

#### Option C: CLI Client

```bash
cd cli
npm start
```

## Development Workflow

### For Web Client Development

```bash
# Terminal 1: Watch shared library
npm run watch:shared

# Terminal 2: Watch shared UI
npm run watch:shared-ui

# Terminal 3: Run web dev server
npm run dev:web
```

### For VSCode Extension Development

```bash
# Terminal 1: Watch shared library
npm run watch:shared

# Terminal 2: Watch extension
cd vscode && npm run watch

# Press F5 in VSCode to debug
```

## Verify Installation

### Check Shared Library

```bash
cd clients/shared
ls dist/
# Should see: index.js, index.d.ts, api/, models/, utils/
```

### Check Shared UI

```bash
cd clients/shared-ui
ls dist/
# Should see: index.js, index.d.ts, hooks/, components/
```

### Test Connection

1. Start Cronus server:
   ```bash
   cd /home/caseymcc/projects/cronus
   ./run_local.sh ninja -C build/linux_x64_debug
   ./run_local.sh ./build/linux_x64_debug/server/cronus/cronus
   ```

2. Start web client:
   ```bash
   cd clients
   npm run dev:web
   ```

3. Open http://localhost:3000
4. Check connection status indicator

## Troubleshooting

### "Cannot find module '@cronus/shared'"

```bash
cd clients
npm run build:shared
npm run build:shared-ui
```

### "npm install" fails

```bash
# Clean everything
rm -rf node_modules
rm -rf */node_modules

# Reinstall
npm install
```

### TypeScript errors in VSCode

```bash
# Reload VSCode window
Ctrl+Shift+P (or Cmd+Shift+P)
> Developer: Reload Window
```

## Next Steps

1. Read [Client Development Guide](./client-development.md)
2. Explore [Architecture Documentation](./architecture.md)
3. Check [API Documentation](./api-documentation.md)
4. Review example code in `clients/web/src/App.js`

## File Structure Overview

```
clients/
├── package.json              # Root with workspaces
├── shared/                   # Core API (@cronus/shared)
│   ├── src/
│   │   ├── api/              # CronusClient, SSEManager
│   │   ├── models/           # TypeScript types
│   │   └── utils/            # Logger, helpers
│   └── dist/                 # Compiled JS/TS
├── shared-ui/                # React library (@cronus/shared-ui)
│   ├── src/
│   │   ├── hooks/            # React hooks
│   │   └── components/       # React components
│   └── dist/                 # Compiled JS/TS
├── web/                      # Web client
├── vscode/                   # VSCode extension
└── cli/                      # CLI client
```

## Common Commands

```bash
# Build everything
npm run build

# Build specific package
npm run build:shared
npm run build:shared-ui
npm run build:web

# Development mode
npm run dev:web
npm run dev:vscode

# Watch mode (auto-rebuild)
npm run watch:shared
npm run watch:shared-ui

# Clean build artifacts
npm run clean

# Run tests (if available)
npm test
```

## Configuration

### Server URL

Edit `clients/web/package.json`:
```json
{
  "proxy": "http://localhost:9000"
}
```

Or for VSCode extension, edit settings:
```json
{
  "cronus.serverUrl": "http://localhost:9000"
}
```

### Port Configuration

Web client uses port 3000 by default. To change:
```bash
PORT=3001 npm run dev:web
```

## Support

- Documentation: `docs/client-development.md`
- Architecture: `docs/architecture.md`
- Issues: Check GitHub issues or create new one
- Shared Library README: `clients/shared/README.md`
- Shared UI README: `clients/shared-ui/README.md`
