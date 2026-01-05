# Cronus Agent Evaluation - Quick Start Guide

This guide will help you get started with evaluating the Cronus agent using Exercism exercises.

## Prerequisites

- Docker (for containerized builds)
- Python 3.8+ (for evaluation scripts)
- Git
- Built Cronus server executable (automatically started by evaluation system)

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

## Step 2: Configure Settings (Optional)

The default configuration automatically starts the Cronus agent server during evaluation.

Edit `evaluation/config.json` to customize settings:

```json
{
  "languages": {
    "python": {
      "enabled": true,
      "max_exercises": 3  // Start with fewer exercises for testing
    }
  },
  "agent": {
    "endpoint": "http://localhost:9000/api",
    "cronus_executable": "build/linux_x64_debug/server/cronus/cronus",
    "cronus_working_dir": "/tmp/cronus_eval"
  }
}
```

## Step 3: Build Cronus

Build the Cronus server:

```bash
./run_local.sh ./generate.sh
./run_local.sh ninja -C build/linux_x64_debug
```

The evaluation system will automatically start and stop the Cronus server as needed.

## Step 4: Run Evaluation (Automatic Docker)

The evaluation script automatically manages Docker for you - no manual setup needed!

**Important**: The evaluation script will automatically verify that Cronus is built before starting. If not built, it will display an error with build instructions.

### Test with One Language First

```bash
# Run Python exercises only (recommended for first test)
./evaluation/run_evaluation.sh --language python
```

If Cronus is not built, you'll see:

```
ERROR: Cronus executable not found
Please build Cronus first:
  ./run_local.sh ./generate.sh
  ./run_local.sh ninja -C build/linux_x64_debug
```

### Run All Evaluations

```bash
./evaluation/run_evaluation.sh
```

### Rebuild Docker Image

If you've updated dependencies or Python packages:

```bash
# Rebuild Docker image (does not run evaluation)
./evaluation/run_evaluation.sh --rebuild

# Then run evaluation
./evaluation/run_evaluation.sh --language python
```

### How It Works

The `run_evaluation.sh` script automatically:
1. Checks if Docker image exists, builds if needed
2. Starts Docker container with all dependencies (pytest, jest, g++, websocket-client)
3. Mounts your project directory
4. **Verifies Cronus is built** (stops with error if not)
5. Starts the Cronus server inside container (via Python evaluation script)
6. Runs the evaluation with detailed interaction logging via WebSocket
7. Stops the Cronus server automatically
8. Saves results to your local filesystem (via mounted volume)

### Troubleshooting

**"Cronus executable not found"**:
```bash
./run_local.sh ./generate.sh
./run_local.sh ninja -C build/linux_x64_debug
```

**"Port 9000 already in use"**:
```bash
# Find and kill the process using port 9000
netstat -tlnp | grep 9000
kill <PID>
```

**"Cronus server crashed during evaluation"**:
- Check the error output in the terminal
- Verify all dependencies are installed
- Try running Cronus manually to see errors:
  ```bash
  ./run_local.sh ./build/linux_x64_debug/server/cronus/cronus --web --port 9000
  ```
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
curl http://localhost:9000/health

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
