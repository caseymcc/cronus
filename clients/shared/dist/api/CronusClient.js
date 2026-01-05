"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.CronusClient = void 0;
const eventemitter3_1 = __importDefault(require("eventemitter3"));
class CronusClient extends eventemitter3_1.default {
    constructor(config) {
        super();
        this.ws = null;
        this.healthCheckTimer = null;
        this.connectionStatus = { connected: false };
        this.requestId = 0;
        this.pendingRequests = new Map();
        this.reconnectTimer = null;
        this.reconnectAttempts = 0;
        this.config = {
            baseUrl: config.baseUrl,
            reconnectInterval: config.reconnectInterval ?? 2000,
            maxReconnectAttempts: config.maxReconnectAttempts ?? 0,
            healthCheckInterval: config.healthCheckInterval ?? 30000,
        };
    }
    /**
     * Start the client (connect WebSocket and start health checks)
     */
    start() {
        this.connectWebSocket();
        this.startHealthCheck();
    }
    /**
     * Stop the client
     */
    stop() {
        this.disconnectWebSocket();
        this.stopHealthCheck();
    }
    /**
     * Check if connected to the server
     */
    isConnected() {
        return this.connectionStatus.connected && this.ws?.readyState === WebSocket.OPEN;
    }
    /**
     * Get current connection status
     */
    getConnectionStatus() {
        return { ...this.connectionStatus };
    }
    /**
     * Connect to WebSocket
     */
    connectWebSocket() {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            return;
        }
        // Convert HTTP(S) URL to WS(S) URL
        const wsUrl = this.config.baseUrl.replace(/^http/, 'ws') + '/ws';
        try {
            this.ws = new WebSocket(wsUrl);
            this.ws.onopen = () => {
                this.reconnectAttempts = 0;
                this.updateConnectionStatus(true);
                this.emit('connected');
                if (this.reconnectTimer) {
                    clearTimeout(this.reconnectTimer);
                    this.reconnectTimer = null;
                }
            };
            this.ws.onclose = () => {
                this.updateConnectionStatus(false);
                this.emit('disconnected');
                this.scheduleReconnect();
            };
            this.ws.onerror = (error) => {
                this.emit('error', new Error('WebSocket error'));
            };
            this.ws.onmessage = (event) => {
                this.handleMessage(event.data);
            };
        }
        catch (error) {
            this.emit('error', error);
            this.scheduleReconnect();
        }
    }
    /**
     * Disconnect WebSocket
     */
    disconnectWebSocket() {
        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }
        if (this.ws) {
            this.ws.close();
            this.ws = null;
        }
        // Reject all pending requests
        for (const [id, { reject }] of this.pendingRequests) {
            reject(new Error('Connection closed'));
        }
        this.pendingRequests.clear();
    }
    /**
     * Schedule reconnection attempt
     */
    scheduleReconnect() {
        if (this.reconnectTimer) {
            return;
        }
        const maxAttempts = this.config.maxReconnectAttempts;
        if (maxAttempts > 0 && this.reconnectAttempts >= maxAttempts) {
            this.emit('reconnect-failed', { attempts: this.reconnectAttempts });
            return;
        }
        this.reconnectAttempts++;
        this.emit('reconnecting', { attempt: this.reconnectAttempts, maxAttempts });
        this.reconnectTimer = setTimeout(() => {
            this.reconnectTimer = null;
            this.connectWebSocket();
        }, this.config.reconnectInterval);
    }
    /**
     * Handle incoming WebSocket message
     */
    handleMessage(data) {
        try {
            const message = JSON.parse(data);
            // Handle JSON-RPC response
            if ('id' in message && message.id !== undefined) {
                const pending = this.pendingRequests.get(message.id);
                if (pending) {
                    this.pendingRequests.delete(message.id);
                    if (message.error) {
                        pending.reject(new Error(message.error.message));
                    }
                    else {
                        pending.resolve(message.result);
                    }
                }
            }
            // Handle JSON-RPC notification
            else if ('method' in message) {
                this.handleNotification(message);
            }
        }
        catch (error) {
            this.emit('error', error);
        }
    }
    /**
     * Handle JSON-RPC notification
     */
    handleNotification(notification) {
        const { method, params } = notification;
        switch (method) {
            case 'message':
                this.emit('message', params);
                break;
            case 'log':
                this.emit('log', params);
                break;
            case 'directory_update':
                this.emit('directory-update', params);
                break;
            case 'agent_status':
                this.emit('agent-status', params);
                break;
            default:
                this.emit('notification', { method, params });
                break;
        }
    }
    /**
     * Send JSON-RPC request and wait for response
     */
    async sendRequest(method, params) {
        if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
            throw new Error('WebSocket not connected');
        }
        const id = ++this.requestId;
        const request = {
            jsonrpc: '2.0',
            method,
            params,
            id,
        };
        return new Promise((resolve, reject) => {
            this.pendingRequests.set(id, { resolve, reject });
            try {
                this.ws.send(JSON.stringify(request));
            }
            catch (error) {
                this.pendingRequests.delete(id);
                reject(error);
            }
            // Set timeout for request
            setTimeout(() => {
                if (this.pendingRequests.has(id)) {
                    this.pendingRequests.delete(id);
                    reject(new Error('Request timeout'));
                }
            }, 30000); // 30 second timeout
        });
    }
    /**
     * Fetch the source map (file tree)
     */
    async fetchSourceMap() {
        const result = await this.sendRequest('getSourceMap');
        return result.fileTree || { root: { name: 'root', path: '/', type: 'directory', children: [] } };
    }
    /**
     * Fetch file content
     */
    async fetchFileContent(filePath) {
        return await this.sendRequest('getFile', { path: filePath });
    }
    /**
     * Fetch log history
     */
    async fetchLogs(limit = 100, levelFilter) {
        const params = { limit };
        if (levelFilter) {
            params.level = levelFilter;
        }
        const result = await this.sendRequest('getLogs', params);
        return result.logs || [];
    }
    /**
     * Send a message/command to the server
     */
    async sendMessage(message, sessionId) {
        await this.sendRequest('input', {
            input: message,
            sessionId: sessionId || 'default',
        });
    }
    /**
     * Check server health
     */
    async checkHealth() {
        try {
            const startTime = Date.now();
            const response = await fetch(`${this.config.baseUrl}/api/health`);
            const latency = Date.now() - startTime;
            if (!response.ok) {
                return { status: 'error', message: response.statusText };
            }
            const data = await response.json();
            return {
                status: data.status || 'ok',
                message: data.message,
                version: data.version,
            };
        }
        catch (error) {
            return {
                status: 'error',
                message: error instanceof Error ? error.message : 'Unknown error',
            };
        }
    }
    /**
     * Update connection status
     */
    updateConnectionStatus(connected, latency) {
        const now = Date.now();
        if (connected !== this.connectionStatus.connected) {
            if (connected) {
                this.connectionStatus.lastConnected = now;
                this.connectionStatus.reconnectAttempts = 0;
            }
            else {
                this.connectionStatus.lastDisconnected = now;
            }
        }
        this.connectionStatus.connected = connected;
        if (latency !== undefined) {
            this.connectionStatus.latency = latency;
        }
        this.emit('connection-status-change', this.connectionStatus);
    }
    /**
     * Start periodic health checks
     */
    startHealthCheck() {
        this.stopHealthCheck();
        // Initial health check
        this.checkHealth();
        // Periodic health checks
        this.healthCheckTimer = setInterval(() => {
            this.checkHealth();
        }, this.config.healthCheckInterval);
    }
    /**
     * Stop periodic health checks
     */
    stopHealthCheck() {
        if (this.healthCheckTimer) {
            clearInterval(this.healthCheckTimer);
            this.healthCheckTimer = null;
        }
    }
}
exports.CronusClient = CronusClient;
//# sourceMappingURL=CronusClient.js.map