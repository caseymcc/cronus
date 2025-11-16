#!/bin/bash
# filepath: /home/caseymcc/projects/cronus/build.sh

# Install dependencies
npm install

# Build the React app
npm run build

# Create the directory in the build folder if it doesn't exist
mkdir -p ../build/web

# Copy the build output to the target directory
cp -r build/* ../build/web/

echo "Frontend built and copied to build/web/"