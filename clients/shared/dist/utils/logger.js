"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.logger = exports.Logger = void 0;
class Logger {
    constructor(config = {}) {
        this.levelPriority = {
            debug: 0,
            info: 1,
            warning: 2,
            error: 3,
        };
        this.config = {
            level: config.level ?? 'info',
            prefix: config.prefix ?? '',
            handler: config.handler ?? this.defaultHandler,
        };
    }
    debug(message, metadata) {
        this.log('debug', message, metadata);
    }
    info(message, metadata) {
        this.log('info', message, metadata);
    }
    warning(message, metadata) {
        this.log('warning', message, metadata);
    }
    error(message, metadata) {
        this.log('error', message, metadata);
    }
    log(level, message, metadata) {
        if (this.levelPriority[level] < this.levelPriority[this.config.level]) {
            return; // Skip logs below configured level
        }
        const entry = {
            timestamp: Date.now(),
            level,
            message: this.config.prefix ? `[${this.config.prefix}] ${message}` : message,
            metadata,
        };
        this.config.handler(entry);
    }
    defaultHandler(entry) {
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
    setLevel(level) {
        this.config.level = level;
    }
    setHandler(handler) {
        this.config.handler = handler;
    }
}
exports.Logger = Logger;
// Export a default logger instance
exports.logger = new Logger({ prefix: 'Cronus' });
//# sourceMappingURL=logger.js.map