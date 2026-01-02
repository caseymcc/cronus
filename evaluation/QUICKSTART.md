# Cronus Agent Evaluation - Quick Start Guide

This guide will help you get started with evaluating the Cronus agent using Exercism exercises.

## Prerequisites

- Docker (for containerized builds)
- Python 3.8+ (for evaluation scripts)
- Git
- Cronus server running (or ability to start it)

## Step 1: Download Exercism Exercises

First, download the exercise repositories. This only needs to be done once (or when you want to update exercises):

```bash
cd /path/to/cronus
./evaluation/setup.sh
```

This will:
- Clone C++, Python, and JavaScript exercise repositories
- Create exercise indexes
- Take a few minutes on first run

## Step 2: Configure Agent Endpoint (Optional)

Edit `evaluation/config.json` to point to your LLM server:

```json
{
  "languages": {
    "python": {
      "enabled": true,
      "max_exercises": 3  // Start with fewer exercises for testing
    }
  },
  "agent": {
    "endpoint": "http://localhost:8080",
    "api_type": "cronus"  // or "openai" for OpenAI-compatible APIs
  }
}
```

For OpenAI-compatible APIs (llama.cpp, vLLM, etc.):
```json
{
  "agent": {
    "endpoint": "http://192.168.2.106:8000/v1",
    "api_type": "openai"
  }
}
```

## Step 3: Run Evaluation (Automatic Docker)

The evaluation automatically runs in Docker with all dependencies installed:

### Test with One Language First

```bash
# Run Python exercises only (recommended for first test)
./evaluation/run_evaluation.sh --language python
```

### Run All Evaluations

```bash
./evaluation/run_evaluation.sh
```

### How It Works

The script automatically:
1. Detects it's running on the host system
2. Launches the Cronus Docker container
3. Re-executes itself with all dependencies (pytest, jest, g++)
4. Runs the evaluation
5. Saves results to your local filesystem

No manual dependency installation required!

## Step 4: View Results

Results are saved in `evaluation/results/latest/`:

```bash
# Open HTML report in browser
xdg-open evaluation/results/latest/summary.html

# Or view JSON summary
cat evaluation/results/latest/summary.json | python3 -m json.tool
```

### Understanding the HTML Report

The interactive HTML report includes:

**Summary Dashboard**:
- Total exercises evaluated
- Pass/fail statistics
- Overall pass rate

**Per-Exercise Details** (expand with "📋 View Details"):
- 📝 **Exercise Instructions**: What the agent needed to solve
- 💬 **Prompt**: Exact prompt sent to the LLM
- 🤖 **Raw Response**: Complete unprocessed LLM output
- ✂️ **Extracted Code**: Code after cleanup
- 🧪 **Test Output**: What happened when tests ran
- ⚠️ **Errors**: Why tests failed (if applicable)

This detailed view is invaluable for:
- Debugging prompt engineering
- Understanding agent behavior
- Identifying code extraction issues
- Analyzing test failures

## Common Usage Patterns

### Run Specific Exercise

```bash
./evaluation/run_evaluation.sh --language python --exercise hello-world
```

### Dry Run (See What Would Execute)

```bash
./evaluation/run_evaluation.sh --dry-run
```

### Verbose Output

```bash
./evaluation/run_evaluation.sh --verbose --language cpp
```

### Clean Results Between Runs

```bash
# Clean just results
./evaluation/clean.sh --results

# Clean everything
./evaluation/clean.sh --all
```

## Troubleshooting

### Server Not Responding

```bash
# Check if server is running
curl http://localhost:8080/health

# Check server logs
./run_local.sh docker logs cronus_dev
```

### Exercises Not Found

```bash
# Re-run setup
./evaluation/setup.sh

# Verify exercises exist
ls -la evaluation/exercises/
```

### Python Dependencies Missing

```bash
# Install required Python packages
pip3 install requests pytest

# Or in Docker
./run_local.sh pip3 install requests pytest
```

### Permission Issues

```bash
# Make scripts executable
chmod +x evaluation/*.sh evaluation/scripts/*.py
```

## Understanding Results

### Summary Report

The HTML report shows:
- Overall pass rate
- Per-language breakdown
- Individual exercise results
- Timing information

### Detailed Results

Each exercise has a detailed JSON file:
```bash
cat evaluation/results/latest/cpp/hello-world.json
```

Contains:
- Generated code
- Test output
- Timing
- Success/failure status

## Next Steps

1. **Analyze Failures**: Review failed exercises to understand agent limitations
2. **Adjust Configuration**: Tune model parameters, timeouts, etc.
3. **Add Custom Exercises**: Extend with domain-specific tests
4. **Track Progress**: Run evaluations periodically to track improvements
5. **Compare Models**: Change agent configuration to compare different LLMs

## Tips

- Start with `max_exercises: 5` for faster initial testing
- Use `--dry-run` to preview what will run
- Archive important results before cleaning
- Run evaluations in Docker for consistent environment
- Review the generated code to understand agent behavior

## File Locations

- **Exercises**: `evaluation/exercises/` (gitignored, fetched from source)
- **Results**: `evaluation/results/` (gitignored)
- **Config**: `evaluation/config.json` (versioned)
- **Scripts**: `evaluation/scripts/` (versioned)
- **Templates**: `evaluation/templates/` (versioned)

## Continuous Evaluation

For ongoing development, consider:

```bash
# Weekly evaluation run
./evaluation/run_evaluation.sh
./evaluation/archive_results.sh weekly

# Before merging features
./evaluation/run_evaluation.sh --language cpp
./evaluation/compare_results.sh baseline feature-branch
```

## Support

- See `evaluation/README.md` for comprehensive documentation
- Check logs in `evaluation/results/latest/`
- Review individual exercise results for debugging
