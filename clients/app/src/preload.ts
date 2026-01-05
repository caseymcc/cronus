import { contextBridge, ipcRenderer } from 'electron';

// Expose protected methods that allow the renderer process to use
// the ipcRenderer without exposing the entire object
contextBridge.exposeInMainWorld('cronusAPI', {
    // Connection control
    connect: () => ipcRenderer.invoke('cronus:connect'),
    disconnect: () => ipcRenderer.invoke('cronus:disconnect'),
    getConnectionStatus: () => ipcRenderer.invoke('cronus:get-connection-status'),
    setServerUrl: (url: string) => ipcRenderer.invoke('cronus:set-server-url', url),

    // Log retrieval
    getLogs: (limit?: number, levelFilter?: string) => 
        ipcRenderer.invoke('cronus:get-logs', limit, levelFilter),

    // Environment detection
    isElectron: () => ipcRenderer.invoke('cronus:is-electron'),

    // Event listeners
    onConnected: (callback: () => void) => {
        ipcRenderer.on('cronus:connected', callback);
        return () => ipcRenderer.removeListener('cronus:connected', callback);
    },
    onDisconnected: (callback: () => void) => {
        ipcRenderer.on('cronus:disconnected', callback);
        return () => ipcRenderer.removeListener('cronus:disconnected', callback);
    },
    onError: (callback: (error: string) => void) => {
        const listener = (_event: any, error: string) => callback(error);
        ipcRenderer.on('cronus:error', listener);
        return () => ipcRenderer.removeListener('cronus:error', listener);
    },
    onLog: (callback: (log: any) => void) => {
        const listener = (_event: any, log: any) => callback(log);
        ipcRenderer.on('cronus:log', listener);
        return () => ipcRenderer.removeListener('cronus:log', listener);
    }
});

// TypeScript declarations
export interface CronusAPI {
    connect: () => Promise<{ success: boolean; error?: string }>;
    disconnect: () => Promise<{ success: boolean; error?: string }>;
    getConnectionStatus: () => Promise<{ connected: boolean; serverUrl: string }>;
    setServerUrl: (url: string) => Promise<{ success: boolean; error?: string }>;
    getLogs: (limit?: number, levelFilter?: string) => Promise<{ success: boolean; logs?: any[]; error?: string }>;
    isElectron: () => Promise<boolean>;
    onConnected: (callback: () => void) => () => void;
    onDisconnected: (callback: () => void) => () => void;
    onError: (callback: (error: string) => void) => () => void;
    onLog: (callback: (log: any) => void) => () => void;
}

declare global {
    interface Window {
        cronusAPI?: CronusAPI;
    }
}
