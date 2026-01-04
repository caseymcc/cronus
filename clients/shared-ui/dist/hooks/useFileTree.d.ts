import { CronusClient, FileTree } from '@cronus/shared';
export interface UseFileTreeOptions {
    client: CronusClient | null;
    autoFetch?: boolean;
}
export interface UseFileTreeResult {
    fileTree: FileTree | null;
    loading: boolean;
    error: Error | null;
    refresh: () => Promise<void>;
}
/**
 * React hook for managing file tree data
 */
export declare function useFileTree(options: UseFileTreeOptions): UseFileTreeResult;
//# sourceMappingURL=useFileTree.d.ts.map