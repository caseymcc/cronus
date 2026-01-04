# Task 001: Mode Detection and Initialization

**Status**: Todo  
**Priority**: Critical  
**Estimated Effort**: 2-3 days  
**Dependencies**: None

## Objective

Implement automatic detection of operational mode (single-agent vs multi-agent) and initialize the appropriate directory structure and configuration files.

## Requirements

### Mode Detection Algorithm

1. **Check for existing `.cronus/` directory**
   - If exists, load `config.json` to determine mode
   - Check for `multi-agent.marker` file

2. **Check for version control markers**
   - `.git/`, `.svn/`, `.hg/` indicate existing repository → Single-Agent Mode

3. **Check directory contents**
   - Empty or nearly empty directory → Prompt for mode or default to Multi-Agent
   - Directory with files but no VCS → Single-Agent Mode

4. **Allow explicit mode specification**
   - Command-line flag: `--mode=single|multi`
   - Environment variable: `CRONUS_MODE`

### Directory Initialization

#### Single-Agent Mode
```
.cronus/
├── config.json
├── state.json
├── lock
├── tasks/
├── context/
├── cache/
└── logs/
```

#### Multi-Agent Mode
```
.cronus/
├── multi-agent.marker
├── config.json
├── shared/
│   ├── cache/
│   └── templates/
├── comparison/
└── logs/

agents/  # Created when agents are added
```

## Implementation Tasks

- [ ] Create `ModeDetector` class
  - [ ] Implement directory inspection logic
  - [ ] Version control detection
  - [ ] Configuration file parsing
  - [ ] Mode validation

- [ ] Create `DirectoryInitializer` class
  - [ ] Single-agent directory structure creation
  - [ ] Multi-agent directory structure creation
  - [ ] Template configuration file generation
  - [ ] Permission setting (chmod 700 for `.cronus/`)

- [ ] Implement CLI commands
  - [ ] `cronus init` - Initialize in detected mode
  - [ ] `cronus init --single-agent` - Force single-agent mode
  - [ ] `cronus init --multi-agent` - Force multi-agent mode
  - [ ] `cronus detect-mode` - Report detected mode

- [ ] Add validation
  - [ ] Prevent mode conflicts
  - [ ] Validate directory permissions
  - [ ] Check for existing Cronus instances

## Testing

- [ ] Unit tests for mode detection logic
- [ ] Integration tests for directory initialization
- [ ] Test mode detection in various scenarios:
  - [ ] Empty directory
  - [ ] Git repository
  - [ ] Existing `.cronus/` directory
  - [ ] Directory with files but no VCS
- [ ] Test initialization failure recovery

## Acceptance Criteria

1. Cronus correctly detects mode in 100% of test cases
2. Directory structure is created correctly for both modes
3. Configuration files are valid JSON with correct schemas
4. Mode can be explicitly overridden via CLI or environment
5. Clear error messages when mode detection fails
6. Documentation updated with mode detection details

## Related Files

- `server/cronus/modeDetector.h`
- `server/cronus/modeDetector.cpp`
- `server/cronus/directoryInitializer.h`
- `server/cronus/directoryInitializer.cpp`
- `server/cronus/config.h` (update for mode-specific config)

## Notes

- Mode detection should be fast (< 100ms)
- Should not modify filesystem during detection, only during initialization
- Must handle race conditions (multiple Cronus instances starting simultaneously)
