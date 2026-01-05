/**
 * Cronus Startup Notification Listener
 * 
 * This simple UDP server listens for startup notifications from the Cronus server
 * and broadcasts them to connected web clients via WebSocket.
 */

const dgram = require('dgram');
const http = require('http');
const WebSocket = require('ws');

const NOTIFICATION_PORT = process.env.NOTIFICATION_PORT || 8999;
const WS_PORT = process.env.WS_PORT || 8998;

// Create UDP server to receive notifications from Cronus server
const udpServer = dgram.createSocket('udp4');

udpServer.on('error', (err) => {
  console.error(`UDP server error:\n${err.stack}`);
  udpServer.close();
});

udpServer.on('message', (msg, rinfo) => {
  try {
    const notification = JSON.parse(msg.toString());
    console.log(`Received startup notification from ${rinfo.address}:${rinfo.port}`);
    console.log(`Server URL: ${notification.serverUrl}`);
    
    // Broadcast to all connected WebSocket clients
    wss.clients.forEach(client => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(JSON.stringify(notification));
      }
    });
  } catch (error) {
    console.error('Error parsing notification:', error);
  }
});

udpServer.on('listening', () => {
  const address = udpServer.address();
  console.log(`UDP notification listener active on ${address.address}:${address.port}`);
});

udpServer.bind(NOTIFICATION_PORT);

// Create HTTP server for WebSocket connections
const server = http.createServer((req, res) => {
  // CORS headers
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') {
    res.writeHead(200);
    res.end();
    return;
  }

  if (req.url === '/health') {
    // Health check endpoint
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ 
      status: 'ok', 
      udpPort: NOTIFICATION_PORT,
      wsPort: WS_PORT,
      clients: wss.clients.size 
    }));
  } else {
    res.writeHead(404);
    res.end('Not found');
  }
});

// Create WebSocket server
const wss = new WebSocket.Server({ server });

wss.on('connection', (ws) => {
  console.log(`WebSocket client connected. Total clients: ${wss.clients.size}`);

  // Send initial connection message
  ws.send(JSON.stringify({ type: 'connected' }));

  ws.on('close', () => {
    console.log(`WebSocket client disconnected. Total clients: ${wss.clients.size}`);
  });

  ws.on('error', (error) => {
    console.error('WebSocket error:', error);
  });
});

server.listen(WS_PORT, () => {
  console.log(`WebSocket server listening on port ${WS_PORT}`);
  console.log(`Connect to ws://localhost:${WS_PORT} for server startup events`);
});

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('\nShutting down notification listener...');
  udpServer.close();
  wss.close();
  server.close();
  process.exit(0);
});
