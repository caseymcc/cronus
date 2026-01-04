#!/bin/bash

# Cronus Debug Client Build Script
# This script builds all necessary components for the debug client

set -e  # Exit on error

echo "================================================"
echo "Building Cronus Debug Client"
echo "================================================"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Navigate to clients directory
cd "$(dirname "$0")"

echo -e "\n${BLUE}Step 1: Installing dependencies...${NC}"
# Install shared library dependencies
cd shared
npm install
cd ..

# Build shared first since others depend on it
echo -e "\n${BLUE}Step 2: Building @cronus/shared...${NC}"
cd shared
npm run build
cd ..

# Install shared-ui dependencies (now that shared is built)
echo -e "\n${BLUE}Step 3: Installing shared-ui dependencies...${NC}"
cd shared-ui
npm install --legacy-peer-deps
cd ..

# Build shared-ui
echo -e "\n${BLUE}Step 4: Building @cronus/shared-ui...${NC}"
cd shared-ui
npm run build
cd ..

# Install debug client dependencies (now that shared-ui is built)
echo -e "\n${BLUE}Step 5: Installing debug client dependencies...${NC}"
cd debug
npm install
cd ..

# Build debug client
echo -e "\n${BLUE}Step 6: Building debug client...${NC}"
cd debug
npm run build
cd ..

echo -e "\n${GREEN}================================================${NC}"
echo -e "${GREEN}Build Complete!${NC}"
echo -e "${GREEN}================================================${NC}"

# Fix permissions if running in Docker (HOST_UID and HOST_GID will be set)
if [ -n "$HOST_UID" ] && [ -n "$HOST_GID" ]; then
    echo -e "\n${BLUE}Fixing file permissions for host user...${NC}"
    chown -R $HOST_UID:$HOST_GID shared/dist shared-ui/dist debug/dist 2>/dev/null || true
    chown -R $HOST_UID:$HOST_GID shared/node_modules shared-ui/node_modules debug/node_modules 2>/dev/null || true
fi

echo -e "\n${BLUE}Next steps:${NC}"
echo "1. Start Cronus server:"
echo "   cd /home/caseymcc/projects/cronus"
echo "   ./run_local.sh ./build/linux_x64_debug/server/cronus/cronus"
echo ""
echo "2. Start debug client:"
echo "   cd /home/caseymcc/projects/cronus/clients/debug"
echo "   npm start"
echo ""
echo "2. Open VSCode extension:"
echo "   - Open the clients/vscode folder in VSCode"
echo "   - Press F5 to debug the extension"
echo "   - Run 'Cronus: Open Debug Panel' from command palette"
echo ""
echo "3. Or run web client:"
echo "   npm run dev:web"
