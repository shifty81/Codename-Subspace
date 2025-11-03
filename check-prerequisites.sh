#!/bin/bash
# AvorionLike - Prerequisites Verification Script
# This script checks if all prerequisites are met without installing anything

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  AvorionLike Prerequisites Check${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""

ALL_OK=true

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check for .NET SDK
echo -e "${YELLOW}Checking for .NET SDK...${NC}"
if ! command_exists dotnet; then
    echo -e "${RED}❌ .NET SDK is not installed${NC}"
    ALL_OK=false
else
    DOTNET_VERSION=$(dotnet --version)
    # Extract major version, handling preview versions like "9.0.0-preview.1"
    MAJOR_VERSION=$(echo $DOTNET_VERSION | cut -d. -f1 | grep -o '^[0-9]*' | head -1)
    
    if [ -n "$MAJOR_VERSION" ] && [ "$MAJOR_VERSION" -ge 9 ]; then
        echo -e "${GREEN}✓ .NET SDK $DOTNET_VERSION (compatible)${NC}"
    else
        echo -e "${YELLOW}⚠️  .NET SDK $DOTNET_VERSION (requires 9.0+)${NC}"
        ALL_OK=false
    fi
fi

# Check for git (optional but recommended)
echo -e "${YELLOW}Checking for Git...${NC}"
if ! command_exists git; then
    echo -e "${YELLOW}⚠️  Git is not installed (optional, but recommended)${NC}"
else
    GIT_VERSION=$(git --version | awk '{print $3}')
    echo -e "${GREEN}✓ Git $GIT_VERSION${NC}"
fi

# Check project files
echo ""
echo -e "${YELLOW}Checking project files...${NC}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/AvorionLike"

if [ ! -d "$PROJECT_DIR" ]; then
    echo -e "${RED}❌ Project directory not found: $PROJECT_DIR${NC}"
    ALL_OK=false
else
    echo -e "${GREEN}✓ Project directory found${NC}"
    
    if [ ! -f "$PROJECT_DIR/AvorionLike.csproj" ]; then
        echo -e "${RED}❌ Project file not found: AvorionLike.csproj${NC}"
        ALL_OK=false
    else
        echo -e "${GREEN}✓ Project file found${NC}"
    fi
fi

echo ""
echo -e "${CYAN}========================================${NC}"

if [ "$ALL_OK" = true ]; then
    echo -e "${GREEN}✓ All prerequisites are met!${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
    echo -e "${YELLOW}Ready to build the project. Run:${NC}"
    echo -e "${CYAN}  ./setup.sh${NC}"
    echo ""
    echo -e "${YELLOW}Or manually:${NC}"
    echo -e "${CYAN}  cd AvorionLike${NC}"
    echo -e "${CYAN}  dotnet restore${NC}"
    echo -e "${CYAN}  dotnet build${NC}"
    echo -e "${CYAN}  dotnet run${NC}"
else
    echo -e "${RED}❌ Some prerequisites are missing${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
    echo -e "${YELLOW}Please install missing prerequisites and run this script again.${NC}"
    echo ""
    echo -e "${YELLOW}To install .NET SDK, visit:${NC}"
    echo -e "${CYAN}  https://dotnet.microsoft.com/download${NC}"
    echo ""
    echo -e "${YELLOW}Or run the automated setup:${NC}"
    echo -e "${CYAN}  ./setup.sh${NC}"
    exit 1
fi

echo ""
