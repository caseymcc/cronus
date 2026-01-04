"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.CronusClient = void 0;
const eventemitter3_1 = __importDefault(require("eventemitter3"));
const SSEManager_1 = require("./SSEManager");
class CronusClient extends eventemitter3_1.default {
    constructor(config) {
        super();
        this.healthCheckTimer = null;
        this.connectionStatus = { connected: false };
        this.config = {
            baseUrl: config.baseUrl,
            reconnectInterval: config.reconnectInterval ?? 2000,
            maxReconnectAttempts: config.maxReconnectAttempts ?? 0,
            healthCheckInterval: config.healthCheckInterval ?? 30000,
        };
        // Initialize SSE manager
        this.sseManager = new SSEManager_1.SSEManager({
            baseUrl: this.config.baseUrl,
            reconnectInterval: this.config.reconnectInterval,
            maxReconnectAttempts: this.config.maxReconnectAttempts,
        });
        this.setupSSEHandlers();
    }
    /**
     * Start the client (connect SSE and start health checks)
     */
    start() {
        this.sseManager.connect();
        this.startHealthCheck();
    }
    /**
     * Stop the client
     */
    stop() {
        this.sseManager.disconnect();
        this.stopHealthCheck();
    }
    /**
     * Check if connected to the server
     */
    isConnected() {
        return this.connectionStatus.connected;
    }
    /**
     * Get current connection status
     */
    getConnectionStatus() {
        return { ...this.connectionStatus };
    }
    /**
     * Fetch the source map (file tree)
     */
    async fetchSourceMap() {
        const response = await fetch(`${this.config.baseUrl}/api/sourcemap`);
        if (!response.ok) {
            throw new Error(`Failed to fetch source map: ${response.statusText}`);
        }
        const data = await response.json();
        return data.fileTree || { root: { name: 'root', path: '/', type: 'directory', children: [] } };
    }
    /**
     * Fetch file content
     */
    async fetchFileContent(filePath) {
        const encodedPath = encodeURIComponent(filePath);
        const response = await fetch(`${this.config.baseUrl}/api/file?path=${encodedPath}`);
        if (!response.ok) {
            throw new Error(`Failed to fetch file: ${response.statusText}`);
        }
        return await response.json();
    }
    /**
     * Fetch log history
     */
    async fetchLogs(limit = 100, levelFilter) {
        let url = `${this.config.baseUrl}/api/logs?limit=${limit}`;
        if (levelFilter) {
            url += `&level=${levelFilter}`;
        }
        const response = await fetch(url);
        if (!response.ok) {
            throw new Error(`Failed to fetch logs: ${response.statusText}`);
        }
        const data = await response.json();
        return data.logs || [];
    }
    /**
     * Send a message/command to the server
     */
    async sendMessage(message, sessionId) {
        const response = await fetch(`${this.config.baseUrl}/api/input`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                input: message,
                sessionId: sessionId || 'default',
            }),
        });
        if (!response.ok) {
            throw new Error(`Failed to send message: ${response.statusText}`);
        }
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
            // Update connection status
            this.updateConnectionStatus(true, latency);
            return {
                status: data.status || 'ok',
                message: data.message,
                version: data.version,
            };
        }
        catch (error) {
            this.updateConnectionStatus(false);
            return {
                status: 'error',
                message: error instanceof Error ? error.message : 'Unknown error',
            };
        }
    }
    /**
     * Setup SSE event handlers
     */
    setupSSEHandlers() {
        // Connection events
        this.sseManager.on('connected', () => {
            this.updateConnectionStatus(true);
            this.emit('connected');
        });
        this.sseManager.on('disconnected', () => {
            this.updateConnectionStatus(false);
            this.emit('disconnected');
        });
        this.sseManager.on('reconnecting', (data) => {
            this.emit('reconnecting', data);
        });
        this.sseManager.on('status', (status) => {
            this.emit('connection-status', status);
        });
        // Data events
        this.sseManager.on('message', (data) => {
            this.emit('message', data);
        });
        this.sseManager.on('directory_update', (data) => {
            this.emit('directory-update', data);
        });
        this.sseManager.on('log', (data) => {
            this.emit('log', data);
        });
        this.sseManager.on('agent_status', (data) => {
            this.emit('agent-status', data);
        });
        // Forward all events
        this.sseManager.on('event', (event) => {
            this.emit('sse-event', event);
        });
        // Error events
        this.sseManager.on('error', (error) => {
            this.emit('error', error);
        });
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