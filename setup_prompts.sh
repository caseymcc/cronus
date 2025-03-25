#!/bin/bash

# Create prompt directories
mkdir -p resources/prompts
mkdir -p ~/.cronus/prompts
mkdir -p .cronus/prompts

# Copy default prompts to resource directory
cp resources/prompts/coder_prompts.json ~/.cronus/prompts/
cp resources/prompts/language_prompts.json ~/.cronus/prompts/
cp resources/prompts/task_prompts.json ~/.cronus/prompts/

echo "Prompt directories and files have been set up successfully."
