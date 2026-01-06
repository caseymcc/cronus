"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.ConnectionStatusComponent = void 0;
const react_1 = __importDefault(require("react"));
const material_1 = require("@mui/material");
const icons_material_1 = require("@mui/icons-material");
const ConnectionStatusComponent = ({ status, reconnecting = false, className = '', }) => {
    const getStatusText = () => {
        if (reconnecting) {
            return 'Reconnecting...';
        }
        return status.connected ? 'Connected' : 'Disconnected';
    };
    const getStatusColor = () => {
        if (reconnecting) {
            return 'warning';
        }
        return status.connected ? 'success' : 'error';
    };
    const formatLatency = () => {
        if (!status.latency) {
            return null;
        }
        return `${status.latency}ms`;
    };
    return (react_1.default.createElement(material_1.Box, { className: className, sx: {
            display: 'flex',
            alignItems: 'center',
            gap: 1,
        } },
        react_1.default.createElement(material_1.Chip, { icon: react_1.default.createElement(icons_material_1.Circle, { sx: { fontSize: 12 } }), label: getStatusText(), color: getStatusColor(), size: "small", variant: "outlined" }),
        status.latency && (react_1.default.createElement(material_1.Typography, { variant: "body2", color: "text.secondary", sx: { fontSize: '0.75rem' } },
            "Latency: ",
            formatLatency())),
        reconnecting && status.reconnectAttempts && (react_1.default.createElement(material_1.Typography, { variant: "body2", color: "warning.main", sx: { fontSize: '0.75rem' } },
            "Attempt ",
            status.reconnectAttempts))));
};
exports.ConnectionStatusComponent = ConnectionStatusComponent;
//# sourceMappingURL=ConnectionStatus.js.map