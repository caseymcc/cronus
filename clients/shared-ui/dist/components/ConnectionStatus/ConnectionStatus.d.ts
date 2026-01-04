import React from 'react';
import { ConnectionStatus } from '@cronus/shared';
import './ConnectionStatus.css';
export interface ConnectionStatusProps {
    status: ConnectionStatus;
    reconnecting?: boolean;
    className?: string;
}
export declare const ConnectionStatusComponent: React.FC<ConnectionStatusProps>;
//# sourceMappingURL=ConnectionStatus.d.ts.map