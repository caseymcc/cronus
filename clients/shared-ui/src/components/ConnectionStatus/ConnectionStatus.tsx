import React from 'react';
import { Box, Chip, Typography } from '@mui/material';
import { Circle as CircleIcon } from '@mui/icons-material';
import { ConnectionStatus } from '@cronus/shared';

export interface ConnectionStatusProps
{
    status: ConnectionStatus;
    reconnecting?: boolean;
    className?: string;
}

export const ConnectionStatusComponent: React.FC<ConnectionStatusProps>=({
    status,
    reconnecting=false,
    className='',
}) =>
{
    const getStatusText=() =>
    {
        if(reconnecting)
        {
            return 'Reconnecting...';
        }
        return status.connected ? 'Connected':'Disconnected';
    };

    const getStatusColor=(): 'success'|'error'|'warning' =>
    {
        if(reconnecting)
        {
            return 'warning';
        }
        return status.connected ? 'success':'error';
    };

    const formatLatency=() =>
    {
        if(!status.latency)
        {
            return null;
        }
        return `${status.latency}ms`;
    };

    return (
        <Box
            className={className}
            sx={{
                display: 'flex',
                alignItems: 'center',
                gap: 1,
            }}
        >
            <Chip
                icon={<CircleIcon sx={{ fontSize: 12 }} />}
                label={getStatusText()}
                color={getStatusColor()}
                size="small"
                variant="outlined"
            />
            {status.latency && (
                <Typography variant="body2" color="text.secondary" sx={{ fontSize: '0.75rem' }}>
                    Latency: {formatLatency()}
                </Typography>
            )}
            {reconnecting && status.reconnectAttempts && (
                <Typography variant="body2" color="warning.main" sx={{ fontSize: '0.75rem' }}>
                    Attempt {status.reconnectAttempts}
                </Typography>
            )}
        </Box>
    );
};
