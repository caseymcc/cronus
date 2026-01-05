import { useEffect, useState, useCallback, useRef } from 'react';
import { CronusClient } from '@cronus/shared';

/**
 * React hook for managing CronusClient connection
 * @param {string} baseUrl - Base URL of the Cronus server
 * @returns {Object} Client instance and connection state
 */
export function useCronusClient(baseUrl) {
  const [connected, setConnected] = useState(false);
  const [error, setError] = useState(null);
  const clientRef = useRef(null);

  useEffect(() => {
    if (!baseUrl) return;

    // Create client
    const client = new CronusClient({
      baseUrl,
      reconnectInterval: 2000,
      maxReconnectAttempts: 0, // Infinite reconnection
      healthCheckInterval: 30000,
    });

    clientRef.current = client;

    // Set up event handlers
    client.on('connected', () => {
      console.log('CronusClient: Connected to server');
      setConnected(true);
      setError(null);
    });

    client.on('disconnected', () => {
      console.log('CronusClient: Disconnected from server');
      setConnected(false);
    });

    client.on('error', (err) => {
      console.error('CronusClient: Error:', err);
      setError(err.message || 'Connection error');
    });

    client.on('reconnecting', ({ attempt }) => {
      console.log(`CronusClient: Reconnecting (attempt ${attempt})...`);
    });

    // Start the client
    client.start();

    // Cleanup on unmount
    return () => {
      client.stop();
      clientRef.current = null;
    };
  }, [baseUrl]);

  const sendInput = useCallback(async (text) => {
    if (!clientRef.current) {
      throw new Error('Client not initialized');
    }
    return clientRef.current.sendInput(text);
  }, []);

  const getSourceMap = useCallback(async () => {
    if (!clientRef.current) {
      throw new Error('Client not initialized');
    }
    return clientRef.current.getSourceMap();
  }, []);

  const getFile = useCallback(async (path) => {
    if (!clientRef.current) {
      throw new Error('Client not initialized');
    }
    return clientRef.current.getFile(path);
  }, []);

  const getDirectory = useCallback(async (path) => {
    if (!clientRef.current) {
      throw new Error('Client not initialized');
    }
    return clientRef.current.getDirectory(path);
  }, []);

  const getLogs = useCallback(async (options = {}) => {
    if (!clientRef.current) {
      throw new Error('Client not initialized');
    }
    return clientRef.current.getLogs(options);
  }, []);

  return {
    client: clientRef.current,
    connected,
    error,
    sendInput,
    getSourceMap,
    getFile,
    getDirectory,
    getLogs,
  };
}
