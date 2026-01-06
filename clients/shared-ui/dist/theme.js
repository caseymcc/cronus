"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.darkTheme = void 0;
exports.getTheme = getTheme;
const styles_1 = require("@mui/material/styles");
// Define colors matching the current dark theme
const darkThemeOptions = {
    palette: {
        mode: 'dark',
        primary: {
            main: '#3498db', // Blue accent
            light: '#5dade2',
            dark: '#2980b9',
        },
        secondary: {
            main: '#2ecc71', // Green accent
            light: '#58d68d',
            dark: '#27ae60',
        },
        background: {
            default: '#1e1e1e', // VS Code dark background
            paper: '#252526', // Slightly lighter for panels
        },
        text: {
            primary: '#cccccc', // Light gray text
            secondary: '#999999',
        },
        divider: '#404040',
        error: {
            main: '#e74c3c',
        },
        warning: {
            main: '#f39c12',
        },
        info: {
            main: '#3498db',
        },
        success: {
            main: '#2ecc71',
        },
    },
    typography: {
        fontFamily: [
            '-apple-system',
            'BlinkMacSystemFont',
            '"Segoe UI"',
            'Roboto',
            '"Helvetica Neue"',
            'Arial',
            'sans-serif',
        ].join(','),
        fontSize: 14,
        h1: {
            fontSize: '2rem',
            fontWeight: 500,
        },
        h2: {
            fontSize: '1.75rem',
            fontWeight: 500,
        },
        h3: {
            fontSize: '1.5rem',
            fontWeight: 500,
        },
        h4: {
            fontSize: '1.25rem',
            fontWeight: 500,
        },
        h5: {
            fontSize: '1.1rem',
            fontWeight: 500,
        },
        h6: {
            fontSize: '1rem',
            fontWeight: 500,
        },
        body1: {
            fontSize: '0.875rem',
        },
        body2: {
            fontSize: '0.8125rem',
        },
    },
    components: {
        MuiAppBar: {
            styleOverrides: {
                root: {
                    backgroundColor: '#2c3e50',
                },
            },
        },
        MuiButton: {
            styleOverrides: {
                root: {
                    textTransform: 'none', // Disable uppercase
                    borderRadius: 4,
                },
            },
        },
        MuiPaper: {
            styleOverrides: {
                root: {
                    backgroundImage: 'none', // Disable gradient
                },
            },
        },
        MuiTextField: {
            styleOverrides: {
                root: {
                    '& .MuiOutlinedInput-root': {
                        '& fieldset': {
                            borderColor: '#404040',
                        },
                        '&:hover fieldset': {
                            borderColor: '#3498db',
                        },
                    },
                },
            },
        },
    },
};
exports.darkTheme = (0, styles_1.createTheme)(darkThemeOptions);
// Export a function to get theme by mode
function getTheme(mode = 'dark') {
    if (mode === 'light') {
        // For now, we only support dark mode
        // Can add light theme later
        return exports.darkTheme;
    }
    return exports.darkTheme;
}
//# sourceMappingURL=theme.js.map