import React, { useState, useEffect, useRef } from 'react';
import {
    Box,
    Paper,
    Typography,
    TextField,
    Button,
    ToggleButton,
    ToggleButtonGroup,
    Chip,
} from '@mui/material';
import {
    Delete as DeleteIcon,
    BugReport as DebugIcon,
    Info as InfoIcon,
    Warning as WarningIcon,
    Error as ErrorIcon,
} from '@mui/icons-material';
import { CronusClient, LogEntry } from '@cronus/shared';

export interface LogViewerProps
{
    client: CronusClient|null;
    maxLogs?: number;
    autoScroll?: boolean;
    showTimestamp?: boolean;
    levelFilter?: string[];
    className?: string;
}

export const LogViewer: React.FC<LogViewerProps>=({
    client,
    maxLogs=500,
    autoScroll=true,
    showTimestamp=true,
    levelFilter=[],
    className='',
}) =>
{
    const [logs, setLogs]=useState<LogEntry[]>([]);
    const [filter, setFilter]=useState<string>('');
    const [selectedLevels, setSelectedLevels]=useState<Set<string>>(
        new Set(levelFilter.length>0 ? levelFilter:['debug', 'info', 'warning', 'error'])
    );
    const logsEndRef=useRef<HTMLDivElement>(null);
    const logContainerRef=useRef<HTMLDivElement>(null);

    // Load initial logs
    useEffect(() =>
    {
        if(!client) return;

        const loadInitialLogs=async() =>
        {
            try
            {
                const historicalLogs=await client.fetchLogs(100);
                setLogs(historicalLogs);
            }
            catch(err)
            {
                console.error('Failed to load initial logs:', err);
            }
        };

        loadInitialLogs();
    }, [client]);

    // Listen for new logs
    useEffect(() =>
    {
        if(!client) return;

        const handleLog=(data: any) =>
        {
            const logEntry: LogEntry={
                timestamp: data.timestamp||Date.now(),
                level: data.level||'info',
                message: data.message||'',
                source: data.source,
                metadata: data.metadata,
            };

            setLogs((prevLogs) =>
            {
                const newLogs=[...prevLogs, logEntry];
                // Trim to max logs
                if(newLogs.length>maxLogs)
                {
                    return newLogs.slice(newLogs.length-maxLogs);
                }
                return newLogs;
            });
        };

        client.on('log', handleLog);

        return () =>
        {
            client.off('log', handleLog);
        };
    }, [client, maxLogs]);

    // Auto-scroll to bottom
    useEffect(() =>
    {
        if(autoScroll && logsEndRef.current)
        {
            logsEndRef.current.scrollIntoView({ behavior: 'smooth' });
        }
    }, [logs, autoScroll]);

    const handleClearLogs=() =>
    {
        setLogs([]);
    };

    const handleToggleLevels=(event: React.MouseEvent<HTMLElement>, newLevels: string[]) =>
    {
        if(newLevels.length>0)
        {
            setSelectedLevels(new Set(newLevels));
        }
    };

    const filteredLogs=logs.filter((log) =>
    {
        // Filter by level
        if(!selectedLevels.has(log.level))
        {
            return false;
        }

        // Filter by search text
        if(filter && !log.message.toLowerCase().includes(filter.toLowerCase()))
        {
            return false;
        }

        return true;
    });

    const getLevelColor=(level: string): 'default'|'info'|'warning'|'error' =>
    {
        switch(level)
        {
            case 'debug':
                return 'default';
            case 'info':
                return 'info';
            case 'warning':
                return 'warning';
            case 'error':
                return 'error';
            default:
                return 'default';
        }
    };

    const getLevelIcon=(level: string) =>
    {
        switch(level)
        {
            case 'debug':
                return <DebugIcon fontSize="small" />;
            case 'info':
                return <InfoIcon fontSize="small" />;
            case 'warning':
                return <WarningIcon fontSize="small" />;
            case 'error':
                return <ErrorIcon fontSize="small" />;
            default:
                return <InfoIcon fontSize="small" />;
        }
    };

    const formatTimestamp=(timestamp: number|string): string =>
    {
        if(typeof timestamp==='string')
        {
            return timestamp;
        }
        const date=new Date(timestamp);
        return date.toLocaleTimeString('en-US', { hour12: false });
    };

    return (
        <Paper
            className={className}
            sx={{
                display: 'flex',
                flexDirection: 'column',
                height: '100%',
                overflow: 'hidden',
            }}
        >
            <Box
                sx={{
                    p: 1.5,
                    borderBottom: 1,
                    borderColor: 'divider',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'space-between',
                    gap: 2,
                }}
            >
                <Typography variant="h6" sx={{ fontSize: '1rem' }}>
                    Logs ({filteredLogs.length})
                </Typography>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, flex: 1 }}>
                    <TextField
                        size="small"
                        placeholder="Filter logs..."
                        value={filter}
                        onChange={(e) => setFilter(e.target.value)}
                        sx={{ flex: 1, maxWidth: 300 }}
                    />
                    <ToggleButtonGroup
                        value={Array.from(selectedLevels)}
                        onChange={handleToggleLevels}
                        size="small"
                        aria-label="log level filter"
                    >
                        <ToggleButton value="debug" aria-label="debug">
                            <DebugIcon fontSize="small" />
                        </ToggleButton>
                        <ToggleButton value="info" aria-label="info">
                            <InfoIcon fontSize="small" />
                        </ToggleButton>
                        <ToggleButton value="warning" aria-label="warning">
                            <WarningIcon fontSize="small" />
                        </ToggleButton>
                        <ToggleButton value="error" aria-label="error">
                            <ErrorIcon fontSize="small" />
                        </ToggleButton>
                    </ToggleButtonGroup>
                    <Button
                        variant="outlined"
                        size="small"
                        startIcon={<DeleteIcon />}
                        onClick={handleClearLogs}
                    >
                        Clear
                    </Button>
                </Box>
            </Box>

            <Box
                ref={logContainerRef}
                sx={{
                    flex: 1,
                    overflow: 'auto',
                    p: 1,
                    fontFamily: 'monospace',
                    fontSize: '0.85rem',
                }}
            >
                {filteredLogs.length===0 ? (
                    <Typography
                        variant="body2"
                        color="text.secondary"
                        sx={{ textAlign: 'center', mt: 4 }}
                    >
                        No logs to display
                    </Typography>
                ):(
                    filteredLogs.map((log, index) => (
                        <Box
                            key={index}
                            sx={{
                                display: 'flex',
                                alignItems: 'flex-start',
                                gap: 1,
                                mb: 0.5,
                                pb: 0.5,
                                borderBottom: '1px solid',
                                borderColor: 'divider',
                            }}
                        >
                            {showTimestamp && (
                                <Typography
                                    component="span"
                                    sx={{
                                        color: 'text.secondary',
                                        fontSize: '0.75rem',
                                        minWidth: '80px',
                                    }}
                                >
                                    {formatTimestamp(log.timestamp)}
                                </Typography>
                            )}
                            <Chip
                                icon={getLevelIcon(log.level)}
                                label={log.level.toUpperCase()}
                                color={getLevelColor(log.level)}
                                size="small"
                                sx={{ minWidth: '90px', fontSize: '0.7rem' }}
                            />
                            <Typography
                                component="span"
                                sx={{
                                    flex: 1,
                                    wordBreak: 'break-word',
                                    fontSize: '0.85rem',
                                }}
                            >
                                {log.message}
                                {log.source && (
                                    <Typography
                                        component="span"
                                        color="text.secondary"
                                        sx={{ ml: 1, fontSize: '0.75rem' }}
                                    >
                                        ({log.source})
                                    </Typography>
                                )}
                            </Typography>
                        </Box>
                    ))
                )}
                <div ref={logsEndRef} />
            </Box>
        </Paper>
    );
};
