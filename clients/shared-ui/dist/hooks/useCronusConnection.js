"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useCronusConnection = useCronusConnection;
const react_1 = require("react");
const shared_1 = require("@cronus/shared");
/**
 * React hook for managing Cronus server connection
 */
function useCronusConnection(options) {
    const [client, setClient] = (0, react_1.useState)(null);
    const [connected, setConnected] = (0, react_1.useState)(false);
    const [connectionStatus, setConnectionStatus] = (0, react_1.useState)({
        connected: false,
    });
    const [reconnecting, setReconnecting] = (0, react_1.useState)(false);
    // Initialize client
    (0, react_1.useEffect)(() => {
        const cronusClient = new shared_1.CronusClient({
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
        cronusClient.on('connection-status-change', (status) => {
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
    const connect = (0, react_1.useCallback)(() => {
        client?.start();
    }, [client]);
    const disconnect = (0, react_1.useCallback)(() => {
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
//# sourceMappingURL=useCronusConnection.js.map