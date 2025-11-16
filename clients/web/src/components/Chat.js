import React, { useEffect, useRef } from 'react';
import './Chat.css';

function formatTimestamp(timestamp) {
  const date = new Date(timestamp);
  return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
}

function Chat({ messages, logs, showLogs, selectedFile }) {
  const messagesEndRef = useRef(null);
  
  // Auto-scroll to bottom when messages change
  useEffect(() => {
    if (messagesEndRef.current) {
      messagesEndRef.current.scrollIntoView({ behavior: 'smooth' });
    }
  }, [messages]);
  
  return (
    <div className="chat-container">
      <div className="messages-container">
        {selectedFile && (
          <div className="file-info">
            <div className="file-header">
              <span className="file-name">{selectedFile.path.split('/').pop()}</span>
            </div>
            <div className="file-path">{selectedFile.path}</div>
          </div>
        )}
        
        {messages.map((message, index) => (
          <div key={index} className={`chat-message ${message.role}`}>
            <div className="chat-message-header">
              <span className="message-role">
                {message.role === 'user' ? 'You' : message.role}
              </span>
              <span className="message-time">
                {formatTimestamp(message.timestamp)}
              </span>
            </div>
            <div className="chat-message-content">{message.content}</div>
          </div>
        ))}
        <div ref={messagesEndRef} />
      </div>
      
      {showLogs && logs.length > 0 && (
        <div className="logs-container">
          <div className="logs-header">System Logs</div>
          <div className="logs-content">
            {logs.map((log, index) => (
              <div key={index} className={`log-entry ${log.level}`}>
                <span className="log-time">{formatTimestamp(log.timestamp)}</span>
                <span className="log-level">[{log.level}]</span>
                <span className="log-message">{log.message}</span>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}

export default Chat;