import EventEmitter from 'eventemitter3';
import { SSEManager } from './SSEManager';
import {
    Message,
    FileTree,
    FileContent,
    AgentState,
    LogEntry,
    ServerHealth,
    ConnectionStatus,
} from '../models/types';

export interface CronusClientConfig {
    baseUrl: string;
    reconnectInterval?: number;
    maxReconnectAttempts?: number;
    healthCheckInterval?: number;
}

export class CronusClient extends EventEmitter {
    private config: Required<CronusClientConfig>;
    private sseManager: SSEManager;
    private healthCheckTimer: NodeJS.Timeout | null = null;
    private connectionStatus: ConnectionStatus = { connected: false };

    constructor(config: CronusClientConfig) {
        super();
        this.config = {
            baseUrl: config.baseUrl,
            reconnectInterval: config.reconnectInterval ?? 2000,
            maxReconnectAttempts: config.maxReconnectAttempts ?? 0,
            healthCheckInterval: config.healthCheckInterval ?? 30000,
        };

        // Initialize SSE manager
        this.sseManager = new SSEManager({
            baseUrl: this.config.baseUrl,
            reconnectInterval: this.config.reconnectInterval,
            maxReconnectAttempts: this.config.maxReconnectAttempts,
        });

        this.setupSSEHandlers();
    }

    /**
     * Start the client (connect SSE and start health checks)
     */
    start(): void {
        this.sseManager.connect();
        this.startHealthCheck();
    }

    /**
     * Stop the client
     */
    stop(): void {
        this.sseManager.disconnect();
        this.stopHealthCheck();
    }

    /**
     * Check if connected to the server
     */
    isConnected(): boolean {
        return this.connectionStatus.connected;
    }

    /**
     * Get current connection status
     */
    getConnectionStatus(): ConnectionStatus {
        return { ...this.connectionStatus };
    }

    /**
     * Fetch the source map (file tree)
     */
    async fetchSourceMap(): Promise<FileTree> {
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
    async fetchFileContent(filePath: string): Promise<FileContent> {
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
    async fetchLogs(limit: number = 100, levelFilter?: string): Promise<LogEntry[]> {
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
    async sendMessage(message: string, sessionId?: string): Promise<void> {
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
    async checkHealth(): Promise<ServerHealth> {
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
        } catch (error) {
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
    private setupSSEHandlers(): void {
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
    private updateConnectionStatus(connected: boolean, latency?: number): void {
        const now = Date.now();
        
        if (connected !== this.connectionStatus.connected) {
            if (connected) {
                this.connectionStatus.lastConnected = now;
                this.connectionStatus.reconnectAttempts = 0;
            } else {
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
    private startHealthCheck(): void {
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
    private stopHealthCheck(): void {
        if (this.healthCheckTimer) {
            clearInterval(this.healthCheckTimer);
            this.healthCheckTimer = null;
        }
    }
}
