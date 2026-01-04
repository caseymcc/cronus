// Renderer process script for Cronus Debug Client

// State
let logs = [];
let filteredLogs = [];
let activeLevels = new Set(['debug', 'info', 'warning', 'error']);
let searchQuery = '';
let autoScroll = true;

// DOM elements
const statusIndicator = document.getElementById('statusIndicator');
const statusText = document.getElementById('statusText');
const serverUrlInput = document.getElementById('serverUrl');
const connectBtn = document.getElementById('connectBtn');
const disconnectBtn = document.getElementById('disconnectBtn');
const logContainer = document.getElementById('logContainer');
const searchInput = document.getElementById('searchInput');
const autoScrollCheckbox = document.getElementById('autoScrollCheckbox');
const clearLogsBtn = document.getElementById('clearLogsBtn');
const totalLogsSpan = document.getElementById('totalLogs');
const filteredLogsSpan = document.getElementById('filteredLogs');
const levelBtns = document.querySelectorAll('.level-btn');

// Initialize
async function init() {
    setupEventListeners();
    await checkConnectionStatus();
    loadHistoricalLogs();
}

function setupEventListeners() {
    // Connection controls
    connectBtn.addEventListener('click', handleConnect);
    disconnectBtn.addEventListener('click', handleDisconnect);
    
    // Filter controls
    searchInput.addEventListener('input', handleSearchChange);
    autoScrollCheckbox.addEventListener('change', handleAutoScrollChange);
    clearLogsBtn.addEventListener('click', handleClearLogs);
    
    // Level filter buttons
    levelBtns.forEach(btn => {
        btn.addEventListener('click', handleLevelToggle);
    });
    
    // Cronus API event listeners
    window.cronusAPI.onConnected(() => {
        updateConnectionStatus(true);
        loadHistoricalLogs();
    });
    
    window.cronusAPI.onDisconnected(() => {
        updateConnectionStatus(false);
    });
    
    window.cronusAPI.onError((error) => {
        console.error('Cronus error:', error);
        addSystemLog('error', `Connection error: ${error}`);
    });
    
    window.cronusAPI.onLog((log) => {
        addLog(log);
    });
}

async function checkConnectionStatus() {
    const status = await window.cronusAPI.getConnectionStatus();
    updateConnectionStatus(status.connected);
    if (status.serverUrl && status.serverUrl !== 'unknown') {
        serverUrlInput.value = status.serverUrl;
    }
}

async function loadHistoricalLogs() {
    const result = await window.cronusAPI.getLogs(100);
    if (result.success && result.logs) {
        // Clear existing logs and load historical ones
        logs = result.logs;
        applyFilters();
        renderLogs();
    }
}

function updateConnectionStatus(connected) {
    if (connected) {
        statusIndicator.className = 'status-indicator connected';
        statusText.textContent = 'Connected';
        connectBtn.disabled = true;
        disconnectBtn.disabled = false;
        serverUrlInput.disabled = true;
    } else {
        statusIndicator.className = 'status-indicator disconnected';
        statusText.textContent = 'Disconnected';
        connectBtn.disabled = false;
        disconnectBtn.disabled = true;
        serverUrlInput.disabled = false;
    }
}

async function handleConnect() {
    const url = serverUrlInput.value.trim();
    if (!url) {
        alert('Please enter a server URL');
        return;
    }
    
    // Update server URL if changed
    await window.cronusAPI.setServerUrl(url);
    
    const result = await window.cronusAPI.connect();
    if (!result.success) {
        alert(`Failed to connect: ${result.error}`);
    }
}

async function handleDisconnect() {
    const result = await window.cronusAPI.disconnect();
    if (!result.success) {
        alert(`Failed to disconnect: ${result.error}`);
    }
}

function handleSearchChange(e) {
    searchQuery = e.target.value.toLowerCase();
    applyFilters();
    renderLogs();
}

function handleAutoScrollChange(e) {
    autoScroll = e.target.checked;
    if (autoScroll) {
        scrollToBottom();
    }
}

function handleClearLogs() {
    logs = [];
    applyFilters();
    renderLogs();
}

function handleLevelToggle(e) {
    const level = e.target.dataset.level;
    if (activeLevels.has(level)) {
        activeLevels.delete(level);
        e.target.classList.remove('active');
    } else {
        activeLevels.add(level);
        e.target.classList.add('active');
    }
    applyFilters();
    renderLogs();
}

function addLog(log) {
    logs.push(log);
    
    // Limit to last 500 logs to prevent memory issues
    if (logs.length > 500) {
        logs.shift();
    }
    
    applyFilters();
    renderLogs();
}

function addSystemLog(level, message) {
    addLog({
        timestamp: new Date().toLocaleString(),
        level: level,
        message: message
    });
}

function applyFilters() {
    filteredLogs = logs.filter(log => {
        // Level filter
        if (!activeLevels.has(log.level)) {
            return false;
        }
        
        // Search filter
        if (searchQuery && !log.message.toLowerCase().includes(searchQuery)) {
            return false;
        }
        
        return true;
    });
    
    updateStats();
}

function updateStats() {
    totalLogsSpan.textContent = logs.length;
    filteredLogsSpan.textContent = filteredLogs.length;
}

function renderLogs() {
    if (filteredLogs.length === 0) {
        logContainer.innerHTML = `
            <div class="log-empty">
                <p>No logs to display</p>
                <p class="log-empty-hint">${logs.length > 0 ? 'Try adjusting filters' : 'Connect to Cronus server to see logs'}</p>
            </div>
        `;
        return;
    }
    
    logContainer.innerHTML = filteredLogs.map(log => `
        <div class="log-entry ${log.level}">
            <span class="log-timestamp">${log.timestamp}</span>
            <span class="log-level ${log.level}">[${log.level.toUpperCase()}]</span>
            <span class="log-message">${escapeHtml(log.message)}</span>
        </div>
    `).join('');
    
    if (autoScroll) {
        scrollToBottom();
    }
}

function scrollToBottom() {
    logContainer.scrollTop = logContainer.scrollHeight;
}

function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Start the app
init();
