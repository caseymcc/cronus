import * as vscode from 'vscode';
import { CronusClient } from '@cronus/shared';

export class CronusDebugPanel {
    public static currentPanel: CronusDebugPanel | undefined;
    private readonly _panel: vscode.WebviewPanel;
    private readonly _extensionUri: vscode.Uri;
    private _disposables: vscode.Disposable[] = [];
    private _client: CronusClient;

    public static createOrShow(extensionUri: vscode.Uri, client: CronusClient) {
        const column = vscode.window.activeTextEditor
            ? vscode.window.activeTextEditor.viewColumn
            : undefined;

        // If we already have a panel, show it
        if (CronusDebugPanel.currentPanel) {
            CronusDebugPanel.currentPanel._panel.reveal(column);
            return;
        }

        // Otherwise, create a new panel
        const panel = vscode.window.createWebviewPanel(
            'cronusDebug',
            'Cronus Debug',
            column || vscode.ViewColumn.One,
            {
                enableScripts: true,
                retainContextWhenHidden: true,
                localResourceRoots: [
                    vscode.Uri.joinPath(extensionUri, 'webview', 'build'),
                ],
            }
        );

        CronusDebugPanel.currentPanel = new CronusDebugPanel(panel, extensionUri, client);
    }

    private constructor(panel: vscode.WebviewPanel, extensionUri: vscode.Uri, client: CronusClient) {
        this._panel = panel;
        this._extensionUri = extensionUri;
        this._client = client;

        // Set the webview's initial html content
        this._update();

        // Listen for when the panel is disposed
        this._panel.onDidDispose(() => this.dispose(), null, this._disposables);

        // Handle messages from the webview
        this._panel.webview.onDidReceiveMessage(
            (message) => {
                switch (message.type) {
                    case 'sendMessage':
                        this._client.sendMessage(message.content, message.sessionId);
                        break;
                    case 'fetchFileTree':
                        this._client.fetchSourceMap().then((tree) => {
                            this._panel.webview.postMessage({
                                type: 'fileTree',
                                data: tree,
                            });
                        });
                        break;
                    case 'fetchFile':
                        this._client.fetchFileContent(message.path).then((file) => {
                            this._panel.webview.postMessage({
                                type: 'fileContent',
                                data: file,
                            });
                        });
                        break;
                    case 'connect':
                        this._client.start();
                        break;
                    case 'disconnect':
                        this._client.stop();
                        break;
                    case 'ready':
                        // Webview is ready, send initial state
                        this._panel.webview.postMessage({
                            type: 'connectionStatus',
                            data: this._client.getConnectionStatus(),
                        });
                        break;
                }
            },
            null,
            this._disposables
        );
    }

    public postMessage(message: any) {
        this._panel.webview.postMessage(message);
    }

    public dispose() {
        CronusDebugPanel.currentPanel = undefined;

        this._panel.dispose();

        while (this._disposables.length) {
            const disposable = this._disposables.pop();
            if (disposable) {
                disposable.dispose();
            }
        }
    }

    private _update() {
        const webview = this._panel.webview;
        this._panel.webview.html = this._getHtmlForWebview(webview);
    }

    private _getHtmlForWebview(webview: vscode.Webview) {
        const config = vscode.workspace.getConfiguration('cronus');
        const serverUrl = config.get<string>('serverUrl', 'http://localhost:9000');

        return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cronus Debug</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }
        body {
            padding: 0;
            color: var(--vscode-foreground);
            background-color: var(--vscode-editor-background);
            font-family: var(--vscode-font-family);
            height: 100vh;
            display: flex;
            flex-direction: column;
        }
        .header {
            padding: 16px;
            background: var(--vscode-editorGroupHeader-tabsBackground);
            border-bottom: 1px solid var(--vscode-panel-border);
        }
        h1 {
            font-size: 18px;
            margin-bottom: 12px;
        }
        .status {
            padding: 10px;
            margin-bottom: 12px;
            border-radius: 4px;
            background: var(--vscode-editor-inactiveSelectionBackground);
            font-size: 13px;
        }
        .connected {
            border-left: 4px solid #4caf50;
        }
        .disconnected {
            border-left: 4px solid #f44336;
        }
        .reconnecting {
            border-left: 4px solid #ff9800;
        }
        .controls {
            display: flex;
            gap: 8px;
        }
        button {
            background: var(--vscode-button-background);
            color: var(--vscode-button-foreground);
            border: none;
            padding: 6px 14px;
            cursor: pointer;
            border-radius: 2px;
            font-size: 12px;
        }
        button:hover {
            background: var(--vscode-button-hoverBackground);
        }
        button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }
        .content {
            flex: 1;
            display: flex;
            flex-direction: column;
            overflow: hidden;
        }
        .log-section {
            flex: 1;
            display: flex;
            flex-direction: column;
            overflow: hidden;
        }
        .log-header {
            padding: 8px 16px;
            background: var(--vscode-editorGroupHeader-tabsBackground);
            border-bottom: 1px solid var(--vscode-panel-border);
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .log-title {
            font-weight: 600;
            font-size: 13px;
        }
        .log-filters {
            display: flex;
            gap: 8px;
            align-items: center;
        }
        .level-btn {
            width: 24px;
            height: 24px;
            border: 1px solid var(--vscode-button-border);
            background: var(--vscode-button-secondaryBackground);
            color: var(--vscode-button-secondaryForeground);
            cursor: pointer;
            border-radius: 3px;
            font-size: 11px;
            font-weight: 600;
            opacity: 0.5;
            padding: 0;
        }
        .level-btn.active {
            opacity: 1;
            border-width: 2px;
        }
        .level-btn.debug.active {
            border-color: #858585;
            color: #858585;
        }
        .level-btn.info.active {
            border-color: #4ec9b0;
            color: #4ec9b0;
        }
        .level-btn.warning.active {
            border-color: #dcdcaa;
            color: #dcdcaa;
        }
        .level-btn.error.active {
            border-color: #f48771;
            color: #f48771;
        }
        .log-container {
            flex: 1;
            overflow-y: auto;
            padding: 8px 16px;
            font-family: var(--vscode-editor-font-family, monospace);
            font-size: 12px;
            line-height: 1.5;
        }
        .log-entry {
            padding: 2px 0;
            word-wrap: break-word;
            white-space: pre-wrap;
        }
        .log-timestamp {
            color: var(--vscode-descriptionForeground);
            margin-right: 8px;
            font-size: 11px;
        }
        .log-level {
            margin-right: 8px;
            font-weight: 600;
            font-size: 11px;
        }
        .log-debug .log-level { color: #858585; }
        .log-info .log-level { color: #4ec9b0; }
        .log-warning .log-level { color: #dcdcaa; }
        .log-error .log-level { color: #f48771; }
        .log-error .log-message { color: #f48771; }
        .log-message {
            color: var(--vscode-foreground);
        }
        .log-empty {
            color: var(--vscode-descriptionForeground);
            text-align: center;
            padding: 40px;
            font-style: italic;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>Cronus Debug Panel</h1>
        
        <div id="status" class="status disconnected">
            <strong>Status:</strong> <span id="statusText">Disconnected</span><br>
            <strong>Server:</strong> ${serverUrl}
        </div>

        <div class="controls">
            <button id="connectBtn" onclick="connect()">Connect</button>
            <button id="disconnectBtn" onclick="disconnect()" disabled>Disconnect</button>
        </div>
    </div>

    <div class="content">
        <div class="log-section">
            <div class="log-header">
                <div class="log-title">Logs (<span id="logCount">0</span>)</div>
                <div class="log-filters">
                    <button class="level-btn debug active" id="levelDebug" onclick="toggleLevel('debug')" title="Debug">D</button>
                    <button class="level-btn info active" id="levelInfo" onclick="toggleLevel('info')" title="Info">I</button>
                    <button class="level-btn warning active" id="levelWarning" onclick="toggleLevel('warning')" title="Warning">W</button>
                    <button class="level-btn error active" id="levelError" onclick="toggleLevel('error')" title="Error">E</button>
                    <button onclick="clearLogs()">Clear</button>
                </div>
            </div>
            <div id="logs" class="log-container">
                <div class="log-empty">Waiting for connection...</div>
            </div>
        </div>
    </div>

    <script>
        const vscode = acquireVsCodeApi();
        let logs = [];
        let levelFilters = new Set(['debug', 'info', 'warning', 'error']);
        let connected = false;
        
        // Handle messages from extension
        window.addEventListener('message', event => {
            const message = event.data;
            
            switch (message.type) {
                case 'connectionStatus':
                    updateConnectionStatus(message.data);
                    break;
                case 'log':
                    addLog(message.data);
                    break;
                case 'clearLogs':
                    logs = [];
                    renderLogs();
                    break;
            }
        });

        function updateConnectionStatus(status) {
            connected = status.connected;
            const statusDiv = document.getElementById('status');
            const statusText = document.getElementById('statusText');
            const connectBtn = document.getElementById('connectBtn');
            const disconnectBtn = document.getElementById('disconnectBtn');
            
            if (status.connected) {
                statusDiv.className = 'status connected';
                statusText.textContent = 'Connected';
                if (status.latency) {
                    statusText.textContent += ' (' + status.latency + 'ms)';
                }
                connectBtn.disabled = true;
                disconnectBtn.disabled = false;
            } else {
                statusDiv.className = 'status disconnected';
                statusText.textContent = 'Disconnected';
                connectBtn.disabled = false;
                disconnectBtn.disabled = true;
            }
        }

        function addLog(entry) {
            logs.push({
                timestamp: entry.timestamp || new Date().toLocaleTimeString('en-US', { hour12: false }),
                level: entry.level || 'info',
                message: entry.message || ''
            });
            
            // Keep only last 500 logs
            if (logs.length > 500) {
                logs.shift();
            }
            
            renderLogs();
        }

        function renderLogs() {
            const container = document.getElementById('logs');
            const filtered = logs.filter(log => levelFilters.has(log.level));
            
            document.getElementById('logCount').textContent = filtered.length;
            
            if (filtered.length === 0) {
                container.innerHTML = '<div class="log-empty">' + 
                    (logs.length === 0 ? 'No logs yet' : 'No logs matching filter') + 
                    '</div>';
                return;
            }
            
            container.innerHTML = filtered.map(log => {
                return '<div class="log-entry log-' + log.level + '">' +
                    '<span class="log-timestamp">' + log.timestamp + '</span>' +
                    '<span class="log-level">[' + log.level.toUpperCase() + ']</span>' +
                    '<span class="log-message">' + escapeHtml(log.message) + '</span>' +
                    '</div>';
            }).join('');
            
            // Auto-scroll to bottom
            container.scrollTop = container.scrollHeight;
        }

        function toggleLevel(level) {
            if (levelFilters.has(level)) {
                levelFilters.delete(level);
                document.getElementById('level' + capitalize(level)).classList.remove('active');
            } else {
                levelFilters.add(level);
                document.getElementById('level' + capitalize(level)).classList.add('active');
            }
            renderLogs();
        }

        function connect() {
            vscode.postMessage({ type: 'connect' });
            addLog({ level: 'info', message: 'Connecting to server...' });
        }

        function disconnect() {
            vscode.postMessage({ type: 'disconnect' });
            addLog({ level: 'info', message: 'Disconnecting from server...' });
        }

        function clearLogs() {
            logs = [];
            renderLogs();
        }

        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }

        function capitalize(str) {
            return str.charAt(0).toUpperCase() + str.slice(1);
        }

        // Notify extension that webview is ready
        vscode.postMessage({ type: 'ready' });
        
        // Initial message
        addLog({ level: 'info', message: 'Debug panel initialized' });
    </script>
</body>
</html>`;
    }
}
