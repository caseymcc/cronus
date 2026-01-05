import React, { useState, useEffect, useRef, useCallback } from 'react';
import { Layout, Model, TabNode } from 'flexlayout-react';
import 'flexlayout-react/style/dark.css';
import './App.css';
import DirectoryTree from './components/DirectoryTree';
import Chat from './components/Chat';
import InputArea from './components/InputArea';
import FileEditor from './components/FileEditor';
import { isElectronEnvironment, getApiBaseUrl, subscribeToConnectionEvents, subscribeToLogs } from './utils/electronBridge';
import { useServerNotifications } from './hooks/useServerNotifications';
import { useCronusClient } from './hooks/useCronusClient';

function App() {
  const [messages, setMessages] = useState({});
  const [logs, setLogs] = useState([]);
  const [showLogs, setShowLogs] = useState(true);
  const [showDirTree, setShowDirTree] = useState(true);
  const [fileTree, setFileTree] = useState({});
  const [isLoading, setIsLoading] = useState(false);
  const [selectedFile, setSelectedFile] = useState(null);
  const [loadedFiles, setLoadedFiles] = useState({});
  const [isElectron, setIsElectron] = useState(false);
  const [currentApiUrl, setCurrentApiUrl] = useState(null);
  const [electronConnected, setElectronConnected] = useState(false);
  
  const layoutRef = useRef(null);
  const pollingIntervalRef = useRef(null);

  const apiBaseUrl = currentApiUrl || getApiBaseUrl() || process.env.REACT_APP_API_URL || 'http://localhost:9000';
  
  // Use CronusClient hook for WebSocket communication (browser mode only)
  const cronusClient = useCronusClient(!isElectronEnvironment() ? apiBaseUrl : null);
  const connected = isElectronEnvironment() ? electronConnected : cronusClient.connected;
  
  // Create the initial JSON model for FlexLayout
  const createJsonModel = () => {
    const model = {
      global: {
        tabEnableClose: true,
        tabEnableFloat: true,
        tabSetTabStripHeight: 30,
        tabSetEnableMaximize: true,
      },
      borders: [
        {
          type: 'border',
          location: 'left',
          size: 250,
          children: [
            {
              type: 'tab',
              id: 'explorer',
              name: 'Explorer',
              component: 'directory-tree',
              enableClose: false,
            }
          ]
        }
      ],
      layout: {
        type: 'row',
        weight: 100,
        children: [
          {
            type: 'tabset',
            weight: 50,
            children: [
              {
                type: 'tab',
                id: 'main',
                name: 'Main',
                component: 'chat-panel',
                config: {
                  tabId: 'main',
                  showInput: true
                }
              }
            ]
          }
        ]
      }
    };
    return model;
  };
  
  const [layoutModel] = useState(
    Model.fromJson(createJsonModel())
  );
  const [layoutReady, setLayoutReady] = useState(false);

  // Handle server startup notifications (browser mode only)
  const handleServerStartup = useCallback((serverUrl) => {
    console.log('Server started at:', serverUrl);
    setCurrentApiUrl(serverUrl);
    
    // Reconnect to the new server
    setTimeout(() => {
      window.location.reload(); // Simple approach: reload to reconnect
    }, 500);
  }, []);

  // Listen for server startup notifications (browser mode only)
  const { isListening } = useServerNotifications(
    isElectronEnvironment() ? null : handleServerStartup
  );
  
  // Function to fetch source map data initially or as fallback
  const fetchSourceMapData = useCallback(async () => {
    try {
      setIsLoading(true);
      const response = await fetch(`${apiBaseUrl}/api/sourcemap`);
      
      if (response.ok) {
        const data = await response.json();
        if (data.fileTree) {
          setFileTree(data.fileTree);
        }
      } else {
        console.error('Error fetching source map data:', await response.text());
      }
    } catch (error) {
      console.error('Error fetching source map data:', error);
    } finally {
      setIsLoading(false);
    }
  }, [apiBaseUrl]);

  // Function to fetch file content
  const fetchFileContent = useCallback(async (filePath) => {
    try {
      setIsLoading(true);
      
      // Check if we already have this file loaded
      if (loadedFiles[filePath]) {
        // File is already loaded
        return loadedFiles[filePath];
      }
      
      // Encode file path for URL parameter
      const encodedPath = encodeURIComponent(filePath);
      const response = await fetch(`${apiBaseUrl}/api/file?path=${encodedPath}`);
      
      if (response.ok) {
        const data = await response.json();
        // Update loaded files cache
        setLoadedFiles(prev => ({
          ...prev,
          [filePath]: data
        }));
        return data;
      } else {
        console.error('Error fetching file content:', await response.text());
        return null;
      }
    } catch (error) {
      console.error('Error fetching file content:', error);
      return null;
    } finally {
      setIsLoading(false);
    }
  }, [apiBaseUrl, loadedFiles]);
  
  // Set up CronusClient event listeners for real-time updates
  useEffect(() => {
    if (!cronusClient.client) return;
    
    const handleMessage = (message) => {
      console.log('Received message from server:', message);
      // Handle incoming messages
      const tabId = 'main'; // TODO: route to appropriate tab
      setMessages(prev => ({
        ...prev,
        [tabId]: [...(prev[tabId] || []), message]
      }));
    };
    
    const handleLog = (log) => {
      console.log('Received log from server:', log);
      setLogs(prev => [...prev, log]);
    };
    
    const handleDirectoryUpdate = (data) => {
      console.log('Received directory update from server');
      if (data.fileTree) {
        setFileTree(data.fileTree);
      }
    };
    
    cronusClient.client.on('message', handleMessage);
    cronusClient.client.on('log', handleLog);
    cronusClient.client.on('directory_update', handleDirectoryUpdate);
    cronusClient.client.on('directory_init', handleDirectoryUpdate);
    
    return () => {
      cronusClient.client.off('message', handleMessage);
      cronusClient.client.off('log', handleLog);
      cronusClient.client.off('directory_update', handleDirectoryUpdate);
      cronusClient.client.off('directory_init', handleDirectoryUpdate);
    };
  }, [cronusClient.client]);
  
  // Function to send user input to the API
  const sendInput = useCallback(async (input, tabId = 'main') => {
    try {
      setIsLoading(true);
      // Add user message to chat
      setMessages(prev => ({
        ...prev,
        [tabId]: [
          ...(prev[tabId] || []),
          { role: 'user', content: input, timestamp: new Date() }
        ]
      }));
      
      await fetch(`${apiBaseUrl}/api/input`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({ input })
      });
    } catch (error) {
      console.error('Error sending input:', error);
    } finally {
      setIsLoading(false);
    }
  }, [apiBaseUrl]);
  
  // Get the active tab ID from the FlexLayout model
  const getActiveTabId = useCallback(() => {
    if (!layoutRef.current) return 'main';
    
    const model = layoutRef.current.getModel();
    const activeTabset = model.getActiveTabset();
    
    if (!activeTabset) return 'main';
    
    const activeTabNode = activeTabset.getSelectedNode();
    if (!activeTabNode || activeTabNode.getType() !== 'tab') return 'main';
    
    const config = activeTabNode.getConfig() || {};
    return config.tabId || activeTabNode.getId();
  }, []);
  
  // Poll for new messages (still need this for messages)
  const pollMessages = useCallback(async () => {
    try {
      const response = await fetch(`${apiBaseUrl}/api/messages`);
      if (response.ok) {
        const data = await response.json();
        
        // Process messages
        if (data.messages && data.messages.length > 0) {
          setMessages(prev => {
            const updatedMessages = { ...prev };
            const currentTabId = getActiveTabId();
            const currentTabMessages = updatedMessages[currentTabId] || [];
            
            // Filter out duplicates
            const newMessages = data.messages.filter(msg => 
              !currentTabMessages.some(existing => 
                existing.content === msg.content && 
                existing.role === msg.provider
              )
            );
            
            if (newMessages.length > 0) {
              updatedMessages[currentTabId] = [
                ...currentTabMessages,
                ...newMessages.map(msg => ({
                  role: msg.provider,
                  content: msg.content,
                  timestamp: new Date()
                }))
              ];
            }
            
            return updatedMessages;
          });
        }
        
        // Process logs
        if (data.logs && data.logs.length > 0) {
          setLogs(prev => {
            // Filter out duplicates
            const newLogs = data.logs.filter(log => 
              !prev.some(existing => 
                existing.message === log.message && 
                existing.level === log.level
              )
            );
            
            return [
              ...prev,
              ...newLogs.map(log => ({
                level: log.level,
                message: log.message,
                timestamp: new Date()
              }))
            ];
          });
        }
      }
    } catch (error) {
      console.error('Error polling messages:', error);
    }
  }, [apiBaseUrl, getActiveTabId]);
  
  // Create a new tab in the FlexLayout model
  const createTab = useCallback((tabId, title, isFile = false, component = 'chat-panel') => {
    // If no tabId provided, generate one
    const newTabId = tabId || `tab-${Date.now()}`;
    
    console.log("layoutRef.current =", layoutRef.current);
    console.log("layoutRef.current.getModel =", layoutRef.current?.getModel);
    console.log("model =", layoutRef.current?.getModel?.());

    if (!layoutRef.current || typeof layoutRef.current.getModel !== 'function') {
      console.warn('Layout reference not fully initialized yet');
      // Either queue this action for later or use a fallback approach
      return newTabId;
    }
    
    // Initialize messages for this tab if needed
    if (component === 'chat-panel') {
      setMessages(prev => {
        if (!prev[newTabId]) {
          return { ...prev, [newTabId]: [] };
        }
        return prev;
      });
    }
    
    // If this tab already exists, just select it
    const existingNode = layoutRef.current.getModel().getNodeById(newTabId);
    if (existingNode) {
      layoutRef.current.getModel().doAction(TabNode.selectTab(existingNode.getId()));
      return newTabId;
    }
    
    // Get the current active tabset to add the new tab there
    const model = layoutRef.current.getModel();
    const activeTabset = model.getActiveTabset();
    
    if (activeTabset) {
      // Add tab to the active tabset
      model.doAction(TabNode.addNode({
        type: 'tab',
        name: title || (isFile ? 'File' : 'Chat'),
        component: component,
        id: newTabId,
        config: {
          tabId: newTabId,
          isFile: isFile,
          showInput: component === 'chat-panel'
        }
      }, activeTabset.getId(), TabNode.POSITION_LAST));
    } else {
      // If no active tabset, add to the first tabset we find
      const tabsets = model.getNodesByType('tabset');
      if (tabsets.length > 0) {
        model.doAction(TabNode.addNode({
          type: 'tab',
          name: title || (isFile ? 'File' : 'Chat'),
          component: component,
          id: newTabId,
          config: {
            tabId: newTabId,
            isFile: isFile,
            showInput: component === 'chat-panel'
          }
        }, tabsets[0].getId(), TabNode.POSITION_LAST));
      }
    }
    
    return newTabId;
  }, [layoutRef]);
  
  // Handle file item click in the directory tree
  const handleItemClick = useCallback(async (item) => {
    if (item.type === 'file') {
      setSelectedFile(item);
      
      // Create a unique tab ID for this file
      const fileTabId = `file-${item.path.replace(/[^a-zA-Z0-9]/g, '-')}`;
      const fileName = item.path.split('/').pop();
      
      let fileData = loadedFiles[item.path];
      
      // Create a new editor tab for this file
      createTab(fileTabId, fileName, true, 'file-editor');

      // Load file content if not already loaded
      if (!fileData) {
        fileData = await fetchFileContent(item.path);
      }
      
      
    }
  }, [createTab, fetchFileContent, loadedFiles]);
  
  // Handle manual refresh request of directory tree
  const handleRefresh = useCallback(() => {
    fetchSourceMapData();
  }, [fetchSourceMapData]);
  
  // Create a new chat tab
  const handleNewChat = useCallback(() => {
    createTab(`chat-${Date.now()}`, 'New Chat');
  }, [createTab]);
  
  // Handler for layout changes
  const handleLayoutChange = useCallback((model) => {
    // Usually you don't need to store the model in state
    // as FlexLayout maintains its own state
    if (!layoutReady && model) {
      console.log('FlexLayout is now fully initialized');
      setLayoutReady(true);
    }
  }, [layoutReady]);
  
  // Factory function for FlexLayout components
  const factory = useCallback((node) => {
    const component = node.getComponent();
    const config = node.getConfig() || {};
    const tabId = config.tabId || node.getId();
    
    if (component === 'directory-tree') {
      return (
        <div className="directory-panel-wrapper">
          <DirectoryTree 
            fileTree={fileTree} 
            onRefresh={handleRefresh}
            onItemClick={handleItemClick}
          />
        </div>
      );
    } else if (component === 'chat-panel') {
      return (
        <div className="chat-panel-wrapper">
          <Chat 
            messages={messages[tabId] || []} 
            logs={logs} 
            showLogs={showLogs && tabId === 'main'} 
            selectedFile={config.isFile ? selectedFile : null}
          />
          {config.showInput && (
            <InputArea 
              onSendMessage={(input) => sendInput(input, tabId)} 
              isLoading={isLoading} 
            />
          )}
        </div>
      );
    } else if (component === 'file-editor') {
      const fileId = tabId;
      const filePath = fileId.replace(/^file-/, '').replace(/-/g, '/');
      const fileData = loadedFiles[filePath] || {};
      
      return (
        <div className="file-editor-wrapper">
          <FileEditor 
            filePath={filePath}
            fileContent={fileData.content}
            tags={fileData.tags || []}
          />
        </div>
      );
    }
    
    return <div>Unknown component: {component}</div>;
  }, [fileTree, handleRefresh, handleItemClick, messages, logs, showLogs, selectedFile, isLoading, sendInput, loadedFiles]);
  
  // Set up SSE and polling on component mount
  useEffect(() => {
    // Detect if running in Electron
    const checkElectron = async () => {
      const isElectronEnv = isElectronEnvironment();
      setIsElectron(isElectronEnv);
      
      if (isElectronEnv) {
        console.log('Running in Electron environment');
        
        // Subscribe to Electron events
        const unsubscribe = subscribeToConnectionEvents(
          () => {
            console.log('Electron: Connected to Cronus server');
            setElectronConnected(true);
          },
          () => {
            console.log('Electron: Disconnected from Cronus server');
            setElectronConnected(false);
          },
          (error) => {
            console.error('Electron: Connection error:', error);
          }
        );
        
        // Subscribe to logs from Electron
        const unsubscribeLogs = subscribeToLogs((log) => {
          setLogs(prev => [...prev, {
            level: log.level,
            message: log.message,
            timestamp: new Date(log.timestamp)
          }]);
        });
        
        return () => {
          unsubscribe();
          unsubscribeLogs();
        };
      }
    };
    
    checkElectron();
    
    // Ensure main tab exists
    if (!messages['main']) {
      setMessages(prev => ({
        ...prev,
        main: []
      }));
    }
    
    // Only set up browser polling if NOT in Electron (WebSocket handled by useCronusClient hook)
    if (!isElectronEnvironment()) {
      // Fetch source map data initially as a fallback
      fetchSourceMapData();
      
      // Start polling for messages
      pollingIntervalRef.current = setInterval(pollMessages, 1000);
      
      // Clean up on unmount
      return () => {
        if (pollingIntervalRef.current) {
          clearInterval(pollingIntervalRef.current);
        }
      };
    }
  }, [fetchSourceMapData, pollMessages, messages]);
  
  return (
    <div className="app">
      <header className="app-header">
        <h1>Cronus {isElectron && <span style={{fontSize: '0.6em', opacity: 0.7}}>(Standalone)</span>}</h1>
        <div className="header-controls">
          <span className={`connection-status ${connected ? 'connected' : 'disconnected'}`}>
            {isElectron ? (connected ? 'Server: Connected' : 'Server: Disconnected') : (connected ? 'Events: Connected' : isListening ? 'Waiting for server...' : 'Events: Disconnected')}
          </span>
          <button
            className="control-button"
            onClick={() => setShowLogs(!showLogs)}
          >
            {showLogs ? 'Logs: ON' : 'Logs: OFF'}
          </button>
          <button
            className="control-button"
            onClick={handleNewChat}
          >
            New Chat
          </button>
          <button
            className="control-button"
            onClick={() => {
              const model = layoutRef.current.getModel();
              const explorerNode = model.getNodeById('explorer');
              const borderNode = model.getBorderNode('left');
              
              if (explorerNode && borderNode) {
                if (showDirTree) {
                  model.hideBorder('left');
                } else {
                  model.showBorder('left');
                }
                setShowDirTree(!showDirTree);
              }
            }}
          >
            {showDirTree ? 'Hide Explorer' : 'Show Explorer'}
          </button>
        </div>
      </header>
      
      <div className="app-content">
        <Layout
          model={layoutModel}
          factory={factory}
          onModelChange={handleLayoutChange}
          ref={layoutRef}
        />
      </div>
      
      <footer className="app-footer">
        <div className="status-bar">
          <div className="status-items">
            <div className="status-item" title={connected ? "Connected to server events" : "Not connected to server events"}>
              <div className={`status-indicator ${connected ? 'connected' : 'disconnected'}`}>
                <svg viewBox="0 0 24 24" width="16" height="16">
                  {connected ? (
                    <path fill="currentColor" d="M8.59,16.58L13.17,12L8.59,7.41L10,6L16,12L10,18L8.59,16.58Z" />
                  ) : (
                    <path fill="currentColor" d="M13,13H11V7H13M13,17H11V15H13M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2Z" />
                  )}
                </svg>
              </div>
              <span className="status-text">{isElectron ? 'Server' : 'Events'}: {connected ? 'Connected' : 'Disconnected'}</span>
            </div>
            <div className="status-item">
              <span className="status-text">{isElectron ? 'Mode: Standalone' : `API URL: ${apiBaseUrl}`}</span>
            </div>
          </div>
        </div>
      </footer>
    </div>
  );
}

export default App;