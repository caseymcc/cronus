# Cronus Agent Integration for Evaluation System

## Overview

The evaluation system uses the Cronus agent server to solve programming exercises. This allows evaluation of the full Cronus agent system including specialized code generation agents, not just the underlying LLM.

## What Changed

### 1. New Cronus Client (`scripts/cronus_client.py`)

A Python client was created to manage the Cronus server lifecycle and handle SSE communication:

- **Automatic server management**: Starts/stops Cronus server as needed
- **SSE event listening**: Listens for real-time responses via Server-Sent Events
- **Request/response handling**: Sends code generation requests and collects results
- **Context manager support**: Clean resource management with `with` statement

### 2. Updated Evaluation Script (`scripts/evaluate_exercise.py`)

The core evaluation logic integrates with Cronus:

- Uses `CronusClient` to start the server
- Sends requests via `/api/input`
- Listens for responses via SSE
- Captures full interaction details

### 3. Updated Configuration (`config.json`)

Configuration fields for Cronus integration:

```json
{
  "agent": {
    "endpoint": "http://localhost:9000/api",
    "cronus_executable": "build/linux_x64_debug/server/cronus/cronus",
    "cronus_working_dir": "/tmp/cronus_eval"
  }
}
```

### 4. Docker Environment Updates

Updated `server/cronus/docker/Dockerfile` to include:
- `sseclient-py` package for SSE event handling
- `requests` package for HTTP communication
- `pytest` and `pytest-cov` for test execution

### 5. Documentation Updates

- **QUICKSTART.md**: Added Cronus build instructions
- **README.md**: Added architecture diagram
- **copilot-instructions.md**: Updated with Cronus integration details

## Architecture

```
┌─────────────────────┐
│ Evaluation Script   │
│ (Python)            │
└──────────┬──────────┘
           │
           │ CronusClient
           │
           v
┌─────────────────────┐     ┌──────────────────┐
│ Cronus Server       │────>│ Coder Agent      │
│ (REST API + SSE)    │     │ (Specialized     │
│ Port: 9000          │     │  prompts)        │
└──────────┬──────────┘     └────────┬─────────┘
           │                          │
           │                          v
           │                ┌──────────────────┐
           │                │ arbiterAI        │
           └───────────────>│ (Unified LLM API)│
                           └────────┬─────────┘
                                    │
                                    v
                           ┌──────────────────┐
                           │ LLM Backend      │
                           │ (llama.cpp, etc.)│
                           └──────────────────┘
```

## Usage

```bash
# 1. Build Cronus server
./run_local.sh ./generate.sh
./run_local.sh ninja -C build/linux_x64_debug

# 2. Run evaluation (auto-starts Cronus server)
./evaluation/run_evaluation.sh --language python
```

The evaluation system will:
1. Build the Cronus server (if not already built)
2. Start the Cronus server on port 9000
3. Run exercises, sending requests to Cronus
4. Capture full agent interactions
5. Stop the Cronus server when done

## Benefits

1. **Evaluates the full system**: Tests Cronus agents, not just the LLM
2. **Specialized prompts**: Uses agent-specific prompt engineering
3. **Real-world usage**: Tests the actual user-facing system
4. **Agent comparison**: Can evaluate different Cronus agent types

## Troubleshooting

### Cronus Server Won't Start

1. Check if Cronus is built:
   ```bash
   ./run_local.sh ninja -C build/linux_x64_debug
   ```

2. Check if port 9000 is available:
   ```bash
   netstat -tlnp | grep 9000
   ```

3. Check Cronus server logs in evaluation output

### SSE Connection Issues

1. Ensure `sseclient-py` is installed:
   ```bash
   pip install sseclient-py
   ```

2. Check firewall/network settings

3. Verify Cronus server is responding:
   ```bash
   curl http://localhost:9000/api/health
   ```

### No Response from Cronus

1. Check SSE event stream:
   ```bash
   curl -N http://localhost:9000/api/events
   ```

2. Increase timeout in config.json
3. Check Cronus server logs
4. Verify LLM backend is accessible from Cronus

## Future Enhancements

Potential improvements to the Cronus integration:

1. **Synchronous API endpoint**: Add `/api/generate_sync` for simpler evaluation
2. **Agent selection**: Specify which Cronus agent to use per exercise
3. **Multi-agent workflows**: Test agent collaboration on complex tasks
4. **Performance metrics**: Track agent overhead and response times
5. **Agent comparison reports**: Side-by-side comparison of different agents

## Implementation Details

### Key Classes

**CronusClient** (`scripts/cronus_client.py`):
- Manages Cronus server subprocess
- Handles SSE connection in background thread
- Provides `generate_code()` method for synchronous-like usage
- Implements context manager protocol

**ExerciseEvaluator** (`scripts/evaluate_exercise.py`):
- Initializes CronusClient based on config
- Routes requests to appropriate backend (Cronus vs OpenAI)
- Captures detailed interaction logs
- Manages server lifecycle

### SSE Event Handling

The SSE implementation uses a background thread to listen for events:

1. Thread starts when first request is made
2. Listens for `response` type events
3. Sets `response_complete` event when response received
4. Main thread waits with timeout for completion

### Server Lifecycle

1. **Startup**: 
   - Check if executable exists
   - Start subprocess with `--web --port 9000`
   - Poll `/api/health` until ready
   - Timeout after 30 seconds

2. **Operation**:
   - SSE thread listens for responses
   - Requests sent via POST to `/api/input`
   - Responses collected from SSE events

3. **Shutdown**:
   - Send SIGTERM to subprocess
   - Wait up to 5 seconds
   - SIGKILL if needed
   - Clean up resources

## Testing

### Test the Cronus Client Standalone

```bash
cd evaluation/scripts
python cronus_client.py ../../build/linux_x64_debug/server/cronus/cronus
```

### Test Evaluation with Cronus

```bash
# Run one exercise with verbose logging
./evaluation/run_evaluation.sh --language python --exercise hello-world
```

## References

- **Cronus REST API**: `docs/api-documentation.md`
- **Cronus Architecture**: `docs/project.md`
- **SSE Specification**: [Server-Sent Events (MDN)](https://developer.mozilla.org/en-US/docs/Web/API/Server-sent_events)
- **Exercism Exercises**: [Exercism GitHub](https://github.com/exercism)
