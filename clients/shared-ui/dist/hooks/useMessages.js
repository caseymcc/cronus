"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useMessages = useMessages;
const react_1 = require("react");
/**
 * React hook for managing chat messages
 */
function useMessages(options) {
    const [messages, setMessages] = (0, react_1.useState)([]);
    const [loading, setLoading] = (0, react_1.useState)(false);
    // Listen for incoming messages
    (0, react_1.useEffect)(() => {
        if (!options.client) {
            return;
        }
        const handleMessage = (data) => {
            const message = {
                id: data.id || `msg-${Date.now()}`,
                role: data.role || 'assistant',
                content: data.content || data.message || '',
                timestamp: data.timestamp || Date.now(),
                metadata: data.metadata,
            };
            setMessages((prev) => [...prev, message]);
        };
        options.client.on('message', handleMessage);
        return () => {
            options.client?.off('message', handleMessage);
        };
    }, [options.client]);
    const sendMessage = (0, react_1.useCallback)(async (content) => {
        if (!options.client) {
            throw new Error('Client not initialized');
        }
        // Add user message immediately
        const userMessage = {
            id: `user-${Date.now()}`,
            role: 'user',
            content,
            timestamp: Date.now(),
        };
        setMessages((prev) => [...prev, userMessage]);
        setLoading(true);
        try {
            await options.client.sendMessage(content, options.sessionId);
        }
        catch (err) {
            // Add error message
            const errorMessage = {
                id: `error-${Date.now()}`,
                role: 'system',
                content: `Error: ${err instanceof Error ? err.message : 'Unknown error'}`,
                timestamp: Date.now(),
            };
            setMessages((prev) => [...prev, errorMessage]);
        }
        finally {
            setLoading(false);
        }
    }, [options.client, options.sessionId]);
    const clearMessages = (0, react_1.useCallback)(() => {
        setMessages([]);
    }, []);
    return {
        messages,
        sendMessage,
        clearMessages,
        loading,
    };
}
//# sourceMappingURL=useMessages.js.map