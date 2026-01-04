import { useState, useEffect, useCallback } from 'react';
import { CronusClient, Message } from '@cronus/shared';

export interface UseMessagesOptions {
    client: CronusClient | null;
    sessionId?: string;
}

export interface UseMessagesResult {
    messages: Message[];
    sendMessage: (content: string) => Promise<void>;
    clearMessages: () => void;
    loading: boolean;
}

/**
 * React hook for managing chat messages
 */
export function useMessages(options: UseMessagesOptions): UseMessagesResult {
    const [messages, setMessages] = useState<Message[]>([]);
    const [loading, setLoading] = useState(false);

    // Listen for incoming messages
    useEffect(() => {
        if (!options.client) {
            return;
        }

        const handleMessage = (data: any) => {
            const message: Message = {
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

    const sendMessage = useCallback(
        async (content: string) => {
            if (!options.client) {
                throw new Error('Client not initialized');
            }

            // Add user message immediately
            const userMessage: Message = {
                id: `user-${Date.now()}`,
                role: 'user',
                content,
                timestamp: Date.now(),
            };
            setMessages((prev) => [...prev, userMessage]);

            setLoading(true);
            try {
                await options.client.sendMessage(content, options.sessionId);
            } catch (err) {
                // Add error message
                const errorMessage: Message = {
                    id: `error-${Date.now()}`,
                    role: 'system',
                    content: `Error: ${err instanceof Error ? err.message : 'Unknown error'}`,
                    timestamp: Date.now(),
                };
                setMessages((prev) => [...prev, errorMessage]);
            } finally {
                setLoading(false);
            }
        },
        [options.client, options.sessionId]
    );

    const clearMessages = useCallback(() => {
        setMessages([]);
    }, []);

    return {
        messages,
        sendMessage,
        clearMessages,
        loading,
    };
}
