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
export declare function useMessages(options: UseMessagesOptions): UseMessagesResult;
//# sourceMappingURL=useMessages.d.ts.map