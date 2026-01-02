# Cronus Agent Evaluation System

This directory contains the infrastructure for evaluating Cronus agent performance using Exercism exercises.

## Overview

The evaluation system tests the agent's ability to solve programming challenges across multiple languages (C++, Python, JavaScript) using exercises from the Exercism platform.

## Directory Structure

```
evaluation/
├── README.md                 # This file
├── setup.sh                 # Download and setup Exercism exercises
├── run_evaluation.sh        # Run evaluations
├── clean.sh                 # Clean evaluation results
├── config.json              # Evaluation configuration
├── exercises/               # Downloaded Exercism exercises (gitignored)
│   ├── cpp/
│   ├── python/
│   └── javascript/
├── results/                 # Evaluation results (gitignored)
│   ├── latest/
│   └── archive/
├── scripts/                 # Helper scripts
│   ├── evaluate_exercise.py
│   ├── generate_report.py
│   └── metrics.py
└── templates/               # Prompt templates for evaluation
    ├── cpp_prompt.txt
    ├── python_prompt.txt
    └── javascript_prompt.txt
```

## Setup

### 1. Download Exercism Exercises

The exercises are stored outside build directories to persist across rebuilds:

```bash
# From project root
./evaluation/setup.sh
```

This will:
- Clone Exercism repositories to `evaluation/exercises/`
- Extract relevant exercise metadata
- Create exercise indexes
- Only download if not already present

### 2. Configure Evaluation

Edit `evaluation/config.json` to configure:
- Which exercises to run
- Model settings
- Timeout values
- Scoring criteria

## Running Evaluations

**All evaluations automatically run in Docker** - no manual dependency installation needed!

### Run All Evaluations

```bash
./evaluation/run_evaluation.sh
```

### Run Specific Language

```bash
./evaluation/run_evaluation.sh --language cpp
./evaluation/run_evaluation.sh --language python
./evaluation/run_evaluation.sh --language javascript
```

### Run Specific Exercises

```bash
./evaluation/run_evaluation.sh --language cpp --exercise hello-world
```

### How It Works

The evaluation script automatically:
1. Detects if it's running on the host
2. Launches the Cronus Docker container
3. Re-executes itself inside the container with all dependencies available
4. Runs the evaluation
5. Saves results back to the host filesystem

## Evaluation Process

For each exercise, the system:

1. **Loads Exercise**: Reads instructions and test cases
2. **Generates Prompt**: Creates a prompt from the exercise description
3. **Invokes Agent**: Calls the Cronus agent to generate solution
4. **Validates Solution**: Runs tests against the generated code
5. **Scores Result**: Records pass/fail and performance metrics
6. **Generates Report**: Creates detailed evaluation report with full interaction details

## Metrics

The evaluation tracks:

- **Correctness**: Percentage of tests passed
- **Completeness**: Whether all required functions are implemented
- **Code Quality**: Linting and style checks
- **Performance**: Execution time for tests
- **Agent Response Time**: Time for LLM to generate solution
- **Total Time**: End-to-end evaluation time

## Results

Results are stored in `evaluation/results/` with timestamps:

```
results/
├── latest/                          # Symlink to most recent run
│   ├── summary.json                # Machine-readable summary
│   ├── summary.html                # Interactive HTML report
│   └── exercises/
│       ├── cpp/
│       ├── python/
│       └── javascript/
└── archive/
    ├── 2026-01-02_14-30-00/
    └── 2026-01-02_15-45-00/
```

### HTML Report Features

The HTML report (`summary.html`) includes:

- **Overall Statistics**: Total exercises, pass rate, aggregate metrics
- **Per-Language Breakdown**: Visual progress bars and statistics
- **Expandable Details** for each exercise (click "📋 View Details"):
  - 📝 **Exercise Instructions**: The original problem description
  - 💬 **Prompt Sent to Agent**: Exact prompt sent to the LLM
  - 🤖 **Raw Agent Response**: Complete unprocessed response from LLM
  - ✂️ **Extracted Code**: Code after markdown/cleanup extraction
  - 🧪 **Test Output**: Full stdout from test execution
  - ⚠️ **Test Errors**: stderr output if tests failed

This detailed view helps debug:
- Prompt engineering issues
- Code extraction problems
- Test failures
- Agent understanding of requirements

## Configuration Options

### config.json

```json
{
  "languages": {
    "cpp": {
      "enabled": true,
      "difficulty_levels": ["easy", "medium"],
      "max_exercises": 10,
      "timeout": 300
    },
    "python": {
      "enabled": true,
      "difficulty_levels": ["easy", "medium"],
      "max_exercises": 10,
      "timeout": 300
    },
    "javascript": {
      "enabled": true,
      "difficulty_levels": ["easy", "medium"],
      "max_exercises": 10,
      "timeout": 300
    }
  },
  "agent": {
    "model": "default",
    "max_tokens": 4096,
    "temperature": 0.2
  },
  "scoring": {
    "weights": {
      "correctness": 0.5,
      "code_quality": 0.2,
      "performance": 0.2,
      "completeness": 0.1
    }
  }
}
```

## Pros and Cons

### Current Approach: Separate Evaluation Directory

**Pros:**
- ✅ Exercises persist across rebuilds
- ✅ Clean separation from build system
- ✅ Can run evaluations without rebuilding
- ✅ Easy to version control evaluation config/scripts
- ✅ Results stored separately from code
- ✅ Can share exercises across different branches
- ✅ Flexible - can run standalone or in Docker

**Cons:**
- ❌ Not integrated into CMake test framework
- ❌ Requires manual invocation
- ❌ Exercise downloads not managed by build system

### Alternative: CMake Integration

**Pros:**
- ✅ Integrated with `ninja test`
- ✅ Standard CMake test reporting
- ✅ Parallel test execution

**Cons:**
- ❌ Exercises in build directory could be deleted
- ❌ Harder to manage exercise downloads
- ❌ Less flexible for ad-hoc evaluation
- ❌ Mixes unit tests with evaluation tests

### Alternative: Dedicated Evaluation Server

**Pros:**
- ✅ Can run continuous evaluations
- ✅ Web dashboard for results
- ✅ Historical tracking

**Cons:**
- ❌ More complex infrastructure
- ❌ Requires dedicated resources
- ❌ Overkill for initial implementation

## Future Enhancements

- Add more programming languages (Rust, Go, TypeScript)
- Implement continuous evaluation pipeline
- Add comparative benchmarks against other LLMs
- Create web dashboard for results
- Add regression detection
- Support custom exercise sets
- Integrate with CI/CD pipeline
