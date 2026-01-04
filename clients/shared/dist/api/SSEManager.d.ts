import EventEmitter from 'eventemitter3';
export interface SSEManagerConfig {
    baseUrl: string;
    reconnectInterval?: number;
    maxReconnectAttempts?: number;
    reconnectBackoff?: boolean;
}
export declare class SSEManager extends EventEmitter {
    private config;
    private eventSource;
    private reconnectTimer;
    private reconnectAttempts;
    private connected;
    private intentionalDisconnect;
    constructor(config: SSEManagerConfig);
    /**
     * Connect to the SSE endpoint
     */
    connect(): void;
    /**
     * Disconnect from the SSE endpoint
     */
    disconnect(): void;
    /**
     * Check if currently connected
     */
    isConnected(): boolean;
    /**
     * Get current reconnect attempts count
     */
    getReconnectAttempts(): number;
    /**
     * Schedule a reconnection attempt
     */
    private scheduleReconnect;
    /**
     * Clear the reconnect timer
     */
    private clearReconnectTimer;
}
//# sourceMappingURL=SSEManager.d.ts.map