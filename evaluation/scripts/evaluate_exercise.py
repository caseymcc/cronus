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


class ExerciseEvaluator:
    """Evaluates agent performance on a single exercise."""
    
    def __init__(self, config: Dict[str, Any], verbose: bool = False):
        self.config = config
        self.verbose = verbose
        self.agent_endpoint = config['agent']['endpoint']
        
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
    
    def invoke_agent(self, prompt: str, language: str) -> Optional[str]:
        """Call the agent to generate a solution using OpenAI-compatible API."""
        try:
            api_type = self.config['agent'].get('api_type', 'cronus')
            
            if api_type == 'openai':
                # OpenAI-compatible API (llama.cpp, etc.)
                response = requests.post(
                    f"{self.agent_endpoint}/chat/completions",
                    json={
                        'messages': [
                            {
                                'role': 'system',
                                'content': f'You are an expert {language} programmer. Provide only code solutions without explanations.'
                            },
                            {
                                'role': 'user',
                                'content': prompt
                            }
                        ],
                        'max_tokens': self.config['agent']['max_tokens'],
                        'temperature': self.config['agent']['temperature'],
                        'stream': False
                    },
                    timeout=self.config['languages'][language]['timeout_seconds']
                )
                
                if response.status_code == 200:
                    result = response.json()
                    content = result['choices'][0]['message']['content']
                    return content
                else:
                    self.log(f"Agent request failed: {response.status_code} - {response.text}", "ERROR")
                    return None
            else:
                # Original Cronus API format
                response = requests.post(
                    f"{self.agent_endpoint}/api/generate",
                    json={
                        'prompt': prompt,
                        'language': language,
                        'model': self.config['agent']['model'],
                        'max_tokens': self.config['agent']['max_tokens'],
                        'temperature': self.config['agent']['temperature']
                    },
                    timeout=self.config['languages'][language]['timeout_seconds']
                )
                
                if response.status_code == 200:
                    result = response.json()
                    return result.get('code', result.get('response', ''))
                else:
                    self.log(f"Agent request failed: {response.status_code}", "ERROR")
                    return None
                
        except requests.exceptions.RequestException as e:
            self.log(f"Error calling agent: {e}", "ERROR")
            return None
    
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
        response = self.invoke_agent(prompt, language)
        agent_elapsed_time = time.time() - agent_start_time
        
        if not response:
            return {
                'success': False,
                'error': 'Agent invocation failed',
                'exercise': exercise['name'],
                'prompt': prompt,
                'agent_response': None,
                'agent_elapsed_time': agent_elapsed_time
            }
        
        # Extract code
        generated_code = self.extract_code(response, language)
        
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
                'raw_response': response,
                'extracted_code': generated_code,
                'instructions': exercise['instructions']
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
    with open(args.config, 'r') as f:
        config = json.load(f)
    
    # Create results directory
    args.results_dir.mkdir(parents=True, exist_ok=True)
    
    # Initialize evaluator
    evaluator = ExerciseEvaluator(config, args.verbose)
    
    # Find exercises to evaluate
    exercises_path = args.exercises_dir / 'exercises' / 'practice'
    
    if args.exercise:
        # Evaluate specific exercise
        exercise_path = exercises_path / args.exercise
        if not exercise_path.exists():
            print(f"Exercise not found: {exercise_path}")
            sys.exit(1)
        exercises = [exercise_path]
    else:
        # Evaluate all exercises
        exercises = [d for d in exercises_path.iterdir() if d.is_dir()]
        
        # Apply filters from config
        max_exercises = config['languages'][args.language].get('max_exercises')
        if max_exercises:
            exercises = exercises[:max_exercises]
    
    # Run evaluations
    results = []
    passed_count = 0
    
    for exercise_path in exercises:
        result = evaluator.evaluate_exercise(exercise_path, args.language, args.results_dir)
        results.append(result)
        if result.get('passed', False):
            passed_count += 1
    
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
