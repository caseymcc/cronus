import { app, BrowserWindow, ipcMain } from 'electron';
import * as path from 'path';
import { CronusClient } from '@cronus/shared';
import * as dgram from 'dgram';

let mainWindow: BrowserWindow | null = null;
let cronusClient: CronusClient | null = null;
let notificationServer: dgram.Socket | null = null;

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1400,
        height: 900,
        minWidth: 1024,
        minHeight: 768,
        title: 'Cronus',
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
            preload: path.join(__dirname, 'preload.js')
        },
        backgroundColor: '#1e1e1e',
        show: false // Don't show until ready
    });

    // Load the web client build
    const startUrl = path.join(__dirname, '../renderer/index.html');
    mainWindow.loadFile(startUrl);

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

function setupNotificationListener() {
    const NOTIFICATION_PORT = parseInt(process.env.NOTIFICATION_PORT || '8999');
    
    notificationServer = dgram.createSocket('udp4');

    notificationServer.on('error', (err) => {
        console.error(`UDP notification listener error: ${err.stack}`);
        notificationServer?.close();
        notificationServer = null;
    });

    notificationServer.on('message', (msg, rinfo) => {
        try {
            const notification = JSON.parse(msg.toString());
            console.log(`Received startup notification from ${rinfo.address}:${rinfo.port}`);
            console.log(`Server URL: ${notification.serverUrl}`);

            if (notification.type === 'server_startup' && notification.serverUrl) {
                // Update the Cronus client to connect to the new server
                if (cronusClient) {
                    cronusClient.stop();
                }

                cronusClient = new CronusClient({
                    baseUrl: notification.serverUrl,
                    reconnectInterval: 2000
                });

                setupCronusClient();
                
                // Notify renderer about the server URL update
                mainWindow?.webContents.send('cronus:server-discovered', notification.serverUrl);
            }
        } catch (error) {
            console.error('Error parsing notification:', error);
        }
    });

    notificationServer.on('listening', () => {
        const address = notificationServer?.address();
        if (address && typeof address !== 'string') {
            console.log(`UDP notification listener active on ${address.address}:${address.port}`);
        }
    });

    notificationServer.bind(NOTIFICATION_PORT);
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

ipcMain.handle('cronus:is-electron', () => {
    return true;
});

// App lifecycle
app.whenReady().then(() => {
    createWindow();
    setupNotificationListener();
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
    if (notificationServer) {
        notificationServer.close();
    }
});
