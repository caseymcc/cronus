import React, { useState, useEffect } from 'react';
import './FileEditor.css';

function FileEditor({ filePath, fileContent, tags }) {
  const [content, setContent] = useState(fileContent || '');
  const [isEditing, setIsEditing] = useState(false);
  
  useEffect(() => {
    if (fileContent) {
      setContent(fileContent);
    }
  }, [fileContent]);

  const handleEditToggle = () => {
    setIsEditing(!isEditing);
  };

  const handleContentChange = (e) => {
    setContent(e.target.value);
  };

  // Determine the language for syntax highlighting
  const getLanguageFromPath = (path) => {
    if (!path) return 'text';
    
    const extension = path.split('.').pop().toLowerCase();
    switch (extension) {
      case 'js':
      case 'jsx':
        return 'javascript';
      case 'ts':
      case 'tsx':
        return 'typescript';
      case 'py':
        return 'python';
      case 'cpp':
      case 'hpp':
      case 'h':
      case 'cc':
      case 'c':
        return 'cpp';
      case 'json':
        return 'json';
      case 'html':
      case 'htm':
        return 'html';
      case 'css':
        return 'css';
      case 'md':
        return 'markdown';
      default:
        return 'text';
    }
  };

  const language = getLanguageFromPath(filePath);
  
  return (
    <div className="file-editor">
      <div className="editor-header">
        <div className="file-path">{filePath}</div>
        <div className="editor-controls">
          <button 
            className="edit-button"
            onClick={handleEditToggle}
          >
            {isEditing ? 'View Mode' : 'Edit Mode'}
          </button>
        </div>
      </div>
      <div className="editor-content">
        {isEditing ? (
          <textarea 
            value={content}
            onChange={handleContentChange}
            className="editor-textarea"
            spellCheck="false"
          />
        ) : (
          <pre className={`language-${language}`}>
            <code>{content}</code>
          </pre>
        )}
      </div>
    </div>
  );
}

export default FileEditor;