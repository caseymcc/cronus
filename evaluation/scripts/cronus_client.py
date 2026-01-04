#!/usr/bin/env python3
"""
Cronus API client for evaluation system.
Handles starting the Cronus server, sending requests, and collecting SSE responses.
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
import sseclient  # pip install sseclient-py


class CronusClient:
    """Client for interacting with Cronus server."""
    
    def __init__(self, executable_path: str, working_dir: str,  
                 api_url: str = "http://localhost:9000", port: int = 9000):
        self.executable_path = executable_path
        self.working_dir = working_dir
        self.api_url = api_url
        self.port = port
        self.process = None
        self.sse_thread = None
        self.latest_response = None
        self.response_complete = threading.Event()
        
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
        cmd = [
            self.executable_path,
            "--web",
            "--port", str(self.port),
            "--resource-dir", self.working_dir
        ]
        
        print(f"Starting Cronus server: {' '.join(cmd)}")
        
        # Set up environment with library path
        env = os.environ.copy()
        # Add the build directory to LD_LIBRARY_PATH for shared libraries
        # Executable is at: /app/build/linux_x64_debug/server/cronus/cronus
        # Libraries are at: /app/build/linux_x64_debug/
        build_dir = str(Path(self.executable_path).parent.parent.parent)
        print(f"Setting LD_LIBRARY_PATH to include: {build_dir}")
        if 'LD_LIBRARY_PATH' in env:
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
        """Stop the Cronus server process."""
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
    
    def _listen_sse(self):
        """Listen for SSE events in background thread."""
        try:
            response = requests.get(
                f"{self.api_url}/api/events",
                stream=True,
                headers={'Accept': 'text/event-stream'}
            )
            
            client = sseclient.SSEClient(response)
            
            for event in client.events():
                if event.data:
                    try:
                        data = json.loads(event.data)
                        event_type = data.get('type')
                        
                        # Look for response events
                        if event_type == 'response':
                            self.latest_response = data.get('message', data.get('response', ''))
                            self.response_complete.set()
                            
                    except json.JSONDecodeError:
                        pass
                        
        except Exception as e:
            print(f"SSE listening error: {e}")
    
    def generate_code(self, prompt: str, timeout: int = 120) -> Optional[Dict[str, Any]]:
        """Send a code generation request and wait for response via SSE.
        
        Returns dict with:
        - code: Generated code
        - raw_response: Full response from Cronus
        - success: Boolean indicating if code was generated
        """
        if not self.process:
            print("Cronus server not running")
            return None
        
        # Reset response tracking
        self.latest_response = None
        self.response_complete.clear()
        
        # Start SSE listener if not already running
        if not self.sse_thread or not self.sse_thread.is_alive():
            self.sse_thread = threading.Thread(target=self._listen_sse, daemon=True)
            self.sse_thread.start()
            time.sleep(1)  # Give it time to connect
        
        # Send the request
        try:
            response = requests.post(
                f"{self.api_url}/api/input",
                json={'input': prompt},
                timeout=10
            )
            
            if response.status_code != 200:
                print(f"Request failed: {response.status_code}")
                return {
                    'code': None,
                    'raw_response': None,
                    'success': False,
                    'error': f"HTTP {response.status_code}"
                }
            
            # Wait for SSE response
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
                
        except requests.exceptions.RequestException as e:
            print(f"Request error: {e}")
            return {
                'code': None,
                'raw_response': None,
                'success': False,
                'error': str(e)
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
