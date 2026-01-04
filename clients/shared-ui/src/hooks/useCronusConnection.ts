import { useState, useEffect, useCallback } from 'react';
import { CronusClient, ConnectionStatus } from '@cronus/shared';

export interface UseCronusConnectionOptions {
    baseUrl: string;
    autoConnect?: boolean;
}

export interface UseCronusConnectionResult {
    client: CronusClient | null;
    connected: boolean;
    connectionStatus: ConnectionStatus;
    reconnecting: boolean;
    connect: () => void;
    disconnect: () => void;
}

/**
 * React hook for managing Cronus server connection
 */
export function useCronusConnection(
    options: UseCronusConnectionOptions
): UseCronusConnectionResult {
    const [client, setClient] = useState<CronusClient | null>(null);
    const [connected, setConnected] = useState(false);
    const [connectionStatus, setConnectionStatus] = useState<ConnectionStatus>({
        connected: false,
    });
    const [reconnecting, setReconnecting] = useState(false);

    // Initialize client
    useEffect(() => {
        const cronusClient = new CronusClient({
            baseUrl: options.baseUrl,
        });

        // Setup event listeners
        cronusClient.on('connected', () => {
            setConnected(true);
            setReconnecting(false);
        });

        cronusClient.on('disconnected', () => {
            setConnected(false);
        });

        cronusClient.on('reconnecting', () => {
            setReconnecting(true);
        });

        cronusClient.on('connection-status-change', (status: ConnectionStatus) => {
            setConnectionStatus(status);
        });

        setClient(cronusClient);

        // Auto-connect if specified
        if (options.autoConnect !== false) {
            cronusClient.start();
        }

        // Cleanup on unmount
        return () => {
            cronusClient.stop();
        };
    }, [options.baseUrl, options.autoConnect]);

    const connect = useCallback(() => {
        client?.start();
    }, [client]);

    const disconnect = useCallback(() => {
        client?.stop();
    }, [client]);

    return {
        client,
        connected,
        connectionStatus,
        reconnecting,
        connect,
        disconnect,
    };
}
