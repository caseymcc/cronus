# Task 007: Agent Comparison System

**Status**: Todo  
**Priority**: Medium  
**Estimated Effort**: 4-5 days  
**Dependencies**: Task 006 (Multi-Agent Infrastructure)

## Objective

Implement a comprehensive agent comparison system that collects metrics, generates diffs between agent outputs, and produces detailed comparison reports for evaluating agent performance in multi-agent mode.

## Requirements

### Metrics Collection

Track multiple dimensions of agent performance:
- **Correctness**: Test pass rate, compilation success
- **Performance**: Execution time, memory usage
- **Code Quality**: Linter score, complexity metrics
- **Test Coverage**: Lines covered, branch coverage
- **Response Time**: Time to generate code
- **Resource Usage**: CPU, memory, disk I/O

### Comparison Features

- **Code Diffs**: Compare file changes between agents
- **Metric Comparison**: Side-by-side metric comparison
- **Report Generation**: HTML and JSON reports
- **Visualization**: Charts and graphs for metrics
- **Historical Tracking**: Track metrics over time

## Implementation Tasks

- [ ] Create `MetricsCollector` class
  - [ ] Collect correctness metrics (tests, compilation)
  - [ ] Collect performance metrics (time, memory)
  - [ ] Collect code quality metrics (linting, complexity)
  - [ ] Collect coverage metrics (optional)
  - [ ] Store metrics in `.cronus/comparison/metrics.json`

- [ ] Create `AgentComparator` class
  - [ ] Compare code changes (git diff)
  - [ ] Compare metrics across agents
  - [ ] Generate comparison summaries
  - [ ] Identify best performing agent
  - [ ] Statistical analysis (mean, median, std dev)

- [ ] Implement report generation
  - [ ] HTML report with interactive charts
  - [ ] JSON report for programmatic access
  - [ ] Markdown summary report
  - [ ] Export to CSV for analysis

- [ ] Create visualization components
  - [ ] Bar charts for metric comparison
  - [ ] Line charts for time series
  - [ ] Diff viewer for code changes
  - [ ] Summary dashboard

- [ ] Implement diff generation
  - [ ] Per-file diffs between agents
  - [ ] Unified diff format
  - [ ] Side-by-side diff display
  - [ ] Syntax highlighting in diffs

- [ ] CLI commands
  - [ ] `cronus compare <agent1> <agent2> [...]` - Compare agents
  - [ ] `cronus compare --all` - Compare all agents
  - [ ] `cronus compare --metric=<metric>` - Compare specific metric
  - [ ] `cronus metrics <agent-id>` - Show agent metrics
  - [ ] `cronus report generate` - Generate comparison report
  - [ ] `cronus report view` - Open latest report in browser

- [ ] API endpoints
  - [ ] `GET /api/comparison` - Get comparison data
  - [ ] `GET /api/comparison/metrics` - Get all metrics
  - [ ] `GET /api/agents/{id}/metrics` - Get agent-specific metrics
  - [ ] `POST /api/comparison/generate` - Generate comparison report
  - [ ] `GET /api/comparison/reports` - List available reports
  - [ ] `GET /api/comparison/reports/{id}` - Get specific report

## Metrics Schema

```json
{
  "agent_id": "agent-alpha",
  "timestamp": "2026-01-03T12:00:00Z",
  "metrics": {
    "correctness": {
      "tests_passed": 45,
      "tests_failed": 2,
      "compilation_success": true,
      "syntax_errors": 0
    },
    "performance": {
      "execution_time_ms": 1250,
      "memory_usage_mb": 128,
      "cpu_percent": 45.2
    },
    "code_quality": {
      "linter_score": 8.5,
      "complexity_score": 6.2,
      "maintainability_index": 72
    },
    "productivity": {
      "lines_added": 234,
      "lines_deleted": 89,
      "files_modified": 12,
      "commits": 5
    }
  }
}
```

## Report Example Structure

### HTML Report
```html
<!DOCTYPE html>
<html>
<head>
    <title>Agent Comparison Report</title>
    <script src="chart.js"></script>
</head>
<body>
    <h1>Multi-Agent Comparison</h1>
    <h2>Summary</h2>
    <table>
        <tr>
            <th>Agent</th>
            <th>Test Pass Rate</th>
            <th>Code Quality</th>
            <th>Response Time</th>
        </tr>
        <!-- Agent comparison rows -->
    </table>
    
    <h2>Metrics Visualization</h2>
    <canvas id="metricsChart"></canvas>
    
    <h2>Code Differences</h2>
    <div class="diff-viewer">
        <!-- Side-by-side diffs -->
    </div>
</body>
</html>
```

## Testing

- [ ] Unit tests for MetricsCollector
- [ ] Unit tests for AgentComparator
- [ ] Test metric collection accuracy
- [ ] Test diff generation
- [ ] Test report generation (HTML, JSON, Markdown)
- [ ] Test CLI commands
- [ ] Test API endpoints
- [ ] Validate HTML report rendering
- [ ] Test with agents producing different outputs

## Acceptance Criteria

1. Metrics are collected accurately for all agents
2. Diffs are generated correctly between agent outputs
3. HTML reports are interactive and visually appealing
4. JSON reports contain complete data for analysis
5. CLI commands provide useful output
6. API endpoints return correct comparison data
7. Reports identify best performing agent
8. Historical metrics can be tracked over time
9. Performance: Report generation < 5s for 10 agents
10. Reports are stored in `.cronus/comparison/reports/`

## Related Files

- `server/cronus/metricsCollector.h`
- `server/cronus/metricsCollector.cpp`
- `server/cronus/agentComparator.h`
- `server/cronus/agentComparator.cpp`
- `server/cronus/reportGenerator.h`
- `server/cronus/reportGenerator.cpp`
- `templates/comparison-report.html`
- `schemas/metrics.json`

## Notes

- Use Chart.js or similar for interactive charts
- Consider using diff2html for nice diff rendering
- Store reports with timestamps for historical tracking
- Provide option to export metrics to external tools (Grafana, etc.)
- Add ability to define custom metrics via configuration
- Consider integration with the evaluation system (evaluation/)
