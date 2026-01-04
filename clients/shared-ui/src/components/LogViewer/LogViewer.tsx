import React, { useState, useEffect, useRef } from 'react';
import { CronusClient, LogEntry } from '@cronus/shared';
import './LogViewer.css';

export interface LogViewerProps {
    client: CronusClient | null;
    maxLogs?: number;
    autoScroll?: boolean;
    showTimestamp?: boolean;
    levelFilter?: string[];
    className?: string;
}

export const LogViewer: React.FC<LogViewerProps> = ({
    client,
    maxLogs = 500,
    autoScroll = true,
    showTimestamp = true,
    levelFilter = [],
    className = '',
}) => {
    const [logs, setLogs] = useState<LogEntry[]>([]);
    const [filter, setFilter] = useState<string>('');
    const [selectedLevels, setSelectedLevels] = useState<Set<string>>(
        new Set(levelFilter.length > 0 ? levelFilter : ['debug', 'info', 'warning', 'error'])
    );
    const logsEndRef = useRef<HTMLDivElement>(null);
    const logContainerRef = useRef<HTMLDivElement>(null);

    // Load initial logs
    useEffect(() => {
        if (!client) return;

        const loadInitialLogs = async () => {
            try {
                const historicalLogs = await client.fetchLogs(100);
                setLogs(historicalLogs);
            } catch (err) {
                console.error('Failed to load initial logs:', err);
            }
        };

        loadInitialLogs();
    }, [client]);

    // Listen for new logs
    useEffect(() => {
        if (!client) return;

        const handleLog = (data: any) => {
            const logEntry: LogEntry = {
                timestamp: data.timestamp || Date.now(),
                level: data.level || 'info',
                message: data.message || '',
                source: data.source,
                metadata: data.metadata,
            };

            setLogs((prevLogs) => {
                const newLogs = [...prevLogs, logEntry];
                // Trim to max logs
                if (newLogs.length > maxLogs) {
                    return newLogs.slice(newLogs.length - maxLogs);
                }
                return newLogs;
            });
        };

        client.on('log', handleLog);

        return () => {
            client.off('log', handleLog);
        };
    }, [client, maxLogs]);

    // Auto-scroll to bottom
    useEffect(() => {
        if (autoScroll && logsEndRef.current) {
            logsEndRef.current.scrollIntoView({ behavior: 'smooth' });
        }
    }, [logs, autoScroll]);

    const handleClearLogs = () => {
        setLogs([]);
    };

    const handleToggleLevel = (level: string) => {
        setSelectedLevels((prev) => {
            const newSet = new Set(prev);
            if (newSet.has(level)) {
                newSet.delete(level);
            } else {
                newSet.add(level);
            }
            return newSet;
        });
    };

    const filteredLogs = logs.filter((log) => {
        // Filter by level
        if (!selectedLevels.has(log.level)) {
            return false;
        }

        // Filter by search text
        if (filter && !log.message.toLowerCase().includes(filter.toLowerCase())) {
            return false;
        }

        return true;
    });

    const getLevelClass = (level: string): string => {
        return `log-level-${level}`;
    };

    const formatTimestamp = (timestamp: number | string): string => {
        if (typeof timestamp === 'string') {
            return timestamp;
        }
        const date = new Date(timestamp);
        return date.toLocaleTimeString('en-US', { hour12: false });
    };

    return (
        <div className={`log-viewer ${className}`}>
            <div className="log-viewer-header">
                <div className="log-viewer-title">Logs ({filteredLogs.length})</div>
                <div className="log-viewer-controls">
                    <input
                        type="text"
                        className="log-search"
                        placeholder="Filter logs..."
                        value={filter}
                        onChange={(e) => setFilter(e.target.value)}
                    />
                    <div className="log-level-filters">
                        {['debug', 'info', 'warning', 'error'].map((level) => (
                            <button
                                key={level}
                                className={`log-level-btn log-level-${level} ${
                                    selectedLevels.has(level) ? 'active' : ''
                                }`}
                                onClick={() => handleToggleLevel(level)}
                                title={`Toggle ${level} logs`}
                            >
                                {level.charAt(0).toUpperCase()}
                            </button>
                        ))}
                    </div>
                    <button className="log-clear-btn" onClick={handleClearLogs} title="Clear logs">
                        Clear
                    </button>
                </div>
            </div>

            <div className="log-viewer-content" ref={logContainerRef}>
                {filteredLogs.length === 0 ? (
                    <div className="log-empty">No logs to display</div>
                ) : (
                    filteredLogs.map((log, index) => (
                        <div key={index} className={`log-entry ${getLevelClass(log.level)}`}>
                            {showTimestamp && (
                                <span className="log-timestamp">{formatTimestamp(log.timestamp)}</span>
                            )}
                            <span className="log-level">[{log.level.toUpperCase()}]</span>
                            <span className="log-message">{log.message}</span>
                            {log.source && <span className="log-source">({log.source})</span>}
                        </div>
                    ))
                )}
                <div ref={logsEndRef} />
            </div>
        </div>
    );
};
