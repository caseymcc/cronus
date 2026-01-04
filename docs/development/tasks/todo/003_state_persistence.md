# Task 003: State Persistence Layer

**Status**: Todo  
**Priority**: High  
**Estimated Effort**: 3-4 days  
**Dependencies**: Task 001 (Mode Detection), Task 002 (Configuration System)

## Objective

Implement a robust state persistence layer that maintains agent state, session information, task tracking, and conversation history across server restarts for both single-agent and multi-agent modes.

## Requirements

### State Files

1. **Agent State** (`state.json`)
   - Agent status (active/idle/error)
   - Current session information
   - Statistics (messages, files modified, tasks completed)
   - Repository state (branch, last commit, uncommitted changes)
   - Loreforge index status

2. **Task State** (`tasks/`)
   - `todo.json` - Pending tasks
   - `in_progress.json` - Active tasks
   - `done.json` - Completed tasks

3. **Context/History** (`context/`)
   - `history.jsonl` - Conversation history (JSON Lines format)
   - `summaries/` - Session summaries for context window management

### Features

- **Atomic Writes**: Prevent corruption during write failures
- **Automatic Backups**: Keep last N versions of state files
- **Recovery**: Detect and recover from corrupted state files
- **Compression**: Optional compression for large history files
- **Periodic Saves**: Auto-save state every N minutes or M messages

## Implementation Tasks

- [ ] Create `StateManager` class
  - [ ] Load state from disk
  - [ ] Save state to disk (atomic)
  - [ ] Validate state schema
  - [ ] State recovery from backups
  - [ ] State migration for version updates

- [ ] Implement atomic file operations
  - [ ] Write to temporary file
  - [ ] Validate written data
  - [ ] Atomic rename to target file
  - [ ] Cleanup temporary files

- [ ] Create `TaskManager` class
  - [ ] Load task lists
  - [ ] Create/update/delete tasks
  - [ ] Move tasks between states (todo → in-progress → done)
  - [ ] Task search and filtering
  - [ ] Task persistence

- [ ] Create `ConversationHistory` class
  - [ ] Append messages to history (JSONL format)
  - [ ] Load recent messages
  - [ ] Summarize old conversations
  - [ ] Prune old history (archive)
  - [ ] Search conversation history

- [ ] Multi-agent state management
  - [ ] Global state file (all agents)
  - [ ] Per-agent state files
  - [ ] State synchronization
  - [ ] Agent status aggregation

- [ ] Implement backup system
  - [ ] Keep last N state backups
  - [ ] Timestamped backup files
  - [ ] Automatic cleanup of old backups
  - [ ] Restore from backup command

- [ ] Recovery mechanisms
  - [ ] Detect corrupted state files
  - [ ] Attempt recovery from backups
  - [ ] Fallback to default state
  - [ ] Log recovery attempts

## State File Schemas

### Single-Agent `state.json`
```json
{
  "version": "1.0",
  "mode": "single-agent",
  "last_updated": "2026-01-03T12:30:00Z",
  "agent": {
    "id": "primary",
    "status": "active",
    "last_active": "2026-01-03T12:30:00Z"
  },
  "session": {
    "id": "session-42",
    "messages": 42,
    "files_modified": 15,
    "tasks_completed": 7
  }
}
```

### Multi-Agent Global `state.json`
```json
{
  "version": "1.0",
  "mode": "multi-agent",
  "last_updated": "2026-01-03T12:30:00Z",
  "agents": [
    {
      "id": "agent-alpha",
      "status": "active",
      "workspace_path": "agents/agent-alpha",
      "stats": {
        "messages": 28,
        "files_modified": 8
      }
    }
  ]
}
```

### Task File Format
```json
{
  "tasks": [
    {
      "id": "task-001",
      "title": "Implement authentication",
      "description": "Add JWT-based authentication",
      "created_at": "2026-01-03T10:00:00Z",
      "priority": "high",
      "assigned_to": "agent-alpha"
    }
  ]
}
```

## Testing

- [ ] Unit tests for StateManager
- [ ] Test atomic writes and rollback
- [ ] Test state recovery from backups
- [ ] Test concurrent state access
- [ ] Test TaskManager operations
- [ ] Test ConversationHistory (append, load, search)
- [ ] Test multi-agent state synchronization
- [ ] Test state migration between versions
- [ ] Stress test with large state files

## Acceptance Criteria

1. State persists correctly across server restarts
2. No data loss during crashes or power failures (atomic writes)
3. Corrupted state files are detected and recovered automatically
4. Task operations (create/update/move/delete) work correctly
5. Conversation history is searchable and manageable
6. Multi-agent state synchronization is reliable
7. Performance: State save < 100ms, load < 200ms
8. Backup system maintains last 10 versions by default

## Related Files

- `server/cronus/stateManager.h`
- `server/cronus/stateManager.cpp`
- `server/cronus/taskManager.h`
- `server/cronus/taskManager.cpp`
- `server/cronus/conversationHistory.h`
- `server/cronus/conversationHistory.cpp`
- `schemas/state-single.json`
- `schemas/state-multi.json`
- `schemas/task.json`

## Notes

- Use JSON Lines (.jsonl) format for conversation history (append-friendly)
- Consider using SQLite for large conversation histories (future enhancement)
- Implement file locking to prevent concurrent modifications
- Provide CLI tools for state inspection and repair
