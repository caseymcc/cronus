"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.LogViewer = void 0;
const react_1 = __importStar(require("react"));
require("./LogViewer.css");
const LogViewer = ({ client, maxLogs = 500, autoScroll = true, showTimestamp = true, levelFilter = [], className = '', }) => {
    const [logs, setLogs] = (0, react_1.useState)([]);
    const [filter, setFilter] = (0, react_1.useState)('');
    const [selectedLevels, setSelectedLevels] = (0, react_1.useState)(new Set(levelFilter.length > 0 ? levelFilter : ['debug', 'info', 'warning', 'error']));
    const logsEndRef = (0, react_1.useRef)(null);
    const logContainerRef = (0, react_1.useRef)(null);
    // Load initial logs
    (0, react_1.useEffect)(() => {
        if (!client)
            return;
        const loadInitialLogs = async () => {
            try {
                const historicalLogs = await client.fetchLogs(100);
                setLogs(historicalLogs);
            }
            catch (err) {
                console.error('Failed to load initial logs:', err);
            }
        };
        loadInitialLogs();
    }, [client]);
    // Listen for new logs
    (0, react_1.useEffect)(() => {
        if (!client)
            return;
        const handleLog = (data) => {
            const logEntry = {
                timestamp: data.timestamp || Date.now(),
                level: data.level || 'info',
                message: data.message || '',
                source: data.source,
                metadata: data.metadata,
            };
            setLogs((prevLogs) => {
                const newLogs = [...prevLogs, logEntry];
                // Trim to max logs
                if (newLogs.length > maxLogs) {
                    return newLogs.slice(newLogs.length - maxLogs);
                }
                return newLogs;
            });
        };
        client.on('log', handleLog);
        return () => {
            client.off('log', handleLog);
        };
    }, [client, maxLogs]);
    // Auto-scroll to bottom
    (0, react_1.useEffect)(() => {
        if (autoScroll && logsEndRef.current) {
            logsEndRef.current.scrollIntoView({ behavior: 'smooth' });
        }
    }, [logs, autoScroll]);
    const handleClearLogs = () => {
        setLogs([]);
    };
    const handleToggleLevel = (level) => {
        setSelectedLevels((prev) => {
            const newSet = new Set(prev);
            if (newSet.has(level)) {
                newSet.delete(level);
            }
            else {
                newSet.add(level);
            }
            return newSet;
        });
    };
    const filteredLogs = logs.filter((log) => {
        // Filter by level
        if (!selectedLevels.has(log.level)) {
            return false;
        }
        // Filter by search text
        if (filter && !log.message.toLowerCase().includes(filter.toLowerCase())) {
            return false;
        }
        return true;
    });
    const getLevelClass = (level) => {
        return `log-level-${level}`;
    };
    const formatTimestamp = (timestamp) => {
        if (typeof timestamp === 'string') {
            return timestamp;
        }
        const date = new Date(timestamp);
        return date.toLocaleTimeString('en-US', { hour12: false });
    };
    return (react_1.default.createElement("div", { className: `log-viewer ${className}` },
        react_1.default.createElement("div", { className: "log-viewer-header" },
            react_1.default.createElement("div", { className: "log-viewer-title" },
                "Logs (",
                filteredLogs.length,
                ")"),
            react_1.default.createElement("div", { className: "log-viewer-controls" },
                react_1.default.createElement("input", { type: "text", className: "log-search", placeholder: "Filter logs...", value: filter, onChange: (e) => setFilter(e.target.value) }),
                react_1.default.createElement("div", { className: "log-level-filters" }, ['debug', 'info', 'warning', 'error'].map((level) => (react_1.default.createElement("button", { key: level, className: `log-level-btn log-level-${level} ${selectedLevels.has(level) ? 'active' : ''}`, onClick: () => handleToggleLevel(level), title: `Toggle ${level} logs` }, level.charAt(0).toUpperCase())))),
                react_1.default.createElement("button", { className: "log-clear-btn", onClick: handleClearLogs, title: "Clear logs" }, "Clear"))),
        react_1.default.createElement("div", { className: "log-viewer-content", ref: logContainerRef },
            filteredLogs.length === 0 ? (react_1.default.createElement("div", { className: "log-empty" }, "No logs to display")) : (filteredLogs.map((log, index) => (react_1.default.createElement("div", { key: index, className: `log-entry ${getLevelClass(log.level)}` },
                showTimestamp && (react_1.default.createElement("span", { className: "log-timestamp" }, formatTimestamp(log.timestamp))),
                react_1.default.createElement("span", { className: "log-level" },
                    "[",
                    log.level.toUpperCase(),
                    "]"),
                react_1.default.createElement("span", { className: "log-message" }, log.message),
                log.source && react_1.default.createElement("span", { className: "log-source" },
                    "(",
                    log.source,
                    ")"))))),
            react_1.default.createElement("div", { ref: logsEndRef }))));
};
exports.LogViewer = LogViewer;
//# sourceMappingURL=LogViewer.js.map