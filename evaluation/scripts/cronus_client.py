#!/usr/bin/env python3
"""
Cronus API client for evaluation system.
Handles starting the Cronus server, sending requests via WebSocket JSON-RPC, and collecting responses.
"""

import subprocess
import time
import requests
import json
import os
import signal
import threading
from typing import Optional, Dict, Any
from pathlib import Path
import websocket  # pip install websocket-client


class CronusClient:
    """Client for interacting with Cronus server via WebSocket JSON-RPC."""
    
    def __init__(self, executable_path: str, working_dir: str,  
                 api_url: str = "http://localhost:9000", port: int = 9000):
        self.executable_path = executable_path
        self.working_dir = working_dir
        self.api_url = api_url
        self.port = port
        self.process = None
        self.ws = None
        self.request_id = 0
        self.pending_requests = {}
        self.latest_response = None
        self.response_complete = threading.Event()
        self.ws_thread = None
        self.lock = threading.Lock()
        
    def start_server(self, timeout: int = 30) -> bool:
        """Start the Cronus server process.
        
        Returns:
            True if server started successfully or is already running
            False if executable not found or server failed to start
        """
        # Check if server is already running by trying to reach it
        try:
            response = requests.get(f"{self.api_url}/api/health", timeout=2)
            if response.status_code == 200:
                print("Cronus server is already running")
                return True
        except requests.exceptions.RequestException:
            # Server not running, continue with startup
            pass
            
        if self.process:
            print("Cronus server process already started by this client")
            return True
        
        # Check if executable exists
        executable_path = Path(self.executable_path)
        if not executable_path.exists():
            print(f"ERROR: Cronus executable not found at: {self.executable_path}")
            print(f"Please build Cronus first:")
            print(f"  ./run_local.sh ./generate.sh")
            print(f"  ./run_local.sh ninja -C build/linux_x64_debug")
            return False
        
        if not executable_path.is_file():
            print(f"ERROR: {self.executable_path} is not a file")
            return False
            
        if not os.access(executable_path, os.X_OK):
            print(f"ERROR: {self.executable_path} is not executable")
            print(f"Run: chmod +x {self.executable_path}")
            return False
            
        # Ensure working directory exists
        os.makedirs(self.working_dir, exist_ok=True)
        
        # Start Cronus server in background
        # Use --web flag to enable HTTP API endpoints (required for evaluation)
        cmd = [
            self.executable_path,
            "--web",
            "--port", str(self.port),
            "--resource-dir", self.working_dir
        ]
        
        print(f"Starting Cronus server: {' '.join(cmd)}")
        
        # Set up environment with library path
        env = os.environ.copy()
        # LD_LIBRARY_PATH should already be set in Docker environment,
        # but we ensure it includes the build directory for shared libraries
        # Executable is at: /app/build/linux_x64_debug/server/cronus/cronus
        # Libraries are at: /app/build/linux_x64_debug/
        build_dir = str(Path(self.executable_path).parent.parent.parent)
        if 'LD_LIBRARY_PATH' in env:
            # Prepend build_dir if not already present
            if build_dir not in env['LD_LIBRARY_PATH'].split(':'):
                env['LD_LIBRARY_PATH'] = f"{build_dir}:{env['LD_LIBRARY_PATH']}"
        else:
            env['LD_LIBRARY_PATH'] = build_dir
        print(f"LD_LIBRARY_PATH={env['LD_LIBRARY_PATH']}")
        
        try:
            self.process = subprocess.Popen(
                cmd,
                cwd=self.working_dir,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                env=env
            )
            
            # Wait for server to be ready
            start_time = time.time()
            while time.time() - start_time < timeout:
                # Check if process has terminated unexpectedly
                if self.process.poll() is not None:
                    # Process terminated, get error output
                    stdout, stderr = self.process.communicate()
                    print(f"ERROR: Cronus server process terminated unexpectedly")
                    print(f"Exit code: {self.process.returncode}")
                    if stderr:
                        print(f"Error output:\n{stderr}")
                    if stdout:
                        print(f"Standard output:\n{stdout}")
                    self.process = None
                    return False
                
                try:
                    response = requests.get(f"{self.api_url}/api/health", timeout=1)
                    if response.status_code == 200:
                        print("Cronus server is ready")
                        return True
                except requests.exceptions.RequestException:
                    pass
                time.sleep(1)
            
            print(f"ERROR: Timeout waiting for Cronus server to start (waited {timeout}s)")
            print(f"Check if port {self.port} is already in use:")
            print(f"  netstat -tlnp | grep {self.port}")
            self.stop_server()
            return False
            
        except Exception as e:
            print(f"Error starting Cronus server: {e}")
            return False
    
    def stop_server(self):
        """Stop the Cronus server process and close WebSocket."""
        self.close_websocket()
        
        if self.process:
            print("Stopping Cronus server...")
            self.process.terminate()
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
            self.process = None
            print("Cronus server stopped")
    
    def connect_websocket(self):
        """Connect to the WebSocket endpoint."""
        if self.ws:
            return True
            
        ws_url = self.api_url.replace('http', 'ws') + '/ws'
        
        try:
            self.ws = websocket.WebSocketApp(
                ws_url,
                on_message=self._on_message,
                on_error=self._on_error,
                on_close=self._on_close,
                on_open=self._on_open
            )
            
            # Run WebSocket in separate thread
            self.ws_thread = threading.Thread(target=self.ws.run_forever, daemon=True)
            self.ws_thread.start()
            
            # Wait for connection
            time.sleep(1)
            return True
        except Exception as e:
            print(f"WebSocket connection error: {e}")
            return False
    
    def close_websocket(self):
        """Close WebSocket connection."""
        if self.ws:
            self.ws.close()
            self.ws = None
        if self.ws_thread:
            self.ws_thread.join(timeout=2)
            self.ws_thread = None
    
    def _on_open(self, ws):
        """WebSocket opened."""
        print("WebSocket connected")
    
    def _on_close(self, ws, close_status_code, close_msg):
        """WebSocket closed."""
        print(f"WebSocket closed: {close_status_code} - {close_msg}")
    
    def _on_error(self, ws, error):
        """WebSocket error."""
        print(f"WebSocket error: {error}")
    
    def _on_message(self, ws, message):
        """Handle incoming WebSocket message."""
        try:
            data = json.loads(message)
            
            # Handle JSON-RPC response
            if 'id' in data and data.get('id') is not None:
                request_id = data['id']
                with self.lock:
                    if request_id in self.pending_requests:
                        event = self.pending_requests[request_id]
                        if 'error' in data:
                            self.pending_requests[request_id + '_result'] = {
                                'error': data['error']['message']
                            }
                        else:
                            self.pending_requests[request_id + '_result'] = data.get('result')
                        event.set()
            
            # Handle JSON-RPC notification
            elif 'method' in data:
                method = data['method']
                params = data.get('params', {})
                
                # Look for response in message notification
                if method == 'message':
                    content = params.get('content', '')
                    if content:
                        self.latest_response = content
                        self.response_complete.set()
                        
        except json.JSONDecodeError as e:
            print(f"JSON decode error: {e}")
    
    def send_request(self, method: str, params: Optional[Dict] = None, timeout: int = 30) -> Optional[Any]:
        """Send JSON-RPC request and wait for response."""
        if not self.ws:
            print("WebSocket not connected")
            return None
        
        with self.lock:
            self.request_id += 1
            request_id = self.request_id
        
        request = {
            'jsonrpc': '2.0',
            'method': method,
            'params': params or {},
            'id': request_id
        }
        
        # Create event for this request
        event = threading.Event()
        with self.lock:
            self.pending_requests[request_id] = event
        
        try:
            # Send request
            self.ws.send(json.dumps(request))
            
            # Wait for response
            if event.wait(timeout):
                with self.lock:
                    result = self.pending_requests.get(request_id + '_result')
                    # Clean up
                    del self.pending_requests[request_id]
                    if request_id + '_result' in self.pending_requests:
                        del self.pending_requests[request_id + '_result']
                    
                    if isinstance(result, dict) and 'error' in result:
                        print(f"JSON-RPC error: {result['error']}")
                        return None
                    return result
            else:
                print(f"Request timeout for method: {method}")
                with self.lock:
                    if request_id in self.pending_requests:
                        del self.pending_requests[request_id]
                return None
        except Exception as e:
            print(f"Error sending request: {e}")
            with self.lock:
                if request_id in self.pending_requests:
                    del self.pending_requests[request_id]
            return None
    
    def generate_code(self, prompt: str, timeout: int = 120) -> Optional[Dict[str, Any]]:
        """Send a code generation request and wait for response via WebSocket.
        
        Returns dict with:
        - code: Generated code
        - raw_response: Full response from Cronus
        - success: Boolean indicating if code was generated
        """
        if not self.process:
            print("Cronus server not running")
            return None
        
        # Connect WebSocket if not already connected
        if not self.ws:
            if not self.connect_websocket():
                return {
                    'code': None,
                    'raw_response': None,
                    'success': False,
                    'error': 'Failed to connect WebSocket'
                }
        
        # Reset response tracking
        self.latest_response = None
        self.response_complete.clear()
        
        # Send the request via JSON-RPC
        self.send_request('input', {'input': prompt, 'sessionId': 'default'})
        
        # Wait for response via notification
        if self.response_complete.wait(timeout=timeout):
            return {
                'code': self.latest_response,
                'raw_response': self.latest_response,
                'success': bool(self.latest_response)
            }
        else:
            print("Timeout waiting for response")
            return {
                'code': None,
                'raw_response': None,
                'success': False,
                'error': "Timeout waiting for response"
            }
    
    def __enter__(self):
        """Context manager entry."""
        self.start_server()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        self.stop_server()


if __name__ == '__main__':
    # Test the client
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: cronus_client.py <path_to_cronus_executable>")
        sys.exit(1)
    
    executable = sys.argv[1]
    working_dir = "/tmp/cronus_test"
    
    with CronusClient(executable, working_dir) as client:
        prompt = "Write a Python function that adds two numbers"
        result = client.generate_code(prompt, timeout=30)
        
        if result and result['success']:
            print("Generated code:")
            print(result['code'])
        else:
            print(f"Failed to generate code: {result.get('error') if result else 'Unknown error'}")
