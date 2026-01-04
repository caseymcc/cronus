import EventEmitter from 'eventemitter3';
import EventSource from 'eventsource';
import { SSEEvent } from '../models/types';

export interface SSEManagerConfig {
    baseUrl: string;
    reconnectInterval?: number;
    maxReconnectAttempts?: number;
    reconnectBackoff?: boolean;
}

export class SSEManager extends EventEmitter {
    private config: Required<SSEManagerConfig>;
    private eventSource: EventSource | null = null;
    private reconnectTimer: NodeJS.Timeout | null = null;
    private reconnectAttempts = 0;
    private connected = false;
    private intentionalDisconnect = false;

    constructor(config: SSEManagerConfig) {
        super();
        this.config = {
            baseUrl: config.baseUrl,
            reconnectInterval: config.reconnectInterval ?? 2000,
            maxReconnectAttempts: config.maxReconnectAttempts ?? 0, // 0 = infinite
            reconnectBackoff: config.reconnectBackoff ?? true,
        };
    }

    /**
     * Connect to the SSE endpoint
     */
    connect(): void {
        if (this.eventSource) {
            return; // Already connected
        }

        this.intentionalDisconnect = false;
        const url = `${this.config.baseUrl}/api/events`;

        try {
            this.eventSource = new EventSource(url);

            this.eventSource.onopen = () => {
                this.connected = true;
                this.reconnectAttempts = 0;
                this.emit('connected');
                this.emit('status', { connected: true });
            };

            this.eventSource.onerror = (error) => {
                this.connected = false;
                this.emit('error', error);
                this.emit('status', { connected: false });
                
                // Close the failed connection
                this.eventSource?.close();
                this.eventSource = null;

                // Attempt reconnection if not intentionally disconnected
                if (!this.intentionalDisconnect) {
                    this.scheduleReconnect();
                }
            };

            this.eventSource.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    const sseEvent: SSEEvent = {
                        type: data.type || 'message',
                        data: data,
                        timestamp: Date.now(),
                    };
                    this.emit('event', sseEvent);
                    this.emit(sseEvent.type, sseEvent.data);
                } catch (err) {
                    this.emit('parse-error', err);
                }
            };
        } catch (err) {
            this.emit('error', err);
            this.scheduleReconnect();
        }
    }

    /**
     * Disconnect from the SSE endpoint
     */
    disconnect(): void {
        this.intentionalDisconnect = true;
        this.clearReconnectTimer();
        
        if (this.eventSource) {
            this.eventSource.close();
            this.eventSource = null;
        }

        this.connected = false;
        this.reconnectAttempts = 0;
        this.emit('disconnected');
        this.emit('status', { connected: false });
    }

    /**
     * Check if currently connected
     */
    isConnected(): boolean {
        return this.connected;
    }

    /**
     * Get current reconnect attempts count
     */
    getReconnectAttempts(): number {
        return this.reconnectAttempts;
    }

    /**
     * Schedule a reconnection attempt
     */
    private scheduleReconnect(): void {
        if (this.reconnectTimer) {
            return; // Already scheduled
        }

        // Check if we've exceeded max attempts
        if (this.config.maxReconnectAttempts > 0 && 
            this.reconnectAttempts >= this.config.maxReconnectAttempts) {
            this.emit('max-reconnect-attempts');
            return;
        }

        // Calculate delay with exponential backoff
        let delay = this.config.reconnectInterval;
        if (this.config.reconnectBackoff) {
            delay = Math.min(
                this.config.reconnectInterval * Math.pow(2, this.reconnectAttempts),
                30000 // Max 30 seconds
            );
        }

        this.reconnectAttempts++;
        this.emit('reconnecting', { attempt: this.reconnectAttempts, delay });

        this.reconnectTimer = setTimeout(() => {
            this.reconnectTimer = null;
            this.connect();
        }, delay);
    }

    /**
     * Clear the reconnect timer
     */
    private clearReconnectTimer(): void {
        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }
    }
}
