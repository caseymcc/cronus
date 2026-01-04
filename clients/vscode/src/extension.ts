import * as vscode from 'vscode';
import { CronusDebugPanel } from './panels/CronusDebugPanel';
import { CronusClient } from '@cronus/shared';

let cronusClient: CronusClient | undefined;

export function activate(context: vscode.ExtensionContext) {
    console.log('Cronus Debug extension is now active');

    // Get configuration
    const config = vscode.workspace.getConfiguration('cronus');
    const serverUrl = config.get<string>('serverUrl', 'http://localhost:9000');
    const autoConnect = config.get<boolean>('autoConnect', true);

    // Initialize Cronus client
    cronusClient = new CronusClient({
        baseUrl: serverUrl,
        reconnectInterval: config.get<number>('reconnectInterval', 2000),
    });

    // Auto-connect if enabled
    if (autoConnect) {
        cronusClient.start();
    }

    // Setup event listeners
    setupClientListeners(cronusClient);

    // Register commands
    context.subscriptions.push(
        vscode.commands.registerCommand('cronus.openDebugPanel', () => {
            CronusDebugPanel.createOrShow(context.extensionUri, cronusClient!);
        })
    );

    context.subscriptions.push(
        vscode.commands.registerCommand('cronus.reconnect', () => {
            cronusClient?.start();
            vscode.window.showInformationMessage('Reconnecting to Cronus server...');
        })
    );

    context.subscriptions.push(
        vscode.commands.registerCommand('cronus.disconnect', () => {
            cronusClient?.stop();
            vscode.window.showInformationMessage('Disconnected from Cronus server');
        })
    );

    context.subscriptions.push(
        vscode.commands.registerCommand('cronus.clearLogs', () => {
            // Send message to webview to clear logs
            CronusDebugPanel.currentPanel?.postMessage({
                type: 'clearLogs',
            });
        })
    );

    // Auto-open debug panel if configured
    if (config.get<boolean>('autoOpenPanel', false)) {
        vscode.commands.executeCommand('cronus.openDebugPanel');
    }

    // Watch for configuration changes
    context.subscriptions.push(
        vscode.workspace.onDidChangeConfiguration((e) => {
            if (e.affectsConfiguration('cronus.serverUrl')) {
                const newUrl = vscode.workspace.getConfiguration('cronus').get<string>('serverUrl');
                vscode.window.showInformationMessage(
                    `Cronus server URL changed to ${newUrl}. Please reload the window.`,
                    'Reload'
                ).then((selection) => {
                    if (selection === 'Reload') {
                        vscode.commands.executeCommand('workbench.action.reloadWindow');
                    }
                });
            }
        })
    );
}

function setupClientListeners(client: CronusClient) {
    client.on('connected', () => {
        vscode.window.showInformationMessage('Connected to Cronus server');
    });

    client.on('disconnected', () => {
        vscode.window.showWarningMessage('Disconnected from Cronus server');
    });

    client.on('error', (error: any) => {
        console.error('Cronus client error:', error);
    });

    // Forward events to webview if panel is open
    client.on('message', (data: any) => {
        CronusDebugPanel.currentPanel?.postMessage({
            type: 'message',
            data,
        });
    });

    client.on('log', (data: any) => {
        CronusDebugPanel.currentPanel?.postMessage({
            type: 'log',
            data,
        });
    });

    client.on('agent-status', (data: any) => {
        CronusDebugPanel.currentPanel?.postMessage({
            type: 'agentStatus',
            data,
        });
    });

    client.on('connection-status-change', (status: any) => {
        CronusDebugPanel.currentPanel?.postMessage({
            type: 'connectionStatus',
            data: status,
        });
    });
}

export function deactivate() {
    cronusClient?.stop();
}
