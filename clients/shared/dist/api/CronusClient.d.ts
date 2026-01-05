import EventEmitter from 'eventemitter3';
import { FileTree, FileContent, LogEntry, ServerHealth, ConnectionStatus } from '../models/types';
export interface CronusClientConfig {
    baseUrl: string;
    reconnectInterval?: number;
    maxReconnectAttempts?: number;
    healthCheckInterval?: number;
}
export declare class CronusClient extends EventEmitter {
    private config;
    private ws;
    private healthCheckTimer;
    private connectionStatus;
    private requestId;
    private pendingRequests;
    private reconnectTimer;
    private reconnectAttempts;
    constructor(config: CronusClientConfig);
    /**
     * Start the client (connect WebSocket and start health checks)
     */
    start(): void;
    /**
     * Stop the client
     */
    stop(): void;
    /**
     * Check if connected to the server
     */
    isConnected(): boolean;
    /**
     * Get current connection status
     */
    getConnectionStatus(): ConnectionStatus;
    /**
     * Connect to WebSocket
     */
    private connectWebSocket;
    /**
     * Disconnect WebSocket
     */
    private disconnectWebSocket;
    /**
     * Schedule reconnection attempt
     */
    private scheduleReconnect;
    /**
     * Handle incoming WebSocket message
     */
    private handleMessage;
    /**
     * Handle JSON-RPC notification
     */
    private handleNotification;
    /**
     * Send JSON-RPC request and wait for response
     */
    private sendRequest;
    /**
     * Fetch the source map (file tree)
     */
    fetchSourceMap(): Promise<FileTree>;
    /**
     * Fetch file content
     */
    fetchFileContent(filePath: string): Promise<FileContent>;
    /**
     * Fetch log history
     */
    fetchLogs(limit?: number, levelFilter?: string): Promise<LogEntry[]>;
    /**
     * Send a message/command to the server
     */
    sendMessage(message: string, sessionId?: string): Promise<void>;
    /**
     * Check server health
     */
    checkHealth(): Promise<ServerHealth>;
    /**
     * Update connection status
     */
    private updateConnectionStatus;
    /**
     * Start periodic health checks
     */
    private startHealthCheck;
    /**
     * Stop periodic health checks
     */
    private stopHealthCheck;
}
//# sourceMappingURL=CronusClient.d.ts.map