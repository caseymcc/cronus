import { LogEntry } from '../models/types';
export type LogLevel = 'debug' | 'info' | 'warning' | 'error';
export interface LoggerConfig {
    level?: LogLevel;
    prefix?: string;
    handler?: (entry: LogEntry) => void;
}
export declare class Logger {
    private config;
    private levelPriority;
    constructor(config?: LoggerConfig);
    debug(message: string, metadata?: Record<string, any>): void;
    info(message: string, metadata?: Record<string, any>): void;
    warning(message: string, metadata?: Record<string, any>): void;
    error(message: string, metadata?: Record<string, any>): void;
    private log;
    private defaultHandler;
    setLevel(level: LogLevel): void;
    setHandler(handler: (entry: LogEntry) => void): void;
}
export declare const logger: Logger;
//# sourceMappingURL=logger.d.ts.map