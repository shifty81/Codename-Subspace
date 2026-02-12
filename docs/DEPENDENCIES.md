# AvorionLike Dependencies

This document lists all dependencies required to build and run AvorionLike, along with their purposes and installation instructions.

## Runtime Dependencies

### Required Dependencies

#### 1. .NET 9.0 SDK (or later)
- **Purpose**: Core runtime environment and build tools for C# applications
- **Version**: 9.0 or higher
- **License**: MIT
- **Installation**:
  - **Windows**: Download from [dotnet.microsoft.com/download](https://dotnet.microsoft.com/download)
  - **Linux**: See [Install .NET on Linux](https://learn.microsoft.com/en-us/dotnet/core/install/linux)
  - **macOS**: 
    - Via Homebrew: `brew install --cask dotnet-sdk`
    - Or download from [dotnet.microsoft.com/download](https://dotnet.microsoft.com/download)
- **Verify Installation**: 
  ```bash
  dotnet --version
  ```
  Should output 9.0.x or higher

#### 2. NLua
- **Purpose**: Lua scripting integration for modding support
- **Version**: 1.7.3
- **NuGet Package**: [NLua](https://www.nuget.org/packages/NLua/)
- **License**: MIT
- **Installation**: Automatically restored via `dotnet restore`
- **Why needed**: Enables Lua scripting capabilities for custom mods and scripts
- **Documentation**: [NLua GitHub](https://github.com/NLua/NLua)

## Built-in .NET Dependencies

These are part of .NET 9.0 and require no additional installation:

- **System.Numerics**: Vector math for physics and 3D calculations
- **System.Net.Sockets**: TCP networking for multiplayer functionality
- **System.Collections**: Core collection types and data structures
- **System.Linq**: Language Integrated Query for data manipulation
- **System.Threading**: Multi-threading support for concurrent operations

## Development Dependencies (Optional)

### Recommended Tools

#### Git
- **Purpose**: Version control and cloning the repository
- **Installation**:
  - **Windows**: [git-scm.com](https://git-scm.com/)
  - **Linux**: `sudo apt install git` (Ubuntu/Debian) or equivalent
  - **macOS**: `brew install git`

#### Visual Studio Code
- **Purpose**: Code editing with IntelliSense and debugging
- **Installation**: [code.visualstudio.com](https://code.visualstudio.com/)
- **Recommended Extensions**:
  - C# Dev Kit
  - .NET Extension Pack

#### Visual Studio 2022 (Windows)
- **Purpose**: Full-featured IDE with advanced debugging
- **Installation**: [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
- **Edition**: Community (free) or higher
- **Workload**: .NET desktop development

#### JetBrains Rider
- **Purpose**: Cross-platform .NET IDE
- **Installation**: [jetbrains.com/rider](https://www.jetbrains.com/rider/)
- **License**: Commercial (30-day free trial)

## Automated Dependency Installation

AvorionLike provides automated setup scripts that handle dependency installation:

### Windows (PowerShell)
```powershell
.\setup.ps1
```

### Linux/macOS (Bash)
```bash
./setup.sh
```

These scripts will:
1. ✅ Verify .NET SDK is installed (9.0+)
2. ✅ Restore NuGet packages (NLua)
3. ✅ Build the project
4. ✅ Confirm everything is ready to run

## Manual Dependency Installation

If you prefer manual installation:

### Step 1: Install .NET 9.0 SDK
Follow the instructions for your platform above.

### Step 2: Restore NuGet Packages
```bash
cd AvorionLike
dotnet restore
```

This downloads NLua 1.7.3 and any other NuGet dependencies.

### Step 3: Verify Installation
```bash
dotnet build
```

If successful, all dependencies are correctly installed.

## Dependency Security

All dependencies are from trusted sources:
- **.NET SDK**: Official Microsoft product
- **NLua**: Open-source project with active maintenance and community support

We regularly update dependencies to include security patches and bug fixes.

## Dependency Licenses

- **.NET SDK**: MIT License
- **NLua**: MIT License

This project itself is licensed under the MIT License, compatible with all dependencies.

## Checking Installed Dependencies

### List All NuGet Packages
```bash
cd AvorionLike
dotnet list package
```

### Check .NET SDK Version
```bash
dotnet --version
```

### List All Installed .NET SDKs
```bash
dotnet --list-sdks
```

## Troubleshooting Dependencies

### NuGet Restore Fails
```bash
# Clear NuGet cache
dotnet nuget locals all --clear

# Try restore again
cd AvorionLike
dotnet restore
```

### Wrong .NET Version
- Uninstall old version (optional, multiple versions can coexist)
- Install .NET 9.0 SDK from official sources
- Restart terminal
- Verify: `dotnet --version`

### Missing NLua After Restore
```bash
# Delete bin and obj folders
rm -rf AvorionLike/bin AvorionLike/obj

# Restore and rebuild
cd AvorionLike
dotnet restore
dotnet build
```

## Future Dependencies

As the project evolves, additional dependencies may be added for:
- Graphics rendering (e.g., OpenGL, DirectX bindings)
- Audio support
- Advanced UI frameworks
- Additional scripting language support

All new dependencies will be documented here and handled by the automated setup scripts.

## Questions?

For dependency-related questions:
1. Check the [Quick Start Guide](QUICKSTART.md)
2. Review the [Troubleshooting](README.md#troubleshooting) section in the README
3. Open an issue on GitHub with the "dependencies" label
