import React, { useState, useEffect } from 'react';
import {
    Box,
    Paper,
    Typography,
    Button,
    Chip,
} from '@mui/material';
import {
    Edit as EditIcon,
    Visibility as ViewIcon,
} from '@mui/icons-material';

function FileEditor({ filePath, fileContent, tags })
{
    const [content, setContent]=useState(fileContent||'');
    const [isEditing, setIsEditing]=useState(false);
    
    useEffect(() =>
    {
        if(fileContent)
        {
            setContent(fileContent);
        }
    }, [fileContent]);

    const handleEditToggle=() =>
    {
        setIsEditing(!isEditing);
    };

    const handleContentChange=(e) =>
    {
        setContent(e.target.value);
    };

    // Determine the language for syntax highlighting
    const getLanguageFromPath=(path) =>
    {
        if(!path) return 'text';
        
        const extension=path.split('.').pop().toLowerCase();
        switch(extension)
        {
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

    const language=getLanguageFromPath(filePath);
    
    return (
        <Paper
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
                    gap: 2,
                }}
            >
                <Box sx={{ flex: 1, minWidth: 0 }}>
                    <Typography
                        variant="subtitle1"
                        noWrap
                        sx={{ fontFamily: 'monospace', fontSize: '0.9rem' }}
                    >
                        {filePath}
                    </Typography>
                    {tags && tags.length>0 && (
                        <Box sx={{ display: 'flex', gap: 0.5, mt: 0.5, flexWrap: 'wrap' }}>
                            {tags.map((tag, idx) => (
                                <Chip
                                    key={idx}
                                    label={tag}
                                    size="small"
                                    variant="outlined"
                                    sx={{ fontSize: '0.7rem', height: 20 }}
                                />
                            ))}
                        </Box>
                    )}
                </Box>
                <Button
                    variant="outlined"
                    size="small"
                    startIcon={isEditing ? <ViewIcon />:<EditIcon />}
                    onClick={handleEditToggle}
                >
                    {isEditing ? 'View':'Edit'}
                </Button>
            </Box>
            <Box
                sx={{
                    flex: 1,
                    overflow: 'auto',
                    p: 2,
                }}
            >
                {isEditing ? (
                    <Box
                        component="textarea"
                        value={content}
                        onChange={handleContentChange}
                        spellCheck="false"
                        sx={{
                            width: '100%',
                            height: '100%',
                            fontFamily: 'monospace',
                            fontSize: '0.875rem',
                            border: 'none',
                            outline: 'none',
                            resize: 'none',
                            backgroundColor: 'transparent',
                            color: 'text.primary',
                            p: 0,
                        }}
                    />
                ):(
                    <Box
                        component="pre"
                        sx={{
                            margin: 0,
                            fontFamily: 'monospace',
                            fontSize: '0.875rem',
                            whiteSpace: 'pre-wrap',
                            wordBreak: 'break-word',
                            overflow: 'auto',
                        }}
                    >
                        <code className={`language-${language}`}>
                            {content}
                        </code>
                    </Box>
                )}
            </Box>
        </Paper>
    );
}

export default FileEditor;
