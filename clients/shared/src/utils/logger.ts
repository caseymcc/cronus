import { LogEntry } from '../models/types';

export type LogLevel = 'debug' | 'info' | 'warning' | 'error';

export interface LoggerConfig {
    level?: LogLevel;
    prefix?: string;
    handler?: (entry: LogEntry) => void;
}

export class Logger {
    private config: Required<LoggerConfig>;
    private levelPriority: Record<LogLevel, number> = {
        debug: 0,
        info: 1,
        warning: 2,
        error: 3,
    };

    constructor(config: LoggerConfig = {}) {
        this.config = {
            level: config.level ?? 'info',
            prefix: config.prefix ?? '',
            handler: config.handler ?? this.defaultHandler,
        };
    }

    debug(message: string, metadata?: Record<string, any>): void {
        this.log('debug', message, metadata);
    }

    info(message: string, metadata?: Record<string, any>): void {
        this.log('info', message, metadata);
    }

    warning(message: string, metadata?: Record<string, any>): void {
        this.log('warning', message, metadata);
    }

    error(message: string, metadata?: Record<string, any>): void {
        this.log('error', message, metadata);
    }

    private log(level: LogLevel, message: string, metadata?: Record<string, any>): void {
        if (this.levelPriority[level] < this.levelPriority[this.config.level]) {
            return; // Skip logs below configured level
        }

        const entry: LogEntry = {
            timestamp: Date.now(),
            level,
            message: this.config.prefix ? `[${this.config.prefix}] ${message}` : message,
            metadata,
        };

        this.config.handler(entry);
    }

    private defaultHandler(entry: LogEntry): void {
        const timestamp = new Date(entry.timestamp).toISOString();
        const prefix = `[${timestamp}] [${entry.level.toUpperCase()}]`;
        
        switch (entry.level) {
            case 'debug':
                console.debug(prefix, entry.message, entry.metadata);
                break;
            case 'info':
                console.info(prefix, entry.message, entry.metadata);
                break;
            case 'warning':
                console.warn(prefix, entry.message, entry.metadata);
                break;
            case 'error':
                console.error(prefix, entry.message, entry.metadata);
                break;
        }
    }

    setLevel(level: LogLevel): void {
        this.config.level = level;
    }

    setHandler(handler: (entry: LogEntry) => void): void {
        this.config.handler = handler;
    }
}

// Export a default logger instance
export const logger = new Logger({ prefix: 'Cronus' });
