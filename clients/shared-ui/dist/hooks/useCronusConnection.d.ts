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
export declare function useCronusConnection(options: UseCronusConnectionOptions): UseCronusConnectionResult;
//# sourceMappingURL=useCronusConnection.d.ts.map