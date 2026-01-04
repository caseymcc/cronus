# Docker Build Fixes Summary

## Issues Found and Fixed

### 1. Missing Dependencies
**Problem**: `@cronus/shared` was missing `eventemitter3` and `@types/node` dependencies.
**Solution**: 
- Added `eventemitter3` to dependencies
- Added `@types/node` to devDependencies
- Ran `npm install` in the shared directory

### 2. TypeScript Configuration Issues
**Problem**: TypeScript couldn't find `fetch`, `EventSource`, `console`, `NodeJS` namespace.
**Solution**: Updated `clients/shared/tsconfig.json`:
```json
{
  "lib": ["ES2020", "DOM"],  // Added DOM for browser APIs
  "types": ["node"]           // Added node types
}
```

### 3. Workspace Protocol Not Supported
**Problem**: npm in Docker didn't support `workspace:*` protocol in package.json dependencies.
**Solution**: Changed all `workspace:*` references to `file:` protocol:
- `clients/shared-ui/package.json`: `"@cronus/shared": "file:../shared"`
- `clients/debug/package.json`: `"@cronus/shared": "file:../shared"`, `"@cronus/shared-ui": "file:../shared-ui"`
- `clients/vscode/package.json`: `"@cronus/shared": "file:../shared"`
- Removed `workspaces` field from `clients/package.json`

### 4. Build Script Dependencies
**Problem**: Build script needed to handle dependencies between packages.
**Solution**: Updated `clients/build-debug-client.sh` to:
1. Install `shared` dependencies first
2. Build `@cronus/shared`
3. Install `shared-ui` dependencies (can now reference built `shared`)
4. Build `@cronus/shared-ui`  
5. Install `debug` dependencies (can now reference both built libraries)
6. Build debug client

### 5. CronusClient API Mismatch
**Problem**: Electron main.ts was using non-existent methods:
- Constructor signature wrong (expected single config object, not two params)
- Methods `connect()`, `disconnect()`, `getServerUrl()` don't exist
- Should use `start()`, `stop()`, `isConnected()`

**Solution**: Updated `clients/debug/src/main.ts`:
```typescript
// Before:
cronusClient = new CronusClient(serverUrl, { autoConnect: true });
cronusClient.connect();
cronusClient.disconnect();

// After:
cronusClient = new CronusClient({ baseUrl: serverUrl, reconnectInterval: 2000 });
cronusClient.start();
cronusClient.stop();
```

### 6. Preload Script Type Errors
**Problem**: Event listener callbacks had incorrect type signatures.
**Solution**: Created proper listener functions with typed parameters:
```typescript
const listener = (_event: any, error: string) => callback(error);
ipcRenderer.on('cronus:error', listener);
return () => ipcRenderer.removeListener('cronus:error', listener);
```

## Final Build Script Flow

```bash
#!/bin/bash
cd clients

# Step 1: Install shared dependencies
cd shared && npm install && cd ..

# Step 2: Build shared (others depend on this)
cd shared && npm run build && cd ..

# Step 3: Install shared-ui dependencies
cd shared-ui && npm install --legacy-peer-deps && cd ..

# Step 4: Build shared-ui  
cd shared-ui && npm run build && cd ..

# Step 5: Install debug client dependencies
cd debug && npm install && cd ..

# Step 6: Build debug client
cd debug && npm run build && cd ..
```

## Verification

Build now completes successfully in Docker:
```bash
./run_local.sh ./clients/build-debug-client.sh
```

Output:
```
✅ Step 1: Installing dependencies... (5 packages)
✅ Step 2: Building @cronus/shared... (TypeScript compilation successful)
✅ Step 3: Installing shared-ui dependencies... (10 packages)
✅ Step 4: Building @cronus/shared-ui... (TypeScript compilation successful)
✅ Step 5: Installing debug client dependencies... (317 packages)
✅ Step 6: Building debug client... (TypeScript compilation successful)
```

## Files Modified

1. `clients/shared/tsconfig.json` - Added DOM lib and node types
2. `clients/shared/package.json` - Already had correct dependencies
3. `clients/shared-ui/package.json` - Changed workspace:* to file:../shared
4. `clients/debug/package.json` - Changed workspace:* to file: protocol
5. `clients/vscode/package.json` - Changed workspace:* to file:../shared
6. `clients/package.json` - Removed workspaces field, updated scripts
7. `clients/build-debug-client.sh` - Complete rewrite for file: protocol approach
8. `clients/debug/src/main.ts` - Fixed CronusClient API usage
9. `clients/debug/src/preload.ts` - Fixed event listener types

## Next Steps

The standalone debug client is now ready to test:

```bash
# In one terminal: Start Cronus server
./run_local.sh ./build/linux_x64_debug/server/cronus/cronus

# In another terminal: Start debug client
cd clients/debug
npm start
```

Note: Electron requires a display server (X11), so running in Docker may require additional X forwarding setup. For development, run the client directly on the host machine after building in Docker.
