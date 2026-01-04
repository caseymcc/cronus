"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.SSEManager = void 0;
const eventemitter3_1 = __importDefault(require("eventemitter3"));
const eventsource_1 = __importDefault(require("eventsource"));
class SSEManager extends eventemitter3_1.default {
    constructor(config) {
        super();
        this.eventSource = null;
        this.reconnectTimer = null;
        this.reconnectAttempts = 0;
        this.connected = false;
        this.intentionalDisconnect = false;
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
    connect() {
        if (this.eventSource) {
            return; // Already connected
        }
        this.intentionalDisconnect = false;
        const url = `${this.config.baseUrl}/api/events`;
        try {
            this.eventSource = new eventsource_1.default(url);
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
                    const sseEvent = {
                        type: data.type || 'message',
                        data: data,
                        timestamp: Date.now(),
                    };
                    this.emit('event', sseEvent);
                    this.emit(sseEvent.type, sseEvent.data);
                }
                catch (err) {
                    this.emit('parse-error', err);
                }
            };
        }
        catch (err) {
            this.emit('error', err);
            this.scheduleReconnect();
        }
    }
    /**
     * Disconnect from the SSE endpoint
     */
    disconnect() {
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
    isConnected() {
        return this.connected;
    }
    /**
     * Get current reconnect attempts count
     */
    getReconnectAttempts() {
        return this.reconnectAttempts;
    }
    /**
     * Schedule a reconnection attempt
     */
    scheduleReconnect() {
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
            delay = Math.min(this.config.reconnectInterval * Math.pow(2, this.reconnectAttempts), 30000 // Max 30 seconds
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
    clearReconnectTimer() {
        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }
    }
}
exports.SSEManager = SSEManager;
//# sourceMappingURL=SSEManager.js.map