# Cronus Client Development Guide

**Last Updated:** January 04, 2026

## Overview

This guide covers the development of Cronus client applications using the shared library architecture. The client ecosystem is organized as a monorepo with maximum code reuse across platforms.

## Architecture Philosophy

### Code Sharing Strategy

The Cronus client architecture follows these principles:

1. **Core Logic in Shared Libraries**: API clients, data models, and business logic live in `@cronus/shared`
2. **UI Logic in Shared UI**: React components and hooks live in `@cronus/shared-ui`
3. **Platform-Specific in Clients**: Only platform integration code lives in individual clients
4. **Type Safety Everywhere**: TypeScript across the entire stack

### Dependency Flow

```
@cronus/shared (no dependencies except utilities)
    ↓
@cronus/shared-ui (depends on @cronus/shared + React)
    ↓
Client Applications (depend on both shared libraries)
```

## Getting Started

### Prerequisites

- Node.js 18+
- npm 9+
- TypeScript 5+

### Initial Setup

```bash
# Navigate to clients directory
cd clients

# Install all dependencies (uses NPM workspaces)
npm install

# Build shared libraries
npm run build:shared
npm run build:shared-ui

# Start development
npm run dev:web  # or dev:vscode
```

## Monorepo Structure

```
clients/
├── package.json              # Root package.json with workspaces
├── README.md                 # This file
├── shared/                   # @cronus/shared
│   ├── package.json
│   ├── tsconfig.json
│   ├── src/
│   │   ├── api/              # API clients
│   │   │   ├── CronusClient.ts
│   │   │   └── SSEManager.ts
│   │   ├── models/           # Data types
│   │   │   └── types.ts
│   │   ├── utils/            # Utilities
│   │   │   └── logger.ts
│   │   └── index.ts          # Public API
│   └── dist/                 # Compiled output
├── shared-ui/                # @cronus/shared-ui
│   ├── package.json
│   ├── tsconfig.json
│   ├── src/
│   │   ├── hooks/            # React hooks
│   │   │   ├── useCronusConnection.ts
│   │   │   ├── useFileTree.ts
│   │   │   └── useMessages.ts
│   │   ├── components/       # React components
│   │   │   └── ConnectionStatus/
│   │   └── index.ts          # Public API
│   └── dist/                 # Compiled output
├── web/                      # @cronus/web
│   ├── package.json
│   ├── src/
│   └── build/
├── vscode/                   # cronus-vscode
│   ├── package.json
│   ├── src/
│   └── dist/
└── cli/                      # CLI client
    ├── package.json
    └── src/
```

## Developing with Shared Libraries

### Adding a New Feature

**1. Determine where the feature belongs:**

- **@cronus/shared**: API calls, data models, utilities, business logic
- **@cronus/shared-ui**: React hooks, UI components
- **Client app**: Platform-specific integration, layout

**2. Example: Adding a new API endpoint**

```typescript
// clients/shared/src/api/CronusClient.ts

export class CronusClient extends EventEmitter {
    // ... existing code ...

    /**
     * Fetch agent statistics
     */
    async fetchAgentStats(): Promise<AgentStats> {
        const response = await fetch(`${this.config.baseUrl}/api/stats`);
        if (!response.ok) {
            throw new Error(`Failed to fetch stats: ${response.statusText}`);
        }
        return await response.json();
    }
}
```

```typescript
// clients/shared/src/models/types.ts

export interface AgentStats {
    messagesProcessed: number;
    filesModified: number;
    uptime: number;
    averageResponseTime: number;
}
```

**3. Create a React hook (if needed)**

```typescript
// clients/shared-ui/src/hooks/useAgentStats.ts

import { useState, useEffect } from 'react';
import { CronusClient, AgentStats } from '@cronus/shared';

export function useAgentStats(client: CronusClient | null) {
    const [stats, setStats] = useState<AgentStats | null>(null);
    const [loading, setLoading] = useState(false);

    const fetchStats = async () => {
        if (!client) return;
        
        setLoading(true);
        try {
            const data = await client.fetchAgentStats();
            setStats(data);
        } catch (err) {
            console.error('Failed to fetch stats:', err);
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        fetchStats();
        const interval = setInterval(fetchStats, 30000); // Refresh every 30s
        return () => clearInterval(interval);
    }, [client]);

    return { stats, loading, refresh: fetchStats };
}
```

**4. Use in client applications**

```typescript
// clients/web/src/components/StatsPanel.tsx

import { useAgentStats, useCronusConnection } from '@cronus/shared-ui';

export function StatsPanel() {
    const { client } = useCronusConnection({ baseUrl: 'http://localhost:9000' });
    const { stats, loading } = useAgentStats(client);

    if (loading) return <div>Loading...</div>;
    if (!stats) return null;

    return (
        <div>
            <h3>Agent Statistics</h3>
            <p>Messages: {stats.messagesProcessed}</p>
            <p>Files Modified: {stats.filesModified}</p>
            <p>Uptime: {stats.uptime}s</p>
        </div>
    );
}
```

### Build Process

**Development (Watch Mode):**

```bash
# Terminal 1: Watch shared library
cd clients
npm run watch:shared

# Terminal 2: Watch shared-ui library
npm run watch:shared-ui

# Terminal 3: Run web client in dev mode
npm run dev:web
```

**Production Build:**

```bash
# Build everything
npm run build

# Or build individually
npm run build:shared
npm run build:shared-ui
npm run build:web
npm run build:vscode
```

### TypeScript Configuration

Each package has its own `tsconfig.json`:

**shared/tsconfig.json:**
```json
{
  "compilerOptions": {
    "target": "ES2020",
    "module": "commonjs",
    "lib": ["ES2020"],
    "declaration": true,
    "outDir": "./dist",
    "rootDir": "./src",
    "strict": true
  }
}
```

**shared-ui/tsconfig.json:**
```json
{
  "compilerOptions": {
    "target": "ES2020",
    "module": "commonjs",
    "lib": ["ES2020", "DOM"],
    "jsx": "react",
    "declaration": true,
    "outDir": "./dist",
    "rootDir": "./src",
    "strict": true
  }
}
```

## VSCode Extension Development

### Extension Structure

```
clients/vscode/
├── src/
│   ├── extension.ts          # Extension entry point
│   └── panels/
│       └── CronusDebugPanel.ts # Webview panel
├── resources/
│   └── icon.svg              # Extension icon
├── package.json              # Extension manifest
└── tsconfig.json
```

### Development Workflow

1. **Make changes to extension code**
2. **Rebuild shared library if needed:**
   ```bash
   npm run build:shared
   ```
3. **Compile extension:**
   ```bash
   cd clients/vscode
   npm run compile
   ```
4. **Debug in VSCode:**
   - Open `clients/vscode` folder in VSCode
   - Press `F5` to launch Extension Development Host
   - Test extension in the new window

### Webview Development

The VSCode extension uses a webview for its UI. The webview can use React components from `@cronus/shared-ui`:

```typescript
// In extension.ts
import { CronusClient } from '@cronus/shared';

const client = new CronusClient({ baseUrl: 'http://localhost:9000' });
client.start();

// Forward events to webview
client.on('message', (data) => {
    webviewPanel.postMessage({ type: 'message', data });
});
```

```html
<!-- In webview HTML -->
<script>
    const vscode = acquireVsCodeApi();
    
    window.addEventListener('message', event => {
        const message = event.data;
        if (message.type === 'message') {
            updateUI(message.data);
        }
    });
</script>
```

## Testing

### Unit Tests

Test shared libraries independently:

```typescript
// clients/shared/src/__tests__/CronusClient.test.ts

import { CronusClient } from '../api/CronusClient';

describe('CronusClient', () => {
    it('should connect to server', async () => {
        const client = new CronusClient({ baseUrl: 'http://localhost:9000' });
        // ... test implementation
    });
});
```

### Integration Tests

Test client applications with shared libraries:

```typescript
// clients/web/src/__tests__/App.test.tsx

import { render } from '@testing-library/react';
import App from '../App';

test('renders app with connection status', () => {
    const { getByText } = render(<App />);
    expect(getByText(/status/i)).toBeInTheDocument();
});
```

## Common Tasks

### Adding a New Shared Component

1. **Create component:**
   ```typescript
   // clients/shared-ui/src/components/MyComponent/MyComponent.tsx
   import React from 'react';
   
   export interface MyComponentProps {
       data: string;
   }
   
   export const MyComponent: React.FC<MyComponentProps> = ({ data }) => {
       return <div>{data}</div>;
   };
   ```

2. **Create styles:**
   ```css
   /* clients/shared-ui/src/components/MyComponent/MyComponent.css */
   .my-component {
       color: var(--vscode-foreground);
   }
   ```

3. **Export from index:**
   ```typescript
   // clients/shared-ui/src/components/MyComponent/index.ts
   export { MyComponent } from './MyComponent';
   export type { MyComponentProps } from './MyComponent';
   ```

4. **Add to package exports:**
   ```typescript
   // clients/shared-ui/src/index.ts
   export { MyComponent } from './components/MyComponent';
   export type { MyComponentProps } from './components/MyComponent';
   ```

5. **Rebuild:**
   ```bash
   npm run build:shared-ui
   ```

6. **Use in clients:**
   ```typescript
   import { MyComponent } from '@cronus/shared-ui';
   
   function App() {
       return <MyComponent data="Hello" />;
   }
   ```

### Updating Dependencies

**Update shared library dependency:**
```bash
cd clients/shared
npm install lodash
npm install --save-dev @types/lodash
```

**Update client dependency:**
```bash
cd clients/web
npm install react-router-dom
```

**Update all dependencies:**
```bash
cd clients
npm update --workspaces
```

## Troubleshooting

### Type Errors After Updating Shared Library

**Problem:** Client shows type errors after updating `@cronus/shared`

**Solution:**
```bash
# Rebuild shared libraries
npm run build:shared
npm run build:shared-ui

# If still having issues, clean and rebuild
npm run clean
npm install
npm run build
```

### VSCode Extension Not Loading Shared Library

**Problem:** Extension can't find `@cronus/shared`

**Solution:**
```bash
# Ensure shared library is built
cd clients
npm run build:shared

# Recompile extension
cd clients/vscode
npm run compile

# Reload VSCode window
```

### Webview Not Updating

**Problem:** Changes to shared-ui components don't appear in webview

**Solution:**
```bash
# Rebuild shared-ui
npm run build:shared-ui

# For development, use watch mode
npm run watch:shared-ui

# Reload webview in VSCode
```

### NPM Workspace Issues

**Problem:** `npm install` fails or packages not found

**Solution:**
```bash
# Clean everything
rm -rf node_modules
rm -rf clients/*/node_modules
rm -rf clients/*/dist

# Reinstall from root
cd clients
npm install

# Rebuild shared libraries
npm run build:shared
npm run build:shared-ui
```

## Best Practices

1. **Always Build Shared Libraries First**: Before working on clients, ensure shared libraries are built
2. **Use Watch Mode in Development**: Run `npm run watch:shared` and `npm run watch:shared-ui` during active development
3. **Type Everything**: Leverage TypeScript's type system for better developer experience
4. **Export Public API Only**: Use `index.ts` files to control what's exported from shared libraries
5. **Platform-Agnostic Shared Code**: Keep platform-specific code out of shared libraries
6. **Test Shared Code Thoroughly**: Bugs in shared code affect all clients
7. **Version Consistently**: Keep shared library versions in sync

## References

- [NPM Workspaces Documentation](https://docs.npmjs.com/cli/v9/using-npm/workspaces)
- [TypeScript Handbook](https://www.typescriptlang.org/docs/)
- [VSCode Extension API](https://code.visualstudio.com/api)
- [React Documentation](https://react.dev/)
