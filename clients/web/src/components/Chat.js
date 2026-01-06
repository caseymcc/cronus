import React, { useEffect, useRef } from 'react';
import {
    Box,
    Paper,
    Typography,
    Chip,
    Avatar,
} from '@mui/material';
import {
    Person as PersonIcon,
    SmartToy as AssistantIcon,
} from '@mui/icons-material';

function formatTimestamp(timestamp)
{
    const date=new Date(timestamp);
    return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
}

function Chat({ messages, logs, showLogs, selectedFile })
{
    const messagesEndRef=useRef(null);
    
    // Auto-scroll to bottom when messages change
    useEffect(() =>
    {
        if(messagesEndRef.current)
        {
            messagesEndRef.current.scrollIntoView({ behavior: 'smooth' });
        }
    }, [messages]);
    
    return (
        <Box
            sx={{
                display: 'flex',
                flexDirection: 'column',
                height: '100%',
                overflow: 'hidden',
            }}
        >
            <Box
                sx={{
                    flex: 1,
                    overflow: 'auto',
                    p: 2,
                    display: 'flex',
                    flexDirection: 'column',
                    gap: 2,
                }}
            >
                {selectedFile && (
                    <Paper
                        sx={{
                            p: 1.5,
                            backgroundColor: 'background.paper',
                            borderLeft: 3,
                            borderColor: 'primary.main',
                        }}
                    >
                        <Typography variant="subtitle2" fontWeight="bold">
                            {selectedFile.path.split('/').pop()}
                        </Typography>
                        <Typography variant="caption" color="text.secondary">
                            {selectedFile.path}
                        </Typography>
                    </Paper>
                )}
                
                {messages.map((message, index) => (
                    <Box
                        key={index}
                        sx={{
                            display: 'flex',
                            gap: 1.5,
                            alignItems: 'flex-start',
                        }}
                    >
                        <Avatar
                            sx={{
                                width: 32,
                                height: 32,
                                bgcolor: message.role==='user' ? 'primary.main':'secondary.main',
                            }}
                        >
                            {message.role==='user' ? <PersonIcon />:<AssistantIcon />}
                        </Avatar>
                        <Box sx={{ flex: 1, minWidth: 0 }}>
                            <Box
                                sx={{
                                    display: 'flex',
                                    alignItems: 'center',
                                    gap: 1,
                                    mb: 0.5,
                                }}
                            >
                                <Typography variant="subtitle2" fontWeight="bold">
                                    {message.role==='user' ? 'You':message.role}
                                </Typography>
                                <Typography variant="caption" color="text.secondary">
                                    {formatTimestamp(message.timestamp)}
                                </Typography>
                            </Box>
                            <Paper
                                sx={{
                                    p: 1.5,
                                    backgroundColor: message.role==='user'
                                        ? 'rgba(52, 152, 219, 0.1)'
                                        :'background.paper',
                                }}
                            >
                                <Typography variant="body2" sx={{ whiteSpace: 'pre-wrap' }}>
                                    {message.content}
                                </Typography>
                            </Paper>
                        </Box>
                    </Box>
                ))}
                <div ref={messagesEndRef} />
            </Box>
            
            {showLogs && logs.length>0 && (
                <Paper
                    sx={{
                        borderTop: 1,
                        borderColor: 'divider',
                        maxHeight: '30%',
                        overflow: 'auto',
                        p: 1,
                    }}
                >
                    <Typography variant="subtitle2" sx={{ mb: 1 }}>
                        System Logs
                    </Typography>
                    <Box sx={{ display: 'flex', flexDirection: 'column', gap: 0.5 }}>
                        {logs.map((log, index) => (
                            <Box
                                key={index}
                                sx={{
                                    display: 'flex',
                                    gap: 1,
                                    alignItems: 'center',
                                    fontSize: '0.75rem',
                                    fontFamily: 'monospace',
                                }}
                            >
                                <Typography
                                    variant="caption"
                                    color="text.secondary"
                                    sx={{ minWidth: '60px' }}
                                >
                                    {formatTimestamp(log.timestamp)}
                                </Typography>
                                <Chip
                                    label={log.level}
                                    size="small"
                                    color={
                                        log.level==='error' ? 'error':
                                        log.level==='warning' ? 'warning':
                                        log.level==='info' ? 'info':'default'
                                    }
                                    sx={{ fontSize: '0.65rem', height: 20 }}
                                />
                                <Typography variant="caption" sx={{ flex: 1 }}>
                                    {log.message}
                                </Typography>
                            </Box>
                        ))}
                    </Box>
                </Paper>
            )}
        </Box>
    );
}

export default Chat;
