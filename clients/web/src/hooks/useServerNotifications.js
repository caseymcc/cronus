/**
 * Hook to listen for Cronus server startup notifications
 * 
 * Connects to the notification listener service and receives
 * server startup events via WebSocket.
 */

import { useState, useEffect } from 'react';

const NOTIFICATION_LISTENER_URL = process.env.REACT_APP_NOTIFICATION_LISTENER_URL || 'ws://localhost:8998';

export const useServerNotifications = (onServerStartup) => {
  const [isListening, setIsListening] = useState(false);
  const [lastNotification, setLastNotification] = useState(null);

  useEffect(() => {
    // Skip if no callback provided (e.g., in Electron mode)
    if (!onServerStartup) {
      return;
    }

    let ws = null;
    let reconnectTimeout = null;

    const connect = () => {
      try {
        ws = new WebSocket(NOTIFICATION_LISTENER_URL);

        ws.onopen = () => {
          console.log('Connected to notification listener');
          setIsListening(true);
        };

        ws.onmessage = (event) => {
          try {
            const notification = JSON.parse(event.data);
            console.log('Received notification:', notification);
            
            setLastNotification(notification);

            // If it's a server startup notification, call the callback
            if (notification.type === 'server_startup' && onServerStartup) {
              onServerStartup(notification.serverUrl);
            }
          } catch (error) {
            console.error('Error parsing notification:', error);
          }
        };

        ws.onerror = (error) => {
          console.error('Notification listener connection error:', error);
        };

        ws.onclose = () => {
          console.log('Notification listener connection closed');
          setIsListening(false);
          
          // Try to reconnect after a delay
          reconnectTimeout = setTimeout(connect, 5000);
        };
      } catch (error) {
        console.error('Error setting up notification listener:', error);
        setIsListening(false);
        reconnectTimeout = setTimeout(connect, 5000);
      }
    };

    // Start listening
    connect();

    // Cleanup on unmount
    return () => {
      if (reconnectTimeout) {
        clearTimeout(reconnectTimeout);
      }
      if (ws) {
        ws.close();
      }
    };
  }, [onServerStartup]);

  return { isListening, lastNotification };
};
