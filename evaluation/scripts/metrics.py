#!/usr/bin/env python3
"""
Metrics calculation utilities for evaluation system.
Provides functions for scoring and analyzing results.
"""

from typing import Dict, List, Any, Optional
import re


class MetricsCalculator:
    """Calculate various metrics for code evaluation."""
    
    @staticmethod
    def calculate_correctness(test_results: Dict[str, Any]) -> float:
        """
        Calculate correctness score based on test results.
        
        Args:
            test_results: Dictionary containing test output
            
        Returns:
            Score between 0.0 and 1.0
        """
        if test_results.get('passed', False):
            return 1.0
        
        # Try to parse partial pass information from test output
        stdout = test_results.get('stdout', '')
        stderr = test_results.get('stderr', '')
        
        # Look for test framework output patterns
        # Example: "5 passed, 2 failed"
        passed_pattern = r'(\d+)\s+passed'
        failed_pattern = r'(\d+)\s+failed'
        
        passed_match = re.search(passed_pattern, stdout + stderr)
        failed_match = re.search(failed_pattern, stdout + stderr)
        
        if passed_match and failed_match:
            passed = int(passed_match.group(1))
            failed = int(failed_match.group(1))
            total = passed + failed
            return passed / total if total > 0 else 0.0
        
        return 0.0
    
    @staticmethod
    def calculate_code_quality(code: str, language: str) -> float:
        """
        Calculate code quality score based on simple heuristics.
        
        Args:
            code: The generated code
            language: Programming language
            
        Returns:
            Score between 0.0 and 1.0
        """
        if not code.strip():
            return 0.0
        
        score = 1.0
        lines = code.split('\n')
        
        # Check for basic quality indicators
        
        # Penalize very long lines
        long_lines = sum(1 for line in lines if len(line) > 120)
        if long_lines > 0:
            score -= 0.1 * min(long_lines / len(lines), 0.3)
        
        # Reward comments (but not too many)
        comment_chars = {'cpp': '//', 'python': '#', 'javascript': '//'}
        comment_char = comment_chars.get(language, '//')
        comment_lines = sum(1 for line in lines if line.strip().startswith(comment_char))
        comment_ratio = comment_lines / len(lines) if lines else 0
        
        if 0.05 <= comment_ratio <= 0.3:
            score += 0.1
        elif comment_ratio > 0.5:
            score -= 0.1  # Too many comments
        
        # Check for proper naming (basic heuristic)
        # Look for descriptive variable names (length > 2)
        if language == 'python':
            var_pattern = r'\b[a-z_][a-z0-9_]{2,}\b'
        else:
            var_pattern = r'\b[a-zA-Z_][a-zA-Z0-9_]{2,}\b'
        
        descriptive_names = len(re.findall(var_pattern, code))
        if descriptive_names > len(lines):
            score += 0.05
        
        # Penalize code smell patterns
        if 'TODO' in code or 'FIXME' in code:
            score -= 0.1
        
        return max(0.0, min(1.0, score))
    
    @staticmethod
    def calculate_completeness(code: str, exercise_config: Dict[str, Any]) -> float:
        """
        Check if all required functions/classes are implemented.
        
        Args:
            code: The generated code
            exercise_config: Exercise configuration from .meta/config.json
            
        Returns:
            Score between 0.0 and 1.0
        """
        # This is a simplified check
        # In a real implementation, you'd parse the exercise requirements
        # and verify all required functions/classes exist
        
        if not code.strip():
            return 0.0
        
        # Basic check: does it have function definitions?
        has_functions = (
            'def ' in code or  # Python
            'function ' in code or  # JavaScript
            '() {' in code or  # C++/JavaScript
            '()const' in code  # C++
        )
        
        if not has_functions:
            return 0.0
        
        # Check for common structure
        has_return = 'return' in code
        
        score = 0.5
        if has_return:
            score += 0.5
        
        return score
    
    @staticmethod
    def calculate_performance_score(elapsed_time: float, timeout: float) -> float:
        """
        Calculate performance score based on execution time.
        
        Args:
            elapsed_time: Time taken to run tests
            timeout: Maximum allowed time
            
        Returns:
            Score between 0.0 and 1.0
        """
        if elapsed_time >= timeout:
            return 0.0
        
        # Score decreases as time approaches timeout
        # 0-10% of timeout: 1.0
        # 50% of timeout: 0.75
        # 90% of timeout: 0.1
        # 100% timeout: 0.0
        
        ratio = elapsed_time / timeout
        
        if ratio < 0.1:
            return 1.0
        elif ratio < 0.5:
            return 1.0 - (ratio - 0.1) * 0.625  # Linear decline
        else:
            return max(0.0, 0.75 - (ratio - 0.5) * 1.5)
    
    @staticmethod
    def calculate_overall_score(
        correctness: float,
        quality: float,
        completeness: float,
        performance: float,
        weights: Optional[Dict[str, float]] = None
    ) -> float:
        """
        Calculate weighted overall score.
        
        Args:
            correctness: Correctness score (0-1)
            quality: Code quality score (0-1)
            completeness: Completeness score (0-1)
            performance: Performance score (0-1)
            weights: Optional custom weights
            
        Returns:
            Overall score between 0.0 and 1.0
        """
        if weights is None:
            weights = {
                'correctness': 0.5,
                'code_quality': 0.2,
                'performance': 0.2,
                'completeness': 0.1
            }
        
        score = (
            correctness * weights.get('correctness', 0.5) +
            quality * weights.get('code_quality', 0.2) +
            performance * weights.get('performance', 0.2) +
            completeness * weights.get('completeness', 0.1)
        )
        
        return max(0.0, min(1.0, score))
    
    @staticmethod
    def analyze_error_patterns(test_output: str) -> List[str]:
        """
        Analyze test output to identify common error patterns.
        
        Args:
            test_output: Combined stdout/stderr from tests
            
        Returns:
            List of identified error patterns
        """
        patterns = []
        
        error_indicators = {
            'syntax': ['SyntaxError', 'syntax error', 'parse error'],
            'type': ['TypeError', 'type error', 'type mismatch'],
            'logic': ['AssertionError', 'assertion failed', 'expected'],
            'runtime': ['RuntimeError', 'runtime error', 'exception'],
            'import': ['ImportError', 'ModuleNotFoundError', 'cannot find'],
            'compile': ['compilation failed', 'undefined reference', 'undeclared'],
            'timeout': ['timeout', 'timed out', 'exceeded'],
        }
        
        for category, indicators in error_indicators.items():
            if any(indicator.lower() in test_output.lower() for indicator in indicators):
                patterns.append(category)
        
        return patterns
    
    @staticmethod
    def format_score_report(
        exercise_name: str,
        scores: Dict[str, float],
        overall: float
    ) -> str:
        """
        Format a human-readable score report.
        
        Args:
            exercise_name: Name of the exercise
            scores: Dictionary of individual scores
            overall: Overall score
            
        Returns:
            Formatted report string
        """
        report = f"\n{'='*60}\n"
        report += f"Exercise: {exercise_name}\n"
        report += f"{'='*60}\n\n"
        
        report += "Individual Scores:\n"
        report += "-" * 40 + "\n"
        
        for metric, score in scores.items():
            bar_length = int(score * 20)
            bar = '█' * bar_length + '░' * (20 - bar_length)
            report += f"{metric.ljust(15)}: [{bar}] {score:.2%}\n"
        
        report += "-" * 40 + "\n"
        bar_length = int(overall * 20)
        bar = '█' * bar_length + '░' * (20 - bar_length)
        report += f"{'OVERALL'.ljust(15)}: [{bar}] {overall:.2%}\n"
        report += "=" * 60 + "\n"
        
        return report


def main():
    """Example usage of metrics calculator."""
    calc = MetricsCalculator()
    
    # Example test results
    test_results = {
        'passed': True,
        'stdout': '5 passed, 0 failed',
        'stderr': ''
    }
    
    code = """
def hello_world():
    '''Return a greeting'''
    return 'Hello, World!'
"""
    
    # Calculate scores
    correctness = calc.calculate_correctness(test_results)
    quality = calc.calculate_code_quality(code, 'python')
    completeness = calc.calculate_completeness(code, {})
    performance = calc.calculate_performance_score(1.5, 300)
    
    overall = calc.calculate_overall_score(
        correctness, quality, completeness, performance
    )
    
    # Print report
    report = calc.format_score_report(
        'hello-world',
        {
            'Correctness': correctness,
            'Quality': quality,
            'Completeness': completeness,
            'Performance': performance
        },
        overall
    )
    
    print(report)


if __name__ == '__main__':
    main()
