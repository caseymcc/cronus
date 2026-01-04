import { useState, useEffect } from 'react';
import { CronusClient, FileTree, FileNode } from '@cronus/shared';

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
export function useFileTree(options: UseFileTreeOptions): UseFileTreeResult {
    const [fileTree, setFileTree] = useState<FileTree | null>(null);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState<Error | null>(null);

    const fetchFileTree = async () => {
        if (!options.client) {
            return;
        }

        setLoading(true);
        setError(null);

        try {
            const tree = await options.client.fetchSourceMap();
            setFileTree(tree);
        } catch (err) {
            setError(err instanceof Error ? err : new Error('Failed to fetch file tree'));
        } finally {
            setLoading(false);
        }
    };

    // Auto-fetch on mount and when client changes
    useEffect(() => {
        if (options.autoFetch !== false && options.client) {
            fetchFileTree();
        }
    }, [options.client, options.autoFetch]);

    // Listen for directory updates
    useEffect(() => {
        if (!options.client) {
            return;
        }

        const handleDirectoryUpdate = (tree: FileTree) => {
            setFileTree(tree);
        };

        options.client.on('directory-update', handleDirectoryUpdate);

        return () => {
            options.client?.off('directory-update', handleDirectoryUpdate);
        };
    }, [options.client]);

    return {
        fileTree,
        loading,
        error,
        refresh: fetchFileTree,
    };
}
