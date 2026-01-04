import React from 'react';
import { CronusClient } from '@cronus/shared';
import './LogViewer.css';
export interface LogViewerProps {
    client: CronusClient | null;
    maxLogs?: number;
    autoScroll?: boolean;
    showTimestamp?: boolean;
    levelFilter?: string[];
    className?: string;
}
export declare const LogViewer: React.FC<LogViewerProps>;
//# sourceMappingURL=LogViewer.d.ts.map