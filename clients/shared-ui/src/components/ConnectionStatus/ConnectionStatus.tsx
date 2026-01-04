import React from 'react';
import { ConnectionStatus } from '@cronus/shared';
import './ConnectionStatus.css';

export interface ConnectionStatusProps {
    status: ConnectionStatus;
    reconnecting?: boolean;
    className?: string;
}

export const ConnectionStatusComponent: React.FC<ConnectionStatusProps> = ({
    status,
    reconnecting = false,
    className = '',
}) => {
    const getStatusText = () => {
        if (reconnecting) {
            return 'Reconnecting...';
        }
        return status.connected ? 'Connected' : 'Disconnected';
    };

    const getStatusClass = () => {
        if (reconnecting) {
            return 'status-reconnecting';
        }
        return status.connected ? 'status-connected' : 'status-disconnected';
    };

    const formatLatency = () => {
        if (!status.latency) {
            return null;
        }
        return `${status.latency}ms`;
    };

    return (
        <div className={`connection-status ${getStatusClass()} ${className}`}>
            <div className="status-indicator">
                <span className="status-dot"></span>
                <span className="status-text">{getStatusText()}</span>
            </div>
            {status.latency && (
                <div className="status-latency">
                    <span className="latency-label">Latency:</span>
                    <span className="latency-value">{formatLatency()}</span>
                </div>
            )}
            {reconnecting && status.reconnectAttempts && (
                <div className="status-attempts">
                    Attempt {status.reconnectAttempts}
                </div>
            )}
        </div>
    );
};
