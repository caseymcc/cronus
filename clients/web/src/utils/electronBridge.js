/**
 * Electron Bridge Utility
 * 
 * Provides a unified interface for both Electron and browser environments.
 * In Electron, uses the preload API. In browser, uses standard fetch/SSE.
 */

export const isElectronEnvironment = () => {
  return typeof window !== 'undefined' && window.cronusAPI !== undefined;
};

export const getElectronAPI = () => {
  if (isElectronEnvironment()) {
    return window.cronusAPI;
  }
  return null;
};

/**
 * Get the appropriate API base URL based on environment
 */
export const getApiBaseUrl = () => {
  // In Electron, the backend handles connections
  if (isElectronEnvironment()) {
    return null; // No direct API calls needed
  }
  
  // In browser, use the configured proxy or default
  return process.env.REACT_APP_API_URL || 'http://localhost:9000';
};

/**
 * Initialize connection based on environment
 */
export const initializeConnection = async () => {
  const electronAPI = getElectronAPI();
  
  if (electronAPI) {
    // In Electron, connection is managed by the main process
    return await electronAPI.getConnectionStatus();
  }
  
  // In browser, return default disconnected state
  // The web app will manage its own SSE connection
  return { connected: false, serverUrl: getApiBaseUrl() };
};

/**
 * Subscribe to log events
 */
export const subscribeToLogs = (callback) => {
  const electronAPI = getElectronAPI();
  
  if (electronAPI) {
    // In Electron, subscribe to IPC events
    return electronAPI.onLog(callback);
  }
  
  // In browser, this will be handled by SSE in the component
  return () => {}; // No-op cleanup
};

/**
 * Subscribe to connection events
 */
export const subscribeToConnectionEvents = (onConnected, onDisconnected, onError) => {
  const electronAPI = getElectronAPI();
  
  if (electronAPI) {
    const unsubConnected = electronAPI.onConnected(onConnected);
    const unsubDisconnected = electronAPI.onDisconnected(onDisconnected);
    const unsubError = electronAPI.onError(onError);
    
    return () => {
      unsubConnected();
      unsubDisconnected();
      unsubError();
    };
  }
  
  // In browser, connection events are handled by SSE
  return () => {}; // No-op cleanup
};

/**
 * Fetch logs (historical)
 */
export const fetchLogs = async (limit, levelFilter) => {
  const electronAPI = getElectronAPI();
  
  if (electronAPI) {
    const result = await electronAPI.getLogs(limit, levelFilter);
    return result.success ? result.logs : [];
  }
  
  // In browser, fetch from API
  const apiUrl = getApiBaseUrl();
  if (!apiUrl) return [];
  
  try {
    const params = new URLSearchParams();
    if (limit) params.append('limit', limit.toString());
    if (levelFilter) params.append('level', levelFilter);
    
    const response = await fetch(`${apiUrl}/api/logs?${params}`);
    if (response.ok) {
      return await response.json();
    }
  } catch (error) {
    console.error('Error fetching logs:', error);
  }
  
  return [];
};
