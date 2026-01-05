import EventEmitter from 'eventemitter3';
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

interface JsonRpcRequest {
    jsonrpc: '2.0';
    method: string;
    params?: any;
    id: number | string;
}

interface JsonRpcResponse {
    jsonrpc: '2.0';
    result?: any;
    error?: {
        code: number;
        message: string;
        data?: any;
    };
    id: number | string | null;
}

interface JsonRpcNotification {
    jsonrpc: '2.0';
    method: string;
    params?: any;
}

export class CronusClient extends EventEmitter {
    private config: Required<CronusClientConfig>;
    private ws: WebSocket | null = null;
    private healthCheckTimer: NodeJS.Timeout | null = null;
    private connectionStatus: ConnectionStatus = { connected: false };
    private requestId = 0;
    private pendingRequests = new Map<number | string, {
        resolve: (value: any) => void;
        reject: (error: Error) => void;
    }>();
    private reconnectTimer: NodeJS.Timeout | null = null;
    private reconnectAttempts = 0;

    constructor(config: CronusClientConfig) {
        super();
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
    start(): void {
        this.connectWebSocket();
        this.startHealthCheck();
    }

    /**
     * Stop the client
     */
    stop(): void {
        this.disconnectWebSocket();
        this.stopHealthCheck();
    }

    /**
     * Check if connected to the server
     */
    isConnected(): boolean {
        return this.connectionStatus.connected && this.ws?.readyState === WebSocket.OPEN;
    }

    /**
     * Get current connection status
     */
    getConnectionStatus(): ConnectionStatus {
        return { ...this.connectionStatus };
    }

    /**
     * Connect to WebSocket
     */
    private connectWebSocket(): void {
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
        } catch (error) {
            this.emit('error', error);
            this.scheduleReconnect();
        }
    }

    /**
     * Disconnect WebSocket
     */
    private disconnectWebSocket(): void {
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
    private scheduleReconnect(): void {
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
    private handleMessage(data: string): void {
        try {
            const message = JSON.parse(data);

            // Handle JSON-RPC response
            if ('id' in message && message.id !== undefined) {
                const pending = this.pendingRequests.get(message.id);
                if (pending) {
                    this.pendingRequests.delete(message.id);
                    
                    if (message.error) {
                        pending.reject(new Error(message.error.message));
                    } else {
                        pending.resolve(message.result);
                    }
                }
            }
            // Handle JSON-RPC notification
            else if ('method' in message) {
                this.handleNotification(message as JsonRpcNotification);
            }
        } catch (error) {
            this.emit('error', error);
        }
    }

    /**
     * Handle JSON-RPC notification
     */
    private handleNotification(notification: JsonRpcNotification): void {
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
    private async sendRequest(method: string, params?: any): Promise<any> {
        if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
            throw new Error('WebSocket not connected');
        }

        const id = ++this.requestId;
        const request: JsonRpcRequest = {
            jsonrpc: '2.0',
            method,
            params,
            id,
        };

        return new Promise((resolve, reject) => {
            this.pendingRequests.set(id, { resolve, reject });

            try {
                this.ws!.send(JSON.stringify(request));
            } catch (error) {
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
    async fetchSourceMap(): Promise<FileTree> {
        const result = await this.sendRequest('getSourceMap');
        return result.fileTree || { root: { name: 'root', path: '/', type: 'directory', children: [] } };
    }

    /**
     * Fetch file content
     */
    async fetchFileContent(filePath: string): Promise<FileContent> {
        return await this.sendRequest('getFile', { path: filePath });
    }

    /**
     * Fetch log history
     */
    async fetchLogs(limit: number = 100, levelFilter?: string): Promise<LogEntry[]> {
        const params: any = { limit };
        if (levelFilter) {
            params.level = levelFilter;
        }
        
        const result = await this.sendRequest('getLogs', params);
        return result.logs || [];
    }

    /**
     * Send a message/command to the server
     */
    async sendMessage(message: string, sessionId?: string): Promise<void> {
        await this.sendRequest('input', {
            input: message,
            sessionId: sessionId || 'default',
        });
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
            
            return {
                status: data.status || 'ok',
                message: data.message,
                version: data.version,
            };
        } catch (error) {
            return {
                status: 'error',
                message: error instanceof Error ? error.message : 'Unknown error',
            };
        }
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
