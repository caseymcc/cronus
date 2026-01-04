"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useFileTree = useFileTree;
const react_1 = require("react");
/**
 * React hook for managing file tree data
 */
function useFileTree(options) {
    const [fileTree, setFileTree] = (0, react_1.useState)(null);
    const [loading, setLoading] = (0, react_1.useState)(false);
    const [error, setError] = (0, react_1.useState)(null);
    const fetchFileTree = async () => {
        if (!options.client) {
            return;
        }
        setLoading(true);
        setError(null);
        try {
            const tree = await options.client.fetchSourceMap();
            setFileTree(tree);
        }
        catch (err) {
            setError(err instanceof Error ? err : new Error('Failed to fetch file tree'));
        }
        finally {
            setLoading(false);
        }
    };
    // Auto-fetch on mount and when client changes
    (0, react_1.useEffect)(() => {
        if (options.autoFetch !== false && options.client) {
            fetchFileTree();
        }
    }, [options.client, options.autoFetch]);
    // Listen for directory updates
    (0, react_1.useEffect)(() => {
        if (!options.client) {
            return;
        }
        const handleDirectoryUpdate = (tree) => {
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
//# sourceMappingURL=useFileTree.js.map