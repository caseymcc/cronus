#!/usr/bin/env python3
"""
Generate evaluation reports from results.
Supports both HTML and JSON output formats.
"""

import argparse
import json
import sys
from pathlib import Path
from typing import Dict, List, Any
from datetime import datetime


class ReportGenerator:
    """Generates evaluation reports from results."""
    
    def __init__(self, results_dir: Path):
        self.results_dir = results_dir
        self.data = self.load_results()
    
    def load_results(self) -> Dict[str, Any]:
        """Load all result files from the results directory."""
        data = {
            'languages': {},
            'timestamp': datetime.now().isoformat(),
            'total_exercises': 0,
            'total_passed': 0,
            'total_failed': 0
        }
        
        # Load results for each language
        for lang_dir in self.results_dir.iterdir():
            if not lang_dir.is_dir():
                continue
            
            lang = lang_dir.name
            summary_file = lang_dir / 'summary.json'
            
            if summary_file.exists():
                with open(summary_file, 'r') as f:
                    lang_data = json.load(f)
                    data['languages'][lang] = lang_data
                    data['total_exercises'] += lang_data.get('total', 0)
                    data['total_passed'] += lang_data.get('passed', 0)
                    data['total_failed'] += lang_data.get('failed', 0)
        
        if data['total_exercises'] > 0:
            data['overall_pass_rate'] = data['total_passed'] / data['total_exercises']
        else:
            data['overall_pass_rate'] = 0.0
        
        return data
    
    def generate_json(self, output_file: Path):
        """Generate JSON report."""
        with open(output_file, 'w') as f:
            json.dump(self.data, f, indent=2)
        print(f"JSON report saved to: {output_file}")
    
    def generate_html(self, output_file: Path):
        """Generate HTML report."""
        html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cronus Agent Evaluation Report</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}
        
        body {{
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            line-height: 1.6;
            color: #333;
            background: #f5f5f5;
            padding: 20px;
        }}
        
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        
        h1 {{
            color: #2c3e50;
            margin-bottom: 10px;
            border-bottom: 3px solid #3498db;
            padding-bottom: 10px;
        }}
        
        h2 {{
            color: #34495e;
            margin-top: 30px;
            margin-bottom: 15px;
        }}
        
        .timestamp {{
            color: #7f8c8d;
            font-size: 0.9em;
            margin-bottom: 30px;
        }}
        
        .summary {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }}
        
        .summary-card {{
            background: #ecf0f1;
            padding: 20px;
            border-radius: 6px;
            text-align: center;
        }}
        
        .summary-card h3 {{
            color: #7f8c8d;
            font-size: 0.9em;
            text-transform: uppercase;
            margin-bottom: 10px;
        }}
        
        .summary-card .value {{
            font-size: 2em;
            font-weight: bold;
            color: #2c3e50;
        }}
        
        .summary-card.pass {{
            background: #d5f4e6;
        }}
        
        .summary-card.pass .value {{
            color: #27ae60;
        }}
        
        .summary-card.fail {{
            background: #fadbd8;
        }}
        
        .summary-card.fail .value {{
            color: #e74c3c;
        }}
        
        .language-section {{
            margin-bottom: 30px;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            overflow: hidden;
        }}
        
        .language-header {{
            background: #3498db;
            color: white;
            padding: 15px 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }}
        
        .language-header h3 {{
            margin: 0;
        }}
        
        .language-stats {{
            background: white;
            font-size: 0.9em;
        }}
        
        .progress-bar {{
            height: 30px;
            background: #ecf0f1;
            border-radius: 15px;
            overflow: hidden;
            margin: 10px 20px;
        }}
        
        .progress-fill {{
            height: 100%;
            background: linear-gradient(90deg, #27ae60, #2ecc71);
            transition: width 0.3s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
            font-size: 0.85em;
        }}
        
        .exercise-list {{
            padding: 20px;
        }}
        
        .exercise-item {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 15px;
            margin-bottom: 8px;
            background: #f8f9fa;
            border-radius: 4px;
            border-left: 4px solid #95a5a6;
        }}
        
        .exercise-item.passed {{
            border-left-color: #27ae60;
        }}
        
        .exercise-item.failed {{
            border-left-color: #e74c3c;
        }}
        
        .exercise-name {{
            font-weight: 500;
        }}
        
        .exercise-status {{
            font-size: 0.85em;
            padding: 4px 12px;
            border-radius: 12px;
            font-weight: bold;
        }}
        
        .exercise-status.passed {{
            background: #d5f4e6;
            color: #27ae60;
        }}
        
        .exercise-status.failed {{
            background: #fadbd8;
            color: #e74c3c;
        }}
        
        .exercise-time {{
            color: #7f8c8d;
            font-size: 0.85em;
        }}
        
        .details {{
            margin-top: 10px;
        }}
        
        details {{
            background: #f8f9fa;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            padding: 10px;
            margin-top: 10px;
        }}
        
        summary {{
            cursor: pointer;
            font-weight: 500;
            color: #3498db;
            padding: 5px;
            user-select: none;
        }}
        
        summary:hover {{
            background: #ecf0f1;
            border-radius: 4px;
        }}
        
        .detail-content {{
            margin-top: 10px;
            padding: 10px;
            background: white;
            border-radius: 4px;
            font-family: 'Courier New', monospace;
            font-size: 0.85em;
        }}
        
        .detail-section {{
            margin-bottom: 15px;
        }}
        
        .detail-section h4 {{
            color: #2c3e50;
            margin-bottom: 5px;
            font-size: 0.9em;
        }}
        
        .code-block {{
            background: #2c3e50;
            color: #ecf0f1;
            padding: 10px;
            border-radius: 4px;
            overflow-x: auto;
            white-space: pre-wrap;
            word-wrap: break-word;
        }}
        
        .error-block {{
            background: #fadbd8;
            color: #c0392b;
            padding: 10px;
            border-radius: 4px;
            overflow-x: auto;
            white-space: pre-wrap;
        }}
        
        .info-block {{
            background: #d6eaf8;
            color: #21618c;
            padding: 10px;
            border-radius: 4px;
            overflow-x: auto;
            white-space: pre-wrap;
        }}
        
        footer {{
            margin-top: 40px;
            padding-top: 20px;
            border-top: 1px solid #e0e0e0;
            text-align: center;
            color: #7f8c8d;
            font-size: 0.9em;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>🤖 Cronus Agent Evaluation Report</h1>
        <div class="timestamp">Generated: {self.data['timestamp']}</div>
        
        <div class="summary">
            <div class="summary-card">
                <h3>Total Exercises</h3>
                <div class="value">{self.data['total_exercises']}</div>
            </div>
            <div class="summary-card pass">
                <h3>Passed</h3>
                <div class="value">{self.data['total_passed']}</div>
            </div>
            <div class="summary-card fail">
                <h3>Failed</h3>
                <div class="value">{self.data['total_failed']}</div>
            </div>
            <div class="summary-card">
                <h3>Pass Rate</h3>
                <div class="value">{self.data['overall_pass_rate']:.1%}</div>
            </div>
        </div>
        
        <h2>Results by Language</h2>
"""
        
        # Add language sections
        for lang, lang_data in self.data['languages'].items():
            total = lang_data.get('total', 0)
            passed = lang_data.get('passed', 0)
            pass_rate = lang_data.get('pass_rate', 0)
            
            html += f"""
        <div class="language-section">
            <div class="language-header">
                <h3>{lang.upper()}</h3>
                <span>{passed}/{total} passed ({pass_rate:.1%})</span>
            </div>
            <div class="progress-bar">
                <div class="progress-fill" style="width: {pass_rate*100}%">
                    {pass_rate:.1%}
                </div>
            </div>
            <div class="exercise-list">
"""
            
            # Add exercise items
            for exercise in lang_data.get('exercises', []):
                status = 'passed' if exercise.get('passed', False) else 'failed'
                status_text = 'PASSED' if exercise.get('passed', False) else 'FAILED'
                elapsed = exercise.get('elapsed_time', 0)
                agent_elapsed = exercise.get('agent_elapsed_time', 0)
                
                # Get interaction details if available
                interaction = exercise.get('interaction_details', {})
                prompt = interaction.get('prompt', 'N/A')
                raw_response = interaction.get('raw_response', 'N/A')
                extracted_code = interaction.get('extracted_code', exercise.get('generated_code', 'N/A'))
                instructions = interaction.get('instructions', 'N/A')
                test_output = exercise.get('test_output', {})
                test_stdout = test_output.get('stdout', '')
                test_stderr = test_output.get('stderr', '')
                
                # Escape HTML in code blocks
                import html as html_module
                prompt_escaped = html_module.escape(prompt)
                raw_response_escaped = html_module.escape(raw_response)
                extracted_code_escaped = html_module.escape(extracted_code)
                instructions_escaped = html_module.escape(instructions)
                test_stdout_escaped = html_module.escape(test_stdout)
                test_stderr_escaped = html_module.escape(test_stderr)
                
                html += f"""
                <div class="exercise-item {status}">
                    <div style="flex-grow: 1;">
                        <div style="display: flex; justify-content: space-between; align-items: center;">
                            <span class="exercise-name">{exercise.get('exercise', 'Unknown')}</span>
                            <div>
                                <span class="exercise-time">Total: {elapsed:.2f}s | Agent: {agent_elapsed:.2f}s</span>
                                <span class="exercise-status {status}">{status_text}</span>
                            </div>
                        </div>
                        
                        <details class="details">
                            <summary>📋 View Details</summary>
                            <div class="detail-content">
                                
                                <div class="detail-section">
                                    <h4>📝 Exercise Instructions</h4>
                                    <div class="info-block">{instructions_escaped}</div>
                                </div>
                                
                                <div class="detail-section">
                                    <h4>💬 Prompt Sent to Agent</h4>
                                    <div class="info-block">{prompt_escaped}</div>
                                </div>
                                
                                <div class="detail-section">
                                    <h4>🤖 Raw Agent Response</h4>
                                    <div class="code-block">{raw_response_escaped}</div>
                                </div>
                                
                                <div class="detail-section">
                                    <h4>✂️ Extracted Code</h4>
                                    <div class="code-block">{extracted_code_escaped}</div>
                                </div>
                                
                                <div class="detail-section">
                                    <h4>🧪 Test Output (stdout)</h4>
                                    <div class="code-block">{test_stdout_escaped if test_stdout else '(no output)'}</div>
                                </div>
                                
                                {f'''<div class="detail-section">
                                    <h4>⚠️ Test Errors (stderr)</h4>
                                    <div class="error-block">{test_stderr_escaped}</div>
                                </div>''' if test_stderr else ''}
                                
                            </div>
                        </details>
                    </div>
                </div>
"""
            
            html += """
            </div>
        </div>
"""
        
        html += """
        <footer>
            <p>Cronus Agent Evaluation System</p>
            <p>Powered by Exercism exercises</p>
        </footer>
    </div>
</body>
</html>
"""
        
        with open(output_file, 'w') as f:
            f.write(html)
        print(f"HTML report saved to: {output_file}")


def main():
    parser = argparse.ArgumentParser(description='Generate evaluation report')
    parser.add_argument('--results-dir', required=True, type=Path,
                       help='Directory containing evaluation results')
    parser.add_argument('--output', required=True, type=Path,
                       help='Output file path')
    parser.add_argument('--format', required=True, choices=['html', 'json'],
                       help='Report format')
    
    args = parser.parse_args()
    
    if not args.results_dir.exists():
        print(f"Results directory not found: {args.results_dir}")
        sys.exit(1)
    
    generator = ReportGenerator(args.results_dir)
    
    if args.format == 'json':
        generator.generate_json(args.output)
    elif args.format == 'html':
        generator.generate_html(args.output)
    
    sys.exit(0)


if __name__ == '__main__':
    main()
