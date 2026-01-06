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
const material_1 = require("@mui/material");
const icons_material_1 = require("@mui/icons-material");
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
    const handleToggleLevels = (event, newLevels) => {
        if (newLevels.length > 0) {
            setSelectedLevels(new Set(newLevels));
        }
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
    const getLevelColor = (level) => {
        switch (level) {
            case 'debug':
                return 'default';
            case 'info':
                return 'info';
            case 'warning':
                return 'warning';
            case 'error':
                return 'error';
            default:
                return 'default';
        }
    };
    const getLevelIcon = (level) => {
        switch (level) {
            case 'debug':
                return react_1.default.createElement(icons_material_1.BugReport, { fontSize: "small" });
            case 'info':
                return react_1.default.createElement(icons_material_1.Info, { fontSize: "small" });
            case 'warning':
                return react_1.default.createElement(icons_material_1.Warning, { fontSize: "small" });
            case 'error':
                return react_1.default.createElement(icons_material_1.Error, { fontSize: "small" });
            default:
                return react_1.default.createElement(icons_material_1.Info, { fontSize: "small" });
        }
    };
    const formatTimestamp = (timestamp) => {
        if (typeof timestamp === 'string') {
            return timestamp;
        }
        const date = new Date(timestamp);
        return date.toLocaleTimeString('en-US', { hour12: false });
    };
    return (react_1.default.createElement(material_1.Paper, { className: className, sx: {
            display: 'flex',
            flexDirection: 'column',
            height: '100%',
            overflow: 'hidden',
        } },
        react_1.default.createElement(material_1.Box, { sx: {
                p: 1.5,
                borderBottom: 1,
                borderColor: 'divider',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'space-between',
                gap: 2,
            } },
            react_1.default.createElement(material_1.Typography, { variant: "h6", sx: { fontSize: '1rem' } },
                "Logs (",
                filteredLogs.length,
                ")"),
            react_1.default.createElement(material_1.Box, { sx: { display: 'flex', alignItems: 'center', gap: 1, flex: 1 } },
                react_1.default.createElement(material_1.TextField, { size: "small", placeholder: "Filter logs...", value: filter, onChange: (e) => setFilter(e.target.value), sx: { flex: 1, maxWidth: 300 } }),
                react_1.default.createElement(material_1.ToggleButtonGroup, { value: Array.from(selectedLevels), onChange: handleToggleLevels, size: "small", "aria-label": "log level filter" },
                    react_1.default.createElement(material_1.ToggleButton, { value: "debug", "aria-label": "debug" },
                        react_1.default.createElement(icons_material_1.BugReport, { fontSize: "small" })),
                    react_1.default.createElement(material_1.ToggleButton, { value: "info", "aria-label": "info" },
                        react_1.default.createElement(icons_material_1.Info, { fontSize: "small" })),
                    react_1.default.createElement(material_1.ToggleButton, { value: "warning", "aria-label": "warning" },
                        react_1.default.createElement(icons_material_1.Warning, { fontSize: "small" })),
                    react_1.default.createElement(material_1.ToggleButton, { value: "error", "aria-label": "error" },
                        react_1.default.createElement(icons_material_1.Error, { fontSize: "small" }))),
                react_1.default.createElement(material_1.Button, { variant: "outlined", size: "small", startIcon: react_1.default.createElement(icons_material_1.Delete, null), onClick: handleClearLogs }, "Clear"))),
        react_1.default.createElement(material_1.Box, { ref: logContainerRef, sx: {
                flex: 1,
                overflow: 'auto',
                p: 1,
                fontFamily: 'monospace',
                fontSize: '0.85rem',
            } },
            filteredLogs.length === 0 ? (react_1.default.createElement(material_1.Typography, { variant: "body2", color: "text.secondary", sx: { textAlign: 'center', mt: 4 } }, "No logs to display")) : (filteredLogs.map((log, index) => (react_1.default.createElement(material_1.Box, { key: index, sx: {
                    display: 'flex',
                    alignItems: 'flex-start',
                    gap: 1,
                    mb: 0.5,
                    pb: 0.5,
                    borderBottom: '1px solid',
                    borderColor: 'divider',
                } },
                showTimestamp && (react_1.default.createElement(material_1.Typography, { component: "span", sx: {
                        color: 'text.secondary',
                        fontSize: '0.75rem',
                        minWidth: '80px',
                    } }, formatTimestamp(log.timestamp))),
                react_1.default.createElement(material_1.Chip, { icon: getLevelIcon(log.level), label: log.level.toUpperCase(), color: getLevelColor(log.level), size: "small", sx: { minWidth: '90px', fontSize: '0.7rem' } }),
                react_1.default.createElement(material_1.Typography, { component: "span", sx: {
                        flex: 1,
                        wordBreak: 'break-word',
                        fontSize: '0.85rem',
                    } },
                    log.message,
                    log.source && (react_1.default.createElement(material_1.Typography, { component: "span", color: "text.secondary", sx: { ml: 1, fontSize: '0.75rem' } },
                        "(",
                        log.source,
                        ")"))))))),
            react_1.default.createElement("div", { ref: logsEndRef }))));
};
exports.LogViewer = LogViewer;
//# sourceMappingURL=LogViewer.js.map