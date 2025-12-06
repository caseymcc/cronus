# Project: 
cronus

## Description: 
This application is a modular AI assistant platform called Cronus, designed to support multiple client interfaces (web, VS Code, CLI) and integrate with various language models through a unified arbiterAI layer. 

## Primary Goals: 
- Generate a c/c++ application that works as a coding ai agent with an web client. Has the ability to run multiple agents in parallel, maintaining state and context between them.

## Secondary Goal: 
- A cli client to the agent server
- A VSCode extension to the agent server (re-using as much as possible from the web client)

## Target Audience: 
Anyone interested in develop applicatoin with ai assistance

##  Core Features:
- AI coding agent server
- Mutiple clients (web, vscode, cli) for the server
- Multiple agents (code writer, code formatter, code debugger, code tester, code optimizer), each with their own prompts and models
- (optional) Built in developement process that maintains 
    - State of code, task both todo, in progress, and done. 
    - Along with design decision, architecture, library dependencies, goals.
- Configurations for:
  - Providers (LLM, Embedding, Storage)
  - Custom models, with custom prompts
  - Custom tools
  - Custom code formatting prompts based on language type of file generated or modified 
- AI arbiter layer to manage multiple LLMs (using server/arbiterAI/)
- AI code understanding and search (using server/loreforge/)
- 

## Additional Documentation:
- docs/developer.md - Contains more detailed information on the project including structure, interface, configuration, and build instructions.
- docs/research-decisions.md - Research to provide insight on decisions for the agent/.