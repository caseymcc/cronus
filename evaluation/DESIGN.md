# Evaluation System Design Decisions

This document explains the design decisions made for the Cronus agent evaluation system.

## Architecture Choice: Standalone Scripts vs Integration

### Selected Approach: Standalone Script-Based System

The evaluation system is implemented as standalone scripts in the `evaluation/` directory, separate from the main build system.

#### Rationale

**Why Standalone:**

1. **Persistent Exercises**: Exercises are downloaded once to `evaluation/exercises/` and persist across rebuilds. Build directories can be cleaned without losing downloaded data.

2. **Flexibility**: Can run evaluations:
   - Without rebuilding the project
   - On different branches easily
   - With different configurations
   - In or out of Docker

3. **Separation of Concerns**: 
   - Evaluation is a testing/benchmarking tool, not part of the application
   - Development workflow remains clean
   - No coupling between build and evaluation systems

4. **Easy Maintenance**: 
   - Self-contained configuration
   - Independent versioning
   - Simple to extend with new languages/exercises

5. **Resource Management**:
   - Large exercise repositories (~100s of MB) don't bloat build
   - Easy to share exercises across branches (single download)
   - Clear cleanup targets

**Why Not Other Approaches:**

#### Alternative 1: CMake Integration
```cmake
# Would look like:
add_custom_target(evaluation
    COMMAND ${PYTHON} evaluation/run.py
    DEPENDS cronus-server
)
```

**Rejected Because:**
- ❌ Exercises in `build/` would be deleted on rebuild
- ❌ Harder to run ad-hoc evaluations
- ❌ Mixes unit tests with evaluation benchmarks
- ❌ Build system complexity increased
- ❌ Less portable across different build configurations

#### Alternative 2: Dedicated Evaluation Server
```
evaluation-server/
├── web-ui/
├── database/
└── continuous-evaluation/
```

**Rejected Because:**
- ❌ Over-engineered for initial needs
- ❌ Requires additional infrastructure
- ❌ Adds deployment complexity
- ❌ Harder to set up and maintain
- ✅ Could be future enhancement

#### Alternative 3: In-Tree Exercise Storage
```
evaluation/
└── exercises/  # In version control
    ├── cpp/
    ├── python/
    └── javascript/
```

**Rejected Because:**
- ❌ Bloats repository size (100s of MB)
- ❌ Duplicates upstream Exercism content
- ❌ Updates require manual sync
- ❌ Licensing concerns (derivative work)
- ❌ Version control churn

## Implementation Decisions

### 1. Python for Evaluation Logic

**Why Python:**
- ✅ Easy scripting for file/process management
- ✅ Good JSON/HTTP libraries
- ✅ Cross-platform
- ✅ Available in Docker container
- ✅ Familiar to most developers

**vs C++:**
- Would require compilation
- Overkill for orchestration tasks
- Harder to modify on-the-fly

**vs Bash only:**
- Complex logic gets messy
- Error handling difficult
- Less portable

### 2. Separate Bash Runner Scripts

**Why Bash Wrappers:**
- ✅ Simple entry points
- ✅ Environment setup
- ✅ User-friendly interface
- ✅ Shell integration

**Structure:**
```
setup.sh          → Download exercises (bash)
run_evaluation.sh → Orchestrate evaluation (bash + python)
evaluate_*.py     → Core logic (python)
```

### 3. Configuration in JSON

**Why JSON:**
- ✅ Human-readable
- ✅ Easy to edit
- ✅ Standard format
- ✅ Good Python/JS support

**vs YAML:**
- JSON simpler, no dependencies
- YAML adds another tool

**vs TOML:**
- JSON more universal
- Better tooling support

### 4. Results Storage Structure

```
results/
├── latest/           # Symlink to most recent
└── archive/
    └── YYYY-MM-DD_HH-MM-SS/
        ├── summary.{json,html}
        ├── cpp/
        │   ├── exercise1.json
        │   └── exercise1_solution.cpp
        ├── python/
        └── javascript/
```

**Why This Structure:**
- ✅ Timestamped archives
- ✅ Easy to compare runs
- ✅ `latest/` for convenience
- ✅ Per-exercise details
- ✅ Generated code saved for review

### 5. Dual Report Format (JSON + HTML)

**Why Both:**
- JSON: Machine-readable, automation, analysis
- HTML: Human-readable, shareable, visual

**Generation:**
- Single source (JSON results)
- HTML generated from JSON
- Consistent data

## Exercise Management

### Download Strategy

**Selected: On-Demand Git Clone**

```bash
# exercises/ not in version control
git clone --depth 1 https://github.com/exercism/cpp.git
```

**Why:**
- ✅ Always get latest exercises
- ✅ Shallow clone saves space
- ✅ Can update easily
- ✅ No licensing issues

**Update Strategy:**
```bash
git fetch origin
git reset --hard origin/main
```

### Exercise Selection

**Filtering:**
```json
{
  "max_exercises": 10,
  "difficulty_levels": [1, 2, 3],
  "skip_exercises": ["legacy-exercise"],
  "only_exercises": []  // Optional whitelist
}
```

**Why Configurable:**
- ✅ Quick smoke tests (5 exercises)
- ✅ Full evaluation (all exercises)
- ✅ Focused testing (specific difficulty)
- ✅ Regression testing (specific exercises)

## Agent Integration

### API Design Decision

**Selected: HTTP REST API**

```python
response = requests.post(
    f"{agent_endpoint}/api/generate",
    json={'prompt': prompt, 'language': lang}
)
```

**Why:**
- ✅ Language-agnostic
- ✅ Can test any agent implementation
- ✅ Easy to mock/stub for testing
- ✅ Network-transparent

**vs Direct Library Import:**
- Would couple evaluation to implementation
- Harder to test different versions
- Language-locked to Python/C++

### Prompt Engineering

**Template-Based:**
```
templates/
├── cpp_prompt.txt
├── python_prompt.txt
└── javascript_prompt.txt
```

**Why:**
- ✅ Easy to iterate on prompts
- ✅ Version-controlled
- ✅ Language-specific instructions
- ✅ Can A/B test prompts

## Testing and Validation

### Test Execution Strategy

**Per-Exercise Temp Directory:**
```python
with tempfile.TemporaryDirectory() as work_dir:
    # Copy exercise files
    # Write generated code
    # Run tests
    # Cleanup automatic
```

**Why:**
- ✅ Isolated execution
- ✅ No pollution between tests
- ✅ Parallel-safe (future)
- ✅ Automatic cleanup

### Metrics Calculation

**Multi-Dimensional Scoring:**
- Correctness (50%): Tests passed
- Code Quality (20%): Style, comments, patterns
- Performance (20%): Execution time
- Completeness (10%): Required functions present

**Why Weighted:**
- ✅ Correctness most important
- ✅ Quality matters for real use
- ✅ Performance indicates efficiency
- ✅ Completeness catches partial solutions

## Future Considerations

### Possible Enhancements

1. **Parallel Execution**: Use Python multiprocessing for faster evaluation
2. **CI/CD Integration**: GitHub Actions for automated evaluation
3. **Historical Tracking**: Database for trend analysis
4. **Web Dashboard**: Real-time monitoring and visualization
5. **Custom Exercise Sets**: Domain-specific evaluation suites
6. **Comparative Analysis**: Side-by-side LLM comparison
7. **Regression Detection**: Alert on performance degradation

### Migration Paths

If needs change:

**To CMake Integration:**
```cmake
# Add evaluation target
include(evaluation/CMakeLists.txt)
```

**To CI/CD:**
```yaml
# .github/workflows/evaluate.yml
- name: Run Evaluation
  run: ./evaluation/run_evaluation.sh
```

**To Dedicated Service:**
- Current scripts become client
- Add API server layer
- Keep data model intact

## Conclusion

The standalone script-based approach provides:
- ✅ Simplicity for initial implementation
- ✅ Flexibility for various use cases
- ✅ Clear separation of concerns
- ✅ Easy maintenance and extension
- ✅ Migration path for future needs

This design favors **pragmatism over perfection**, enabling rapid iteration while maintaining a clear path for future enhancements.
