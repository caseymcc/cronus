import React, { useState } from 'react';
import {
    Box,
    Typography,
    IconButton,
    List,
    ListItemButton,
    ListItemIcon,
    ListItemText,
    Collapse,
} from '@mui/material';
import {
    Folder as FolderIcon,
    FolderOpen as FolderOpenIcon,
    InsertDriveFile as FileIcon,
    Refresh as RefreshIcon,
    ExpandMore as ExpandMoreIcon,
    ChevronRight as ChevronRightIcon,
} from '@mui/icons-material';

// Component for a single tree item (file or directory)
const TreeItem=({ name, item, level, onItemClick }) =>
{
    const [expanded, setExpanded]=useState(false);
    const isDirectory=item.type==='directory';
    const hasChildren=isDirectory && item.children && Object.keys(item.children).length>0;
    
    const handleToggle=(e) =>
    {
        e.stopPropagation();
        if(isDirectory)
        {
            setExpanded(!expanded);
        }
    };

    const handleClick=() =>
    {
        onItemClick(item);
        if(isDirectory)
        {
            setExpanded(!expanded);
        }
    };
    
    return (
        <Box>
            <ListItemButton
                onClick={handleClick}
                sx={{
                    pl: level*2+1,
                    py: 0.5,
                }}
            >
                <ListItemIcon sx={{ minWidth: 32 }}>
                    {isDirectory ? (
                        <IconButton
                            size="small"
                            onClick={handleToggle}
                            sx={{ p: 0 }}
                        >
                            {expanded ? <ExpandMoreIcon fontSize="small" />:<ChevronRightIcon fontSize="small" />}
                        </IconButton>
                    ):(
                        <FileIcon fontSize="small" color="action" />
                    )}
                </ListItemIcon>
                <ListItemIcon sx={{ minWidth: 32 }}>
                    {isDirectory ? (
                        expanded ? <FolderOpenIcon fontSize="small" color="primary" />:<FolderIcon fontSize="small" color="action" />
                    ):(
                        <FileIcon fontSize="small" color="action" />
                    )}
                </ListItemIcon>
                <ListItemText
                    primary={name}
                    primaryTypographyProps={{
                        variant: 'body2',
                        noWrap: true,
                    }}
                />
            </ListItemButton>
            
            {hasChildren && (
                <Collapse in={expanded} timeout="auto" unmountOnExit>
                    <List component="div" disablePadding>
                        {Object.entries(item.children).map(([childName, childItem]) => (
                            <TreeItem
                                key={childName}
                                name={childName}
                                item={childItem}
                                level={level+1}
                                onItemClick={onItemClick}
                            />
                        ))}
                    </List>
                </Collapse>
            )}
        </Box>
    );
};

function DirectoryTree({ fileTree, onRefresh, onItemClick })
{
    return (
        <Box
            sx={{
                display: 'flex',
                flexDirection: 'column',
                height: '100%',
                overflow: 'hidden',
            }}
        >
            <Box
                sx={{
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'space-between',
                    p: 1.5,
                    borderBottom: 1,
                    borderColor: 'divider',
                }}
            >
                <Typography variant="h6" sx={{ fontSize: '1rem' }}>
                    Project Files
                </Typography>
                <IconButton
                    onClick={onRefresh}
                    size="small"
                    title="Refresh file structure"
                >
                    <RefreshIcon fontSize="small" />
                </IconButton>
            </Box>
            <Box sx={{ flex: 1, overflow: 'auto' }}>
                {fileTree && Object.keys(fileTree).length>0 ? (
                    <List component="nav" disablePadding>
                        {Object.entries(fileTree).map(([name, item]) => (
                            <TreeItem
                                key={name}
                                name={name}
                                item={item}
                                level={0}
                                onItemClick={onItemClick}
                            />
                        ))}
                    </List>
                ):(
                    <Typography
                        variant="body2"
                        color="text.secondary"
                        sx={{ textAlign: 'center', mt: 4, px: 2 }}
                    >
                        No files found
                    </Typography>
                )}
            </Box>
        </Box>
    );
}

export default DirectoryTree;
