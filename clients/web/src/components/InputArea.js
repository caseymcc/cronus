import React, { useState } from 'react';
import {
    Box,
    TextField,
    IconButton,
    CircularProgress,
} from '@mui/material';
import {
    Send as SendIcon,
} from '@mui/icons-material';

function InputArea({ onSendMessage, isLoading })
{
    const [input, setInput]=useState('');

    const handleSubmit=(e) =>
    {
        e.preventDefault();
        if(input.trim())
        {
            onSendMessage(input);
            setInput(''); // Clear input after sending
        }
    };

    return (
        <Box
            component="form"
            onSubmit={handleSubmit}
            sx={{
                p: 2,
                borderTop: 1,
                borderColor: 'divider',
                backgroundColor: 'background.paper',
            }}
        >
            <Box sx={{ display: 'flex', gap: 1, alignItems: 'flex-end' }}>
                <TextField
                    fullWidth
                    multiline
                    maxRows={6}
                    value={input}
                    onChange={(e) => setInput(e.target.value)}
                    placeholder="Type your message here..."
                    disabled={isLoading}
                    onKeyDown={(e) =>
                    {
                        if(e.key==='Enter' && !e.shiftKey)
                        {
                            e.preventDefault();
                            handleSubmit(e);
                        }
                    }}
                    variant="outlined"
                    size="small"
                />
                <IconButton
                    type="submit"
                    color="primary"
                    disabled={isLoading||!input.trim()}
                    sx={{
                        width: 40,
                        height: 40,
                    }}
                >
                    {isLoading ? <CircularProgress size={24} />:<SendIcon />}
                </IconButton>
            </Box>
        </Box>
    );
}

export default InputArea;
