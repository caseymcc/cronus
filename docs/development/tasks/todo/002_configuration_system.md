# Task 002: Configuration Management System

**Status**: Todo  
**Priority**: Critical  
**Estimated Effort**: 3-4 days  
**Dependencies**: Task 001 (Mode Detection)

## Objective

Implement a comprehensive configuration management system that supports both single-agent and multi-agent modes, with JSON schemas, validation, environment variable substitution, and hot-reloading capabilities.

## Requirements

### Configuration File Schemas

1. **Global Configuration** (`.cronus/config.json`)
   - Server settings (host, port, API path)
   - Mode specification
   - Workspace settings (multi-agent only)
   - Shared resources configuration
   - Logging configuration

2. **Agent Configuration** (per-agent)
   - Single-agent: `.cronus/config.json` (combined with global)
   - Multi-agent: `agents/<id>/.cronus/config.json`
   - Model settings (provider, name, parameters)
   - Prompts and system instructions
   - Tool permissions
   - Repository settings

### Features

- **Environment Variable Substitution**: `${OPENAI_API_KEY}` → actual value
- **Configuration Validation**: JSON schema validation
- **Hot Reloading**: Detect and reload configuration changes
- **Inheritance**: Multi-agent mode - agents inherit from global config
- **Merge Strategy**: Override rules for inherited settings

## Implementation Tasks

- [ ] Create configuration schemas
  - [ ] Define JSON schemas for validation
  - [ ] Single-agent schema
  - [ ] Multi-agent global schema
  - [ ] Per-agent schema

- [ ] Create `ConfigManager` class
  - [ ] Load configuration from file
  - [ ] Parse JSON with nlohmann/json
  - [ ] Environment variable substitution
  - [ ] Schema validation
  - [ ] Configuration caching

- [ ] Implement inheritance system (multi-agent)
  - [ ] Load global configuration
  - [ ] Load per-agent configuration
  - [ ] Merge configurations with override rules
  - [ ] Validate merged configuration

- [ ] Hot reload support
  - [ ] File watching for configuration changes
  - [ ] Validate before applying changes
  - [ ] Notify components of configuration updates
  - [ ] Rollback on invalid configuration

- [ ] CLI commands
  - [ ] `cronus config show` - Display current configuration
  - [ ] `cronus config edit` - Open editor for configuration
  - [ ] `cronus config set <key> <value>` - Set configuration value
  - [ ] `cronus config validate` - Validate configuration file
  - [ ] `cronus config reset` - Reset to default configuration

- [ ] API endpoints
  - [ ] `GET /api/config` - Get current configuration
  - [ ] `PUT /api/config` - Update configuration
  - [ ] `POST /api/config/validate` - Validate configuration

## Configuration Examples

### Single-Agent `config.json`
```json
{
  "version": "1.0",
  "mode": "single-agent",
  "server": {
    "host": "localhost",
    "port": 8080
  },
  "agent": {
    "model": {
      "provider": "openai",
      "name": "gpt-4",
      "api_key": "${OPENAI_API_KEY}"
    }
  }
}
```

### Multi-Agent Global `config.json`
```json
{
  "version": "1.0",
  "mode": "multi-agent",
  "server": {
    "host": "localhost",
    "port": 8080
  },
  "agents": [
    {
      "id": "agent-alpha",
      "workspace_path": "agents/agent-alpha",
      "model": {
        "provider": "openai",
        "name": "gpt-4"
      }
    }
  ]
}
```

## Testing

- [ ] Unit tests for ConfigManager
- [ ] Test environment variable substitution
- [ ] Test schema validation (valid and invalid configs)
- [ ] Test configuration inheritance
- [ ] Test hot reload functionality
- [ ] Test CLI commands
- [ ] Test API endpoints

## Acceptance Criteria

1. Configuration files are loaded and validated correctly
2. Environment variables are substituted properly
3. Invalid configurations are rejected with clear error messages
4. Hot reload works without server restart
5. Multi-agent configuration inherits from global correctly
6. CLI commands work for all configuration operations
7. API endpoints secured and functional
8. Performance: Config load < 50ms, reload < 100ms

## Related Files

- `server/cronus/configManager.h`
- `server/cronus/configManager.cpp`
- `server/cronus/config.h` (update)
- `server/cronus/config.cpp` (update)
- `schemas/config-single.json` (JSON schema)
- `schemas/config-multi.json` (JSON schema)
- `schemas/config-agent.json` (JSON schema)

## Notes

- Use nlohmann/json for JSON parsing
- Consider using JSON Schema validator library
- Ensure thread-safety for hot reload
- Provide migration tool for old configuration formats
