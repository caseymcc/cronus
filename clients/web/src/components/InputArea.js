import React, { useState, useRef, useEffect } from 'react';
import './InputArea.css';

function InputArea({ onSendMessage, isLoading }) {
  const [input, setInput] = useState('');
  const textareaRef = useRef(null);

  // Auto-resize the textarea as content grows
  useEffect(() => {
    if (textareaRef.current) {
      textareaRef.current.style.height = 'auto';
      textareaRef.current.style.height = `${Math.min(textareaRef.current.scrollHeight, 200)}px`;
    }
  }, [input]);

  const handleSubmit = (e) => {
    e.preventDefault();
    if (input.trim()) {
      onSendMessage(input);
      setInput(''); // Clear input after sending
    }
  };

  return (
    <div className="input-area">
      <form onSubmit={handleSubmit} className="input-box">
        <textarea
          ref={textareaRef}
          className="input-textarea"
          value={input}
          onChange={(e) => setInput(e.target.value)}
          placeholder="Type your message here..."
          disabled={isLoading}
          rows={1}
          onKeyDown={(e) => {
            if (e.key === 'Enter' && !e.shiftKey) {
              e.preventDefault();
              handleSubmit(e);
            }
          }}
        />
        <button 
          type="submit" 
          className="send-button"
          disabled={isLoading || !input.trim()}
        >
          {isLoading ? 'Processing...' : 'Send'}
        </button>
      </form>
    </div>
  );
}

export default InputArea;