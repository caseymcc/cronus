import React, { useState } from 'react';

// Component for a single tree item (file or directory)
const TreeItem = ({ name, item, level, onItemClick }) => {
  const [expanded, setExpanded] = useState(false);
  const isDirectory = item.type === 'directory';
  const hasChildren = isDirectory && item.children && Object.keys(item.children).length > 0;
  
  const handleToggle = (e) => {
    e.stopPropagation();
    if (isDirectory) {
      setExpanded(!expanded);
    }
  };

  const handleClick = () => {
    onItemClick(item);
  };
  
  return (
    <div className="tree-item-container">
      <div 
        className={`directory-item ${isDirectory ? 'directory' : 'file'}`}
        style={{ paddingLeft: `${level * 20}px` }}
        onClick={handleClick}
      >
        <span className="directory-item-icon" onClick={handleToggle}>
          {isDirectory ? (expanded ? '📂' : '📁') : '📄'}
        </span>
        <span className="directory-item-name">{name}</span>
      </div>
      
      {expanded && hasChildren && (
        <div className="directory-children">
          {Object.entries(item.children).map(([childName, childItem]) => (
            <TreeItem 
              key={childName} 
              name={childName} 
              item={childItem} 
              level={level + 1}
              onItemClick={onItemClick}
            />
          ))}
        </div>
      )}
    </div>
  );
};

function DirectoryTree({ fileTree, onRefresh, onItemClick }) {
  return (
    <div className="directory-tree">
      <div className="directory-tree-header">
        <h3>Project Files</h3>
        <button onClick={onRefresh} className="refresh-button" title="Refresh file structure">
          🔄
        </button>
      </div>
      <div className="directory-tree-content">
        {fileTree && Object.keys(fileTree).length > 0 ? (
          Object.entries(fileTree).map(([name, item]) => (
            <TreeItem 
              key={name} 
              name={name} 
              item={item} 
              level={0}
              onItemClick={onItemClick}
            />
          ))
        ) : (
          <div className="directory-empty">No files found</div>
        )}
      </div>
    </div>
  );
}

export default DirectoryTree;