#!/usr/bin/env python3
"""
Evaluate agent performance on Exercism exercises.
Handles loading exercises, invoking the agent, and running tests.
"""

import argparse
import json
import os
import sys
import time
import subprocess
import tempfile
import shutil
from pathlib import Path
from typing import Dict, List, Optional, Any
import requests

# Add scripts directory to Python path
SCRIPT_DIR = Path(__file__).parent
sys.path.insert(0, str(SCRIPT_DIR))

# Import Cronus client
try:
    from cronus_client import CronusClient
except ImportError as e:
    print(f"ERROR: Failed to import cronus_client module: {e}")
    print(f"Script directory: {SCRIPT_DIR}")
    
    # Check if file exists
    cronus_client_path = SCRIPT_DIR / 'cronus_client.py'
    if not cronus_client_path.exists():
        print(f"ERROR: cronus_client.py not found at {cronus_client_path}")
        sys.exit(1)
    
    # Check for missing dependencies
    print("\nThis is likely due to missing Python dependencies.")
    print("Required packages: requests, sseclient-py")
    print("\nTo fix:")
    print("  1. Rebuild Docker image: ./run_local.sh -r")
    print("  2. Or install manually: pip3 install requests sseclient-py")
    sys.exit(1)


class ExerciseEvaluator:
    """Evaluates agent performance on a single exercise."""
    
    def __init__(self, config: Dict[str, Any], verbose: bool = False):
        self.config = config
        self.verbose = verbose
        self.agent_endpoint = config['agent']['endpoint']
        self.cronus_client = None
        
        # Initialize Cronus client
        cronus_config = config['agent']
        executable = cronus_config.get('cronus_executable', 'build/linux_x64_debug/server/cronus/cronus')
        working_dir = cronus_config.get('cronus_working_dir', '/tmp/cronus_eval')
        
        # Parse endpoint to get port
        import urllib.parse
        parsed = urllib.parse.urlparse(self.agent_endpoint)
        port = parsed.port or 9000
        
        self.log("Initializing Cronus client...")
        self.cronus_client = CronusClient(
            executable_path=executable,
            working_dir=working_dir,
            api_url=f"{parsed.scheme}://{parsed.hostname}:{port}",
            port=port
        )
        
        # Start the server
        self.log("Starting Cronus server...")
        if not self.cronus_client.start_server():
            error_msg = (
                "Failed to start Cronus server. Please ensure:\n"
                "1. Cronus is built: ./run_local.sh ninja -C build/linux_x64_debug\n"
                f"2. Executable exists at: {executable}\n"
                f"3. Port {port} is available"
            )
            self.log(error_msg, "ERROR")
            raise RuntimeError(error_msg)
        
        self.log("Cronus server started successfully")
    
    def __del__(self):
        """Cleanup on deletion."""
        if self.cronus_client:
            self.cronus_client.stop_server()
        
    def log(self, message: str, level: str = 'INFO'):
        """Log a message with timestamp."""
        timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
        print(f"[{timestamp}] [{level}] {message}")
        
    def load_exercise(self, exercise_path: Path) -> Optional[Dict[str, Any]]:
        """Load exercise metadata and instructions."""
        try:
            # Read exercise configuration
            config_file = exercise_path / '.meta' / 'config.json'
            if not config_file.exists():
                self.log(f"Config not found: {config_file}", "WARN")
                return None
                
            with open(config_file, 'r') as f:
                exercise_config = json.load(f)
            
            # Read instructions
            instructions_file = exercise_path / '.docs' / 'instructions.md'
            instructions = ""
            if instructions_file.exists():
                with open(instructions_file, 'r') as f:
                    instructions = f.read()
            
            # Get test files
            test_files = list(exercise_path.glob('*test*'))
            
            return {
                'name': exercise_path.name,
                'path': str(exercise_path),
                'config': exercise_config,
                'instructions': instructions,
                'test_files': [str(f) for f in test_files]
            }
        except Exception as e:
            self.log(f"Error loading exercise {exercise_path}: {e}", "ERROR")
            return None
    
    def create_agent_prompt(self, exercise: Dict[str, Any], language: str) -> str:
        """Create a prompt for the agent based on exercise instructions."""
        prompt = f"""You are a coding assistant. Please solve the following {language} programming exercise.

Exercise: {exercise['name']}

Instructions:
{exercise['instructions']}

Please provide a complete, working solution. Only include the code implementation, no explanations.
Make sure your code passes all tests and follows best practices for {language}.
"""
        return prompt
    
    def invoke_agent(self, prompt: str, language: str) -> Dict[str, Any]:
        """Call the Cronus agent to generate a solution.
        
        Returns a dict with:
        - code: The generated code
        - raw_response: The full response from the agent
        - interaction_details: Detailed log of the interaction
        """
        try:
            interaction_details = {
                'prompt': prompt,
                'endpoint': self.agent_endpoint,
                'timestamp': time.strftime('%Y-%m-%d %H:%M:%S')
            }
            
            # Use Cronus client
            if not self.cronus_client:
                error_msg = "Cronus client not initialized"
                self.log(error_msg, "ERROR")
                interaction_details['error'] = error_msg
                return {
                    'code': None,
                    'raw_response': None,
                    'interaction_details': interaction_details
                }
            
            timeout = self.config['languages'][language]['timeout_seconds']
            result = self.cronus_client.generate_code(prompt, timeout=timeout)
            
            if result and result['success']:
                interaction_details['raw_response'] = result['raw_response']
                return {
                    'code': result['code'],
                    'raw_response': result['raw_response'],
                    'interaction_details': interaction_details
                }
            else:
                error_msg = result.get('error', 'Unknown error') if result else 'No response from Cronus'
                interaction_details['error'] = error_msg
                return {
                    'code': None,
                    'raw_response': None,
                    'interaction_details': interaction_details
                }
                
        except Exception as e:
            error_msg = f"Error calling agent: {e}"
            self.log(error_msg, "ERROR")
            interaction_details['error'] = error_msg
            return {
                'code': None,
                'raw_response': None,
                'interaction_details': interaction_details
            }
    
    def extract_code(self, response: str, language: str) -> str:
        """Extract code from agent response (remove markdown, etc.)."""
        # Remove markdown code blocks if present
        lines = response.split('\n')
        code_lines = []
        in_code_block = False
        
        for line in lines:
            if line.strip().startswith('```'):
                in_code_block = not in_code_block
                continue
            if in_code_block or (not any(line.strip().startswith(x) for x in ['#', '//', '/*', '*'])):
                code_lines.append(line)
        
        return '\n'.join(code_lines).strip()
    
    def run_tests(self, exercise: Dict[str, Any], generated_code: str, 
                  language: str, work_dir: Path) -> Dict[str, Any]:
        """Run tests against generated code."""
        try:
            # Copy exercise files to work directory
            exercise_path = Path(exercise['path'])
            
            # Copy test files and supporting files
            for file in exercise_path.iterdir():
                if file.is_file():
                    shutil.copy2(file, work_dir)
            
            # Write generated code to appropriate file
            lang_config = self.config['languages'][language]
            
            if language == 'cpp':
                # Find the expected source file name from config
                impl_file = work_dir / f"{exercise['name'].replace('-', '_')}.cpp"
                with open(impl_file, 'w') as f:
                    f.write(generated_code)
                    
                # Try to compile and run tests
                test_cmd = lang_config.get('test_command', 'make test')
                
            elif language == 'python':
                impl_file = work_dir / f"{exercise['name'].replace('-', '_')}.py"
                with open(impl_file, 'w') as f:
                    f.write(generated_code)
                    
                test_cmd = lang_config.get('test_command', 'pytest')
                
            elif language == 'javascript':
                impl_file = work_dir / f"{exercise['name'].replace('-', '_')}.js"
                with open(impl_file, 'w') as f:
                    f.write(generated_code)
                    
                test_cmd = lang_config.get('test_command', 'npm test')
            
            # Run tests
            result = subprocess.run(
                test_cmd,
                shell=True,
                cwd=work_dir,
                capture_output=True,
                text=True,
                timeout=lang_config['timeout_seconds']
            )
            
            return {
                'passed': result.returncode == 0,
                'stdout': result.stdout,
                'stderr': result.stderr,
                'return_code': result.returncode
            }
            
        except subprocess.TimeoutExpired:
            return {
                'passed': False,
                'stdout': '',
                'stderr': 'Test execution timed out',
                'return_code': -1
            }
        except Exception as e:
            return {
                'passed': False,
                'stdout': '',
                'stderr': str(e),
                'return_code': -1
            }
    
    def evaluate_exercise(self, exercise_path: Path, language: str,
                         results_dir: Path) -> Dict[str, Any]:
        """Evaluate a single exercise."""
        start_time = time.time()
        
        # Load exercise
        exercise = self.load_exercise(exercise_path)
        if not exercise:
            return {'success': False, 'error': 'Failed to load exercise'}
        
        self.log(f"Evaluating {language} exercise: {exercise['name']}")
        
        # Create prompt
        prompt = self.create_agent_prompt(exercise, language)
        
        # Invoke agent
        self.log(f"Invoking agent for {exercise['name']}...")
        agent_start_time = time.time()
        agent_response = self.invoke_agent(prompt, language)
        agent_elapsed_time = time.time() - agent_start_time
        
        # Extract results from agent response dict
        generated_code_raw = agent_response.get('code')
        interaction_details = agent_response.get('interaction_details', {})
        
        if not generated_code_raw:
            return {
                'success': False,
                'error': 'Agent invocation failed - no code generated',
                'exercise': exercise['name'],
                'prompt': prompt,
                'agent_response': agent_response.get('raw_response'),
                'agent_elapsed_time': agent_elapsed_time,
                'interaction_details': interaction_details
            }
        
        # Extract code (remove markdown formatting, etc.)
        generated_code = self.extract_code(generated_code_raw, language)
        
        # Create temporary work directory for testing
        with tempfile.TemporaryDirectory() as temp_dir:
            work_dir = Path(temp_dir)
            
            # Run tests
            self.log(f"Running tests for {exercise['name']}...")
            test_results = self.run_tests(exercise, generated_code, language, work_dir)
        
        elapsed_time = time.time() - start_time
        
        # Compile results with detailed interaction info
        result = {
            'success': True,
            'exercise': exercise['name'],
            'language': language,
            'passed': test_results['passed'],
            'generated_code': generated_code,
            'test_output': {
                'stdout': test_results['stdout'],
                'stderr': test_results['stderr'],
                'return_code': test_results['return_code']
            },
            'elapsed_time': elapsed_time,
            'agent_elapsed_time': agent_elapsed_time,
            'timestamp': time.strftime('%Y-%m-%d %H:%M:%S'),
            # Additional details for debugging
            'interaction_details': {
                'prompt': prompt,
                'raw_response': agent_response.get('raw_response'),
                'extracted_code': generated_code,
                'instructions': exercise['instructions'],
                **interaction_details  # Merge in the detailed interaction log
            }
        }
        
        # Save individual exercise result
        exercise_result_file = results_dir / f"{exercise['name']}.json"
        with open(exercise_result_file, 'w') as f:
            json.dump(result, f, indent=2)
        
        # Save generated code if configured
        if self.config['evaluation'].get('save_generated_code', True):
            code_file = results_dir / f"{exercise['name']}_solution.{language}"
            with open(code_file, 'w') as f:
                f.write(generated_code)
        
        status = "PASSED" if test_results['passed'] else "FAILED"
        self.log(f"{exercise['name']}: {status} ({elapsed_time:.2f}s)")
        
        return result


def main():
    parser = argparse.ArgumentParser(description='Evaluate agent on Exercism exercises')
    parser.add_argument('--language', required=True, choices=['cpp', 'python', 'javascript'],
                       help='Programming language to evaluate')
    parser.add_argument('--exercises-dir', required=True, type=Path,
                       help='Directory containing Exercism exercises')
    parser.add_argument('--results-dir', required=True, type=Path,
                       help='Directory to save results')
    parser.add_argument('--config', required=True, type=Path,
                       help='Configuration file')
    parser.add_argument('--exercise', help='Specific exercise to run (optional)')
    parser.add_argument('--verbose', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    # Load configuration
    try:
        with open(args.config, 'r') as f:
            config = json.load(f)
    except Exception as e:
        print(f"ERROR: Failed to load configuration: {e}")
        sys.exit(1)
    
    # Create results directory
    args.results_dir.mkdir(parents=True, exist_ok=True)
    
    # Initialize evaluator
    try:
        evaluator = ExerciseEvaluator(config, args.verbose)
    except RuntimeError as e:
        print(f"\nERROR: Failed to initialize evaluator:")
        print(f"{e}")
        print("\nEvaluation cannot continue without Cronus server.")
        sys.exit(1)
    except Exception as e:
        print(f"ERROR: Unexpected error during initialization: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
    
    # Find exercises to evaluate
    exercises_path = args.exercises_dir / 'exercises' / 'practice'
    
    if args.exercise:
        # Evaluate specific exercise
        exercise_path = exercises_path / args.exercise
        if not exercise_path.exists():
            print(f"Exercise not found: {exercise_path}")
            evaluator.cronus_client.stop_server()
            sys.exit(1)
        exercises = [exercise_path]
    else:
        # Evaluate all exercises
        exercises = [d for d in exercises_path.iterdir() if d.is_dir()]
        
        # Apply filters from config
        max_exercises = config['languages'][args.language].get('max_exercises')
        if max_exercises:
            exercises = exercises[:max_exercises]
    
    # Run evaluations with cleanup on exit
    results = []
    passed_count = 0
    
    try:
        for exercise_path in exercises:
            result = evaluator.evaluate_exercise(exercise_path, args.language, args.results_dir)
            results.append(result)
            if result.get('passed', False):
                passed_count += 1
    except KeyboardInterrupt:
        print("\n\nEvaluation interrupted by user")
        evaluator.cronus_client.stop_server()
        sys.exit(1)
    except Exception as e:
        print(f"\nERROR during evaluation: {e}")
        import traceback
        traceback.print_exc()
        evaluator.cronus_client.stop_server()
        sys.exit(1)
    finally:
        # Always stop the server when done
        evaluator.cronus_client.stop_server()
    
    # Save summary
    summary = {
        'language': args.language,
        'total': len(results),
        'passed': passed_count,
        'failed': len(results) - passed_count,
        'pass_rate': passed_count / len(results) if results else 0,
        'exercises': results
    }
    
    summary_file = args.results_dir / 'summary.json'
    with open(summary_file, 'w') as f:
        json.dump(summary, f, indent=2)
    
    print(f"\nEvaluation complete: {passed_count}/{len(results)} passed ({summary['pass_rate']:.1%})")
    print(f"Results saved to: {args.results_dir}")
    
    sys.exit(0 if passed_count == len(results) else 1)


if __name__ == '__main__':
    main()
