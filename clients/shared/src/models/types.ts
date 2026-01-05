/**
 * Core data types shared across all Cronus clients
 */

export interface Message {
    id: string;
    role: 'user' | 'assistant' | 'system';
    content: string;
    timestamp: number;
    metadata?: Record<string, any>;
}

export interface FileNode {
    name: string;
    path: string;
    type: 'file' | 'directory';
    children?: FileNode[];
    size?: number;
    modified?: number;
}

export interface FileTree {
    root: FileNode;
    version?: string;
}

export interface FileContent {
    path: string;
    content: string;
    language?: string;
    encoding?: string;
}

export interface AgentState {
    id: string;
    status: 'active' | 'idle' | 'error' | 'disconnected';
    model?: string;
    created_at?: string;
    last_active?: string;
    working_directory?: string;
}

export interface LogEntry {
    timestamp: number;
    level: 'debug' | 'info' | 'warning' | 'error';
    message: string;
    source?: string;
    metadata?: Record<string, any>;
}

export interface ConnectionStatus {
    connected: boolean;
    lastConnected?: number;
    lastDisconnected?: number;
    reconnectAttempts?: number;
    latency?: number;
}

export interface ServerHealth {
    status: 'ok' | 'error';
    message?: string;
    version?: string;
    uptime?: number;
}
