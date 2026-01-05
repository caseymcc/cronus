# Cronus Clients

Monorepo for all Cronus client applications and shared libraries.

## Structure

```
clients/
├── shared/          # @cronus/shared - Core API client and utilities
├── shared-ui/       # @cronus/shared-ui - React components and hooks
├── web/             # React web application (browser mode)
├── app/             # Electron wrapper for web client (standalone mode)
├── vscode/          # cronus-vscode - VSCode extension
└── cli/             # CLI client
```

## Shared Libraries

### @cronus/shared

Core TypeScript library providing:
- **CronusClient**: Unified API client with auto-reconnection
- **SSEManager**: Server-Sent Events manager
- **Type Definitions**: Shared data models
- **Logger**: Configurable logging utility

See [clients/shared/README.md](./shared/README.md) for details.

### @cronus/shared-ui

React UI library providing:
- **Hooks**: `useCronusConnection`, `useFileTree`, `useMessages`
- **Components**: `ConnectionStatus`, etc.
- Platform-agnostic React components

See [clients/shared-ui/README.md](./shared-ui/README.md) for details.

## Client Applications

### Web Client

React-based web application for Cronus interaction. Runs in a web browser and connects to a Cronus server.

**Development:**
```bash
cd web
npm start
```

**Build:**
```bash
cd web
npm run build
```

See [clients/web/README.md](./web/README.md) for details.

### Standalone App

Electron-based standalone application that wraps the web client. Allows running Cronus as a native desktop application without a browser.

**Build & Run:**
```bash
# Build everything (from clients/ directory)
./build-app.sh

# Or build manually
cd web && npm run build && cd ../app && npm run build

# Run the app
cd app
npm start       # Production mode
npm run dev     # Development mode with DevTools
```

The app automatically detects it's running in Electron and uses the preload API for server communication instead of direct HTTP/SSE.

See [clients/app/README.md](./app/README.md) for details.

### VSCode Extension

Debug and monitor Cronus from VS Code.

**Development:**
```bash
npm run dev:vscode  # Watch mode
# Press F5 in VSCode to debug
```

**Package:**
```bash
cd clients/vscode
npm run package
```

See [clients/vscode/README.md](./vscode/README.md) for details.

### CLI Client

Command-line interface for Cronus.

See [clients/cli/README.md](./cli/README.md) for details.

## Development

### Initial Setup

```bash
# Install all dependencies
npm install

# Build shared libraries
npm run build:shared
npm run build:shared-ui
```

### Development Workflow

**Working on Web Client (Browser Mode):**
```bash
# Terminal 1: Watch shared libraries
npm run watch:shared
npm run watch:shared-ui

# Terminal 2: Run web dev server
cd web
npm start
```

**Working on Standalone App:**
```bash
# Build web client first
cd web
npm run build

# Then build and run Electron wrapper
cd ../app
npm run dev
```

**Working on VSCode Extension:**
```bash
# Terminal 1: Watch shared library
npm run watch:shared

# Terminal 2: Watch extension
npm run dev:vscode

# Then press F5 in VSCode to debug
```

### Build Everything

```bash
npm run build
```

This will:
1. Build `@cronus/shared`
2. Build `@cronus/shared-ui`
3. Build all client applications

### Clean Build Artifacts

```bash
npm run clean
```

## Code Sharing

The monorepo maximizes code reuse across clients:

| Component | Web | VSCode | CLI | Shared % |
|-----------|-----|--------|-----|----------|
| API Client | ✓ | ✓ | ✓ | **100%** |
| SSE Manager | ✓ | ✓ | ✓ | **100%** |
| Data Models | ✓ | ✓ | ✓ | **100%** |
| React Hooks | ✓ | ✓ (webview) | ✗ | **95%** |
| UI Components | ✓ | ✓ (webview) | ✗ | **90%** |

**Overall: ~70-80% code sharing**

## NPM Workspaces

This project uses NPM workspaces for monorepo management. Benefits:

- **Single `node_modules`**: Shared dependencies across all packages
- **Workspace References**: Packages can depend on each other using `workspace:*`
- **Single `npm install`**: Install all dependencies at once
- **Workspace Commands**: Run scripts across all or specific workspaces

Example workspace commands:
```bash
# Run build in specific workspace
npm run build -w @cronus/shared

# Run test in all workspaces that have it
npm test --workspaces --if-present

# Install package in specific workspace
npm install lodash -w @cronus/web
```

## TypeScript

All packages use TypeScript with consistent configuration:
- Strict mode enabled
- ES2020 target
- CommonJS modules for Node.js compatibility
- Source maps for debugging

## Troubleshooting

### Type Errors in Shared Libraries

After modifying shared libraries, rebuild them:
```bash
npm run build:shared
npm run build:shared-ui
```

Or use watch mode during development:
```bash
npm run watch:shared
npm run watch:shared-ui
```

### Dependency Issues

Clear and reinstall:
```bash
rm -rf node_modules
rm -rf clients/*/node_modules
rm -rf clients/*/dist
npm install
npm run build
```

### VSCode Extension Not Loading

1. Rebuild shared library: `npm run build:shared`
2. Compile extension: `npm run build:vscode`
3. Reload VSCode window

## Contributing

When adding new features:

1. **Core Logic**: Add to `@cronus/shared`
2. **UI Logic**: Add to `@cronus/shared-ui` (if React)
3. **Client-Specific**: Add to individual client packages

This ensures maximum code reuse across all clients.

## License

MIT
