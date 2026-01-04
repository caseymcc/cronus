# Task 006: Multi-Agent Infrastructure

**Status**: Todo  
**Priority**: High  
**Estimated Effort**: 5-7 days  
**Dependencies**: Task 001, 002, 003, 004

## Objective

Implement the core infrastructure for multi-agent mode, including agent workspace isolation, repository management, parallel execution, and agent lifecycle management.

## Requirements

### Agent Workspace Isolation

Each agent operates in its own isolated workspace:
```
agents/
├── agent-alpha/
│   ├── .cronus/         # Agent's configuration
│   ├── .git/            # Agent's repository
│   └── src/             # Agent's source code
└── agent-beta/
    ├── .cronus/
    └── ...
```

### Features

- **Repository Cloning**: Clone source repository for each agent
- **Workspace Isolation**: Each agent has independent filesystem view
- **Parallel Execution**: Multiple agents run simultaneously
- **Agent Lifecycle**: Create, start, stop, restart, remove agents
- **Resource Management**: CPU/memory limits per agent
- **Status Tracking**: Monitor each agent's health and activity

## Implementation Tasks

- [ ] Create `MultiAgentManager` class
  - [ ] Agent registry and tracking
  - [ ] Agent creation and initialization
  - [ ] Agent lifecycle management
  - [ ] Parallel agent execution
  - [ ] Agent status monitoring

- [ ] Implement agent workspace creation
  - [ ] Create agent directory structure
  - [ ] Clone source repository to agent workspace
  - [ ] Initialize agent's `.cronus/` directory
  - [ ] Set up agent-specific configuration
  - [ ] Create isolation mechanisms (chroot/containers optional)

- [ ] Create `AgentExecutor` class
  - [ ] Execute agent in isolated workspace
  - [ ] Capture agent output/errors
  - [ ] Resource monitoring (CPU, memory, disk)
  - [ ] Timeout handling
  - [ ] Agent crash recovery

- [ ] Implement repository management
  - [ ] Git clone with progress tracking
  - [ ] Branch management per agent
  - [ ] Repository health checks
  - [ ] Disk space monitoring

- [ ] CLI commands
  - [ ] `cronus agent create <id> --source=<repo> --model=<model>` - Create agent
  - [ ] `cronus agent list` - List all agents
  - [ ] `cronus agent status <id>` - Show agent status
  - [ ] `cronus agent start <id>` - Start agent
  - [ ] `cronus agent stop <id>` - Stop agent
  - [ ] `cronus agent restart <id>` - Restart agent
  - [ ] `cronus agent remove <id>` - Remove agent
  - [ ] `cronus agent logs <id>` - View agent logs

- [ ] API endpoints
  - [ ] `POST /api/agents` - Create new agent
  - [ ] `GET /api/agents` - List all agents
  - [ ] `GET /api/agents/{id}` - Get agent details
  - [ ] `GET /api/agents/{id}/status` - Get agent status
  - [ ] `POST /api/agents/{id}/start` - Start agent
  - [ ] `POST /api/agents/{id}/stop` - Stop agent
  - [ ] `DELETE /api/agents/{id}` - Remove agent
  - [ ] `GET /api/agents/{id}/logs` - Stream agent logs

## Agent Creation Workflow

```cpp
void createAgent(const std::string& agentId, 
                 const std::filesystem::path& sourceRepo,
                 const AgentConfig& config) {
    // 1. Create agent workspace directory
    auto agentDir = workspaceRoot / "agents" / agentId;
    fs::create_directories(agentDir);
    
    // 2. Clone source repository
    if (!sourceRepo.empty()) {
        git::clone(sourceRepo, agentDir);
    } else {
        git::init(agentDir);
    }
    
    // 3. Create .cronus configuration
    auto cronusDir = agentDir / ".cronus";
    fs::create_directories(cronusDir);
    initializeAgentConfig(cronusDir, config);
    
    // 4. Register agent
    multiAgentManager.registerAgent(agentId, agentDir);
    
    // 5. Update global state
    updateGlobalState();
}
```

## Testing

- [ ] Unit tests for MultiAgentManager
- [ ] Test agent creation and initialization
- [ ] Test parallel agent execution
- [ ] Test agent lifecycle (start/stop/restart)
- [ ] Test agent isolation
- [ ] Test resource limits
- [ ] Test agent crash recovery
- [ ] Test CLI commands
- [ ] Test API endpoints
- [ ] Stress test with many agents (10+)

## Acceptance Criteria

1. Agents can be created with source repository cloning
2. Each agent has isolated workspace with own `.cronus/` config
3. Multiple agents can run in parallel without interference
4. Agent lifecycle operations (start/stop/restart/remove) work correctly
5. Agent status is tracked and reported accurately
6. Resource limits can be set and enforced per agent
7. Agent crashes are detected and can be recovered
8. CLI commands provide clear output and error messages
9. API endpoints are secured and functional
10. Performance: Agent creation < 30s, start/stop < 5s

## Related Files

- `server/cronus/multiAgentManager.h`
- `server/cronus/multiAgentManager.cpp`
- `server/cronus/agentExecutor.h`
- `server/cronus/agentExecutor.cpp`
- `server/cronus/repositoryManager.h`
- `server/cronus/repositoryManager.cpp`

## Notes

- Consider using Docker containers for stronger agent isolation (optional)
- Monitor disk space as each agent needs a full repository copy
- Implement cleanup of orphaned agent directories
- Provide agent template system for common configurations
- Add agent naming validation (alphanumeric + hyphens only)
