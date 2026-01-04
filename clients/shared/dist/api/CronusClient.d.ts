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
    private sseManager;
    private healthCheckTimer;
    private connectionStatus;
    constructor(config: CronusClientConfig);
    /**
     * Start the client (connect SSE and start health checks)
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
     * Setup SSE event handlers
     */
    private setupSSEHandlers;
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