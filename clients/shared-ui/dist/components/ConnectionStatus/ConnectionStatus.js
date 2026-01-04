"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.ConnectionStatusComponent = void 0;
const react_1 = __importDefault(require("react"));
require("./ConnectionStatus.css");
const ConnectionStatusComponent = ({ status, reconnecting = false, className = '', }) => {
    const getStatusText = () => {
        if (reconnecting) {
            return 'Reconnecting...';
        }
        return status.connected ? 'Connected' : 'Disconnected';
    };
    const getStatusClass = () => {
        if (reconnecting) {
            return 'status-reconnecting';
        }
        return status.connected ? 'status-connected' : 'status-disconnected';
    };
    const formatLatency = () => {
        if (!status.latency) {
            return null;
        }
        return `${status.latency}ms`;
    };
    return (react_1.default.createElement("div", { className: `connection-status ${getStatusClass()} ${className}` },
        react_1.default.createElement("div", { className: "status-indicator" },
            react_1.default.createElement("span", { className: "status-dot" }),
            react_1.default.createElement("span", { className: "status-text" }, getStatusText())),
        status.latency && (react_1.default.createElement("div", { className: "status-latency" },
            react_1.default.createElement("span", { className: "latency-label" }, "Latency:"),
            react_1.default.createElement("span", { className: "latency-value" }, formatLatency()))),
        reconnecting && status.reconnectAttempts && (react_1.default.createElement("div", { className: "status-attempts" },
            "Attempt ",
            status.reconnectAttempts))));
};
exports.ConnectionStatusComponent = ConnectionStatusComponent;
//# sourceMappingURL=ConnectionStatus.js.map