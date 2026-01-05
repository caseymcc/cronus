#!/bin/bash

# Cronus Standalone App Build Script
# This script builds the web client and wraps it in an Electron app

set -e  # Exit on error

echo "================================================"
echo "Building Cronus Standalone App"
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

# Install web client dependencies
echo -e "\n${BLUE}Step 5: Installing web client dependencies...${NC}"
cd web
npm install
cd ..

# Build web client
echo -e "\n${BLUE}Step 6: Building web client...${NC}"
cd web
npm run build
cd ..

# Install app (Electron wrapper) dependencies
echo -e "\n${BLUE}Step 7: Installing app dependencies...${NC}"
cd app
npm install
cd ..

# Build and package the Electron app
echo -e "\n${BLUE}Step 8: Building Electron app...${NC}"
cd app
npm run build
cd ..

echo -e "\n${GREEN}================================================${NC}"
echo -e "${GREEN}Build Complete!${NC}"
echo -e "${GREEN}================================================${NC}"

# Fix permissions if running in Docker (HOST_UID and HOST_GID will be set)
if [ -n "$HOST_UID" ] && [ -n "$HOST_GID" ]; then
    echo -e "\n${BLUE}Fixing file permissions for host user...${NC}"
    chown -R $HOST_UID:$HOST_GID shared/dist shared-ui/dist web/build app/dist app/renderer 2>/dev/null || true
    chown -R $HOST_UID:$HOST_GID shared/node_modules shared-ui/node_modules web/node_modules app/node_modules 2>/dev/null || true
fi

echo -e "\n${BLUE}Next steps:${NC}"
echo "1. Start Cronus server:"
echo "   cd /home/caseymcc/projects/cronus"
echo "   ./run_local.sh ./build/linux_x64_debug/server/cronus/cronus"
echo ""
echo "2. Start standalone app:"
echo "   cd /home/caseymcc/projects/cronus/clients/app"
echo "   npm start"
echo ""
echo "3. Or run web client in browser:"
echo "   cd /home/caseymcc/projects/cronus/clients/web"
echo "   npm start"

