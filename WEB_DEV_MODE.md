# Web Client Development Mode

## Quick Start

Start the web client in development mode with hot-reloading:

```bash
./run_local.sh -w
```

This will:
1. Start the Docker container if not already running
2. Install npm dependencies if needed (first run only)
3. Start the React development server on port 3000
4. Auto-reload on file changes

## Access the Application

- **Web UI**: http://localhost:3000
- **API Backend**: http://localhost:9000 (must be running separately)

## Requirements

The Cronus backend server must be running on port 9000 for the web client to function. Start it in a separate terminal:

```bash
# Terminal 1: Start Cronus backend
./run_local.sh ./build/linux_x64_debug/server/cronus/cronus --web --port 9000

# Terminal 2: Start web dev server
./run_local.sh -w
```

## How It Works

- The React dev server runs on port **3000** inside Docker and forwards to your host
- API requests are proxied to port **9000** (configured in `package.json`)
- WebSocket connections go to `ws://localhost:9000/ws`
- Hot module replacement (HMR) enables instant updates without refresh

## Production Build

To build the production version (served directly by Cronus backend):

```bash
cd clients/web
npm run build
# Output: clients/web/build/

# Or use CMake:
./run_local.sh ninja -C build/linux_x64_debug web-ui
```

## Troubleshooting

**Port 3000 already in use:**
```bash
# Stop any existing processes on port 3000
lsof -ti:3000 | xargs kill -9

# Or stop the Docker container
./run_local.sh -s
```

**Cannot connect to backend:**
- Ensure Cronus server is running: `curl http://localhost:9000/api/health`
- Check proxy setting in `clients/web/package.json` points to correct port
- Verify port forwarding in Docker: `docker ps` should show `0.0.0.0:9000->9000/tcp`

**Changes not reflecting:**
- The dev server has auto-reload, but you may need to clear browser cache
- Check the terminal for compilation errors
- Ensure you're editing files in the correct location (not inside Docker)
