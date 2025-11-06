# How to Build and Run Codename:Subspace

This guide provides detailed instructions for building and running the Codename:Subspace game engine with the new integrated Player UI.

## Prerequisites

- **.NET 9.0 SDK or later** - [Download here](https://dotnet.microsoft.com/download)
- **OpenGL-compatible graphics card** (for 3D rendering)
- **Minimum 4GB RAM** recommended
- **Operating System**: Windows, Linux, or macOS

### Verify Prerequisites

```bash
# Check .NET version
dotnet --version
# Should show 9.0.0 or higher
```

## Quick Start

### Windows

```powershell
# 1. Clone or download the repository
git clone https://github.com/shifty81/AvorionLike.git
cd AvorionLike

# 2. Run setup script (installs dependencies)
.\setup.ps1

# 3. Navigate to project directory
cd AvorionLike

# 4. Run the game
dotnet run
```

### Linux/macOS

```bash
# 1. Clone or download the repository
git clone https://github.com/shifty81/AvorionLike.git
cd AvorionLike

# 2. Run setup script (installs dependencies)
chmod +x setup.sh
./setup.sh

# 3. Navigate to project directory
cd AvorionLike

# 4. Run the game
dotnet run
```

## Manual Build Process

If you prefer to build manually:

```bash
# 1. Navigate to the project directory
cd AvorionLike/AvorionLike

# 2. Restore NuGet packages
dotnet restore

# 3. Build the project
dotnet build

# 4. Run the executable
dotnet run

# Or build and run in one command
dotnet run --project AvorionLike.csproj
```

## Creating a Standalone Executable

To create a standalone executable that can be distributed:

### Windows (64-bit)

```bash
dotnet publish -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true
```

Output: `bin/Release/net9.0/win-x64/publish/AvorionLike.exe`

### Linux (64-bit)

```bash
dotnet publish -c Release -r linux-x64 --self-contained true -p:PublishSingleFile=true
```

Output: `bin/Release/net9.0/linux-x64/publish/AvorionLike`

### macOS (64-bit)

```bash
dotnet publish -c Release -r osx-x64 --self-contained true -p:PublishSingleFile=true
```

Output: `bin/Release/net9.0/osx-x64/publish/AvorionLike`

## Running the Game

After launching, you'll see a main menu with options:

### 🎮 Option 1: NEW GAME (Recommended)

Select **1 - NEW GAME - Start Full Gameplay Experience** to launch the integrated player experience:

**Features:**
- Fully functional player ship with all systems
- 3D graphics window with real-time rendering
- Player UI showing ship status, inventory, and mission info
- Control your ship in space with 6DOF movement
- Explore asteroids and mine resources
- All UI panels accessible (Inventory, Ship Builder, etc.)

**Controls:**

**Toggle Control Modes:**
- **C** - Switch between Camera Mode and Ship Control Mode

**Camera Mode (Default):**
- **WASD** - Move camera forward/back/left/right
- **Space** - Move camera up
- **Shift** - Move camera down
- **Mouse** - Look around

**Ship Control Mode (Press C to activate):**
- **W** - Forward thrust
- **S** - Backward thrust
- **A** - Left thrust
- **D** - Right thrust
- **Space** - Upward thrust
- **Shift** - Downward thrust
- **Arrow Up/Down** - Pitch
- **Arrow Left/Right** - Yaw
- **Q** - Roll left
- **E** - Roll right
- **X** - Emergency brake (stops all movement)

**UI Controls (Always Available):**
- **TAB** - Toggle Player Status Panel
- **J** - Toggle Mission Info
- **I** - Toggle Inventory
- **B** - Toggle Ship Builder
- **F1** - Toggle Debug Info
- **F2** - Toggle Entity List
- **F3** - Toggle Resource Info
- **F4** - Toggle Futuristic HUD
- **ESC** - Exit to menu

### Other Menu Options

Options 2-14 are individual system demos for testing specific features:
- Engine Demo - Test ship creation
- Voxel System - Ship building mechanics
- Physics - Newtonian movement simulation
- Procedural Generation - Galaxy sector creation
- And more...

## Configuration

The game creates a configuration file at:
- **Windows**: `%APPDATA%\Codename-Subspace\config.json`
- **Linux**: `~/.config/Codename-Subspace/config.json`
- **macOS**: `~/Library/Application Support/Codename-Subspace/config.json`

Edit this file to customize:
- Graphics settings (resolution, VSync, FPS)
- Audio volumes
- Gameplay parameters
- Development/debug options

## Troubleshooting

### Build Errors

**"SDK not found"**
- Ensure .NET 9.0 SDK is installed
- Restart your terminal after installing

**"Package restore failed"**
- Check internet connection
- Try: `dotnet nuget locals all --clear`
- Then: `dotnet restore`

### Runtime Errors

**"OpenGL not supported"**
- Update your graphics drivers
- Ensure your GPU supports OpenGL 3.3 or higher

**"Failed to create window"**
- Your system may not have display access
- Try running in a graphical environment (not SSH/terminal-only)
- On Linux, ensure you have X11 or Wayland installed

**Crashes on startup**
- Check the log files in: `~/.config/Codename-Subspace/Logs/`
- Report issues with the log file contents

### Performance Issues

**Low FPS**
- Lower resolution in config.json
- Disable VSync if you want uncapped FPS
- Ensure graphics drivers are up to date

**High memory usage**
- Normal for 3D rendering
- Close other applications
- Consider increasing system RAM

## Dependencies

The project uses these main packages (automatically installed):
- **ImGui.NET** (1.91.0+) - UI rendering
- **Silk.NET** (2.21.0) - OpenGL and windowing
- **NLua** (1.7.3) - Lua scripting support

## Development

To modify the code:

```bash
# Open in your favorite editor
code .  # VS Code
rider . # JetBrains Rider

# Or use Visual Studio 2022+ on Windows
# Open: AvorionLike.sln
```

### Project Structure

```
AvorionLike/
├── Core/
│   ├── ECS/          # Entity Component System
│   ├── Physics/      # Physics simulation
│   ├── Graphics/     # 3D rendering
│   ├── UI/           # User interface (including PlayerUIManager)
│   ├── Input/        # Player control system
│   ├── Voxel/        # Ship building
│   └── ... (other systems)
└── Program.cs        # Main entry point
```

## Next Steps

1. **Start the game** with Option 1 (NEW GAME)
2. **Experiment with controls** in Ship Control Mode (Press C)
3. **Explore the UI panels** using keyboard shortcuts
4. **Read the documentation** in the various .md files
5. **Build your own ships** using the Ship Builder (Press B)
6. **Try the demos** to understand individual systems

## Support

- **Issues**: Report on [GitHub Issues](https://github.com/shifty81/AvorionLike/issues)
- **Documentation**: See the various .md files in the repository
- **Logs**: Check `~/.config/Codename-Subspace/Logs/` for debugging info

---

**Happy exploring! 🚀**
