# Task: Implement Specialized Agents

## Description
Expand the agent capabilities beyond the basic "Coder" agent. Implement specialized agents for specific tasks.

## Requirements
- Implement the following agents in `server/cronus/agents`:
    - **Code Formatter**: Specialized in applying style guides.
    - **Code Debugger**: Specialized in analyzing errors and logs.
    - **Code Tester**: Generates unit and integration tests.
    - **Code Optimizer**: Suggests performance and readability improvements.
- Each agent should have:
    - Dedicated system prompts
    - Specific tool sets
    - Configuration options

## Dependencies
- Cronus Server Agent Infrastructure
