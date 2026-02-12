#!/bin/bash
# Codename: Subspace - Automated Setup Script for Linux/macOS
# This script checks for prerequisites and sets up both the C# prototype
# and the new C++ engine.

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  AvorionLike Setup Script${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Detect OS
OS="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
fi

echo -e "${YELLOW}Detected OS: $OS${NC}"
echo ""

# Check for .NET SDK
echo -e "${YELLOW}Checking for .NET SDK...${NC}"
if ! command_exists dotnet; then
    echo -e "${RED}❌ .NET SDK is not installed!${NC}"
    echo ""
    echo -e "${YELLOW}Please install .NET 9.0 SDK or later:${NC}"
    echo ""
    
    if [[ "$OS" == "linux" ]]; then
        echo -e "${CYAN}For Ubuntu/Debian:${NC}"
        echo "  wget https://dot.net/v1/dotnet-install.sh -O dotnet-install.sh"
        echo "  chmod +x dotnet-install.sh"
        echo "  ./dotnet-install.sh --channel 9.0"
        echo "  export PATH=\"\$PATH:\$HOME/.dotnet\""
        echo ""
        echo -e "${CYAN}Or visit: https://learn.microsoft.com/en-us/dotnet/core/install/linux${NC}"
    elif [[ "$OS" == "macos" ]]; then
        echo -e "${CYAN}For macOS (using Homebrew):${NC}"
        echo "  brew install --cask dotnet-sdk"
        echo ""
        echo -e "${CYAN}Or download from: https://dotnet.microsoft.com/download${NC}"
    else
        echo -e "${CYAN}Visit: https://dotnet.microsoft.com/download${NC}"
    fi
    echo ""
    exit 1
fi

DOTNET_VERSION=$(dotnet --version)
echo -e "${GREEN}✓ .NET SDK is installed (version $DOTNET_VERSION)${NC}"

# Check if version is sufficient (9.0 or higher)
# Extract major version, handling preview versions like "9.0.0-preview.1"
MAJOR_VERSION=$(echo $DOTNET_VERSION | cut -d. -f1 | grep -o '^[0-9]*' | head -1)
if [ -z "$MAJOR_VERSION" ] || [ "$MAJOR_VERSION" -lt 9 ]; then
    echo -e "${YELLOW}⚠️  Warning: .NET SDK version $DOTNET_VERSION detected.${NC}"
    echo -e "${YELLOW}   This project requires .NET 9.0 or later.${NC}"
    echo -e "${YELLOW}   Please update your .NET SDK from: https://dotnet.microsoft.com/download${NC}"
    echo ""
    read -p "Do you want to continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo ""
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  Installing Dependencies${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""

# Navigate to project directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/AvorionLike"

if [ ! -d "$PROJECT_DIR" ]; then
    echo -e "${RED}❌ Project directory not found: $PROJECT_DIR${NC}"
    exit 1
fi

cd "$PROJECT_DIR"

# Restore NuGet packages
echo -e "${YELLOW}Restoring NuGet packages...${NC}"
if ! dotnet restore; then
    echo -e "${RED}❌ Failed to restore NuGet packages!${NC}"
    exit 1
fi
echo -e "${GREEN}✓ NuGet packages restored successfully${NC}"
echo ""

# Build the project
echo -e "${YELLOW}Building the project...${NC}"
if ! dotnet build; then
    echo -e "${RED}❌ Build failed!${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Project built successfully${NC}"
echo ""

echo -e "${CYAN}========================================${NC}"
echo -e "${GREEN}  C# Prototype Setup Complete!${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""
echo -e "${YELLOW}You can now run the C# prototype using:${NC}"
echo -e "${CYAN}  cd AvorionLike${NC}"
echo -e "${CYAN}  dotnet run${NC}"
echo ""
echo -e "${YELLOW}Or build and run in release mode:${NC}"
echo -e "${CYAN}  cd AvorionLike${NC}"
echo -e "${CYAN}  dotnet run --configuration Release${NC}"
echo ""

# ================================================================
# C++ Engine Build
# ================================================================
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  C++ Engine (CMake)${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""

if command_exists cmake; then
    CMAKE_VERSION=$(cmake --version | head -1)
    echo -e "${GREEN}✓ CMake found: $CMAKE_VERSION${NC}"
    echo ""

    if command_exists g++ || command_exists clang++; then
        echo -e "${YELLOW}Building C++ engine...${NC}"
        cd "$SCRIPT_DIR/engine"
        cmake -B build -DCMAKE_BUILD_TYPE=Debug 2>&1
        if cmake --build build 2>&1; then
            echo -e "${GREEN}✓ C++ engine built successfully${NC}"
            echo ""

            echo -e "${YELLOW}Running C++ tests...${NC}"
            if [ -f "build/subspace_tests" ]; then
                ./build/subspace_tests
            fi
        else
            echo -e "${YELLOW}⚠️  C++ engine build had issues (this is optional)${NC}"
        fi
    else
        echo -e "${YELLOW}⚠️  No C++ compiler found. Install g++ or clang++:${NC}"
        if [[ "$OS" == "linux" ]]; then
            echo -e "${CYAN}  sudo apt install build-essential libgl-dev${NC}"
        elif [[ "$OS" == "macos" ]]; then
            echo -e "${CYAN}  xcode-select --install${NC}"
        fi
    fi
else
    echo -e "${YELLOW}⚠️  CMake not found. Install it to build the C++ engine:${NC}"
    if [[ "$OS" == "linux" ]]; then
        echo -e "${CYAN}  sudo apt install cmake build-essential libgl-dev${NC}"
    elif [[ "$OS" == "macos" ]]; then
        echo -e "${CYAN}  brew install cmake${NC}"
    fi
fi

echo ""
echo -e "${CYAN}========================================${NC}"
echo -e "${GREEN}  All Done!${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""
echo -e "${YELLOW}Projects in this repository:${NC}"
echo -e "${CYAN}  C# Prototype (AvorionLike)  — existing gameplay prototype${NC}"
echo -e "${CYAN}  C++ Engine   (engine/)       — new block-based ship engine${NC}"
echo ""
echo -e "${YELLOW}On Windows, open AvorionLike.sln in Visual Studio to build everything.${NC}"
echo -e "${YELLOW}On Linux/macOS, use dotnet + cmake as shown above.${NC}"
echo ""
