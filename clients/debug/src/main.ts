import { app, BrowserWindow, ipcMain } from 'electron';
import * as path from 'path';
import { CronusClient } from '@cronus/shared';

let mainWindow: BrowserWindow | null = null;
let cronusClient: CronusClient | null = null;

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1200,
        height: 800,
        minWidth: 800,
        minHeight: 600,
        title: 'Cronus Debug Client',
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
            preload: path.join(__dirname, 'preload.js')
        },
        backgroundColor: '#1e1e1e',
        show: false // Don't show until ready
    });

    // Load the renderer
    mainWindow.loadFile(path.join(__dirname, '../renderer/index.html'));

    // Show window when ready
    mainWindow.once('ready-to-show', () => {
        mainWindow?.show();
    });

    // Open DevTools in development mode
    if (process.argv.includes('--dev')) {
        mainWindow.webContents.openDevTools();
    }

    mainWindow.on('closed', () => {
        mainWindow = null;
    });
}

function setupCronusClient() {
    // Default server URL, can be configured later
    const serverUrl = process.env.CRONUS_SERVER_URL || 'http://localhost:9000';
    
    cronusClient = new CronusClient({
        baseUrl: serverUrl,
        reconnectInterval: 2000
    });

    // Forward connection events to renderer
    cronusClient.on('connected', () => {
        mainWindow?.webContents.send('cronus:connected');
    });

    cronusClient.on('disconnected', () => {
        mainWindow?.webContents.send('cronus:disconnected');
    });

    cronusClient.on('error', (error: Error) => {
        mainWindow?.webContents.send('cronus:error', error.message);
    });

    // Forward log events to renderer
    cronusClient.on('log', (logData: any) => {
        mainWindow?.webContents.send('cronus:log', logData);
    });

    // Start connecting
    cronusClient.start();
}

// IPC handlers for renderer requests
ipcMain.handle('cronus:connect', async () => {
    if (cronusClient) {
        cronusClient.start();
        return { success: true };
    }
    return { success: false, error: 'Client not initialized' };
});

ipcMain.handle('cronus:disconnect', async () => {
    if (cronusClient) {
        cronusClient.stop();
        return { success: true };
    }
    return { success: false, error: 'Client not initialized' };
});

ipcMain.handle('cronus:get-logs', async (event, limit?: number, levelFilter?: string) => {
    if (cronusClient) {
        try {
            const logs = await cronusClient.fetchLogs(limit, levelFilter);
            return { success: true, logs };
        } catch (error) {
            return { success: false, error: (error as Error).message };
        }
    }
    return { success: false, error: 'Client not initialized' };
});

ipcMain.handle('cronus:get-connection-status', () => {
    if (cronusClient) {
        return {
            connected: cronusClient.isConnected(),
            serverUrl: (cronusClient as any).config?.baseUrl || 'unknown'
        };
    }
    return { connected: false, serverUrl: 'unknown' };
});

ipcMain.handle('cronus:set-server-url', (_event: any, url: string) => {
    if (cronusClient) {
        cronusClient.stop();
        cronusClient = new CronusClient({
            baseUrl: url,
            reconnectInterval: 2000
        });
        setupCronusClient();
        return { success: true };
    }
    return { success: false, error: 'Client not initialized' };
});

// App lifecycle
app.whenReady().then(() => {
    createWindow();
    setupCronusClient();

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow();
        }
    });
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('before-quit', () => {
    if (cronusClient) {
        cronusClient.stop();
    }
});
