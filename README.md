# Codename:Subspace - Advanced Space Game Engine

A cutting-edge game engine for voxel-based space exploration and combat, featuring real-time 3D graphics rendering with OpenGL. Built with C# and .NET 9.0, cross-platform compatible (Windows, Linux, macOS).

> **🎮 Project Status:** Early Development - Core systems operational & **NOW PLAYABLE!**
> **🌌 Inspired by:** Avorion's gameplay mechanics
> **🎯 Goal:** Create an extensible, moddable space game engine

> **🚀 New to Codename:Subspace?** Check out our [Quick Start Guide](QUICKSTART.md) for one-click setup!
> 
> **✨ LATEST FEATURES:** 
> - 🎮 **INTEGRATED PLAYER UI** - Full gameplay experience with player-controlled ships! ✨ **NEW!**
> - 🕹️ **6DOF Ship Controls** - Realistic space flight with thrust, rotation, and braking
> - 📊 **Player Status HUD** - Real-time ship stats, inventory, and mission tracking
> - 🎭 **Stellaris-Style Faction System** - Dynamic faction politics with pops, policies, and influence
> - 🎮 **Enhanced Lua Modding** - Comprehensive API, mod manager, and auto-discovery
> - ⚡ **Power Management System** - Dynamic power distribution with priority controls
> - 🔨 **Block Stretching** - Create elaborate ship designs with size-based stat scaling  
> - 🌐 **Strategy Grid** - RTS-style fleet management with Octree spatial partitioning
> - 🎨 **3D Graphics** - Real-time OpenGL rendering with PBR materials and starfield
> - 🛡️ **Futuristic HUD** - Holographic-style interface with radar and ship status

## 🎮 How to Play

**[See detailed build & run instructions](HOW_TO_BUILD_AND_RUN.md)**

Quick start:
```bash
dotnet run
# Select Option 1: NEW GAME - Start Full Gameplay Experience
```

**Controls:**
- **C** - Toggle between Camera and Ship Control
- **WASD + Space/Shift** - Movement/Thrust
- **Arrow Keys + Q/E** - Ship rotation
- **TAB** - Player Status, **I** - Inventory, **B** - Ship Builder
- **ESC** - Exit

## 🌟 Overview

Codename:Subspace is a powerful game engine designed for creating voxel-based space games. It features a modular architecture with support for dynamic ship building, Newtonian physics, procedural generation, multiplayer networking, extensive modding capabilities through Lua scripting, and **real-time 3D graphics visualization with integrated player UI**.

## Core Systems

### 1. Entity-Component System (ECS)
- Flexible architecture for managing game objects and their properties
- Efficient component storage and retrieval with thread-safe concurrent dictionaries
- System-based update loop for processing entities
- Event-driven lifecycle notifications
- Comprehensive validation and error handling

**Key Classes:**
- `Entity` - Represents a game object with a unique identifier
- `IComponent` - Interface for all components
- `EntityManager` - Manages entities and their components with validation
- `SystemBase` - Base class for game systems

### 2. Configuration Management **NEW** 🎉
- Centralized game configuration with JSON serialization
- Categories: Graphics, Audio, Gameplay, Network, Development
- Automatic configuration file management
- Validation of configuration values
- Singleton access pattern

**Key Classes:**
- `GameConfiguration` - Comprehensive game settings
- `ConfigurationManager` - Manages configuration lifecycle

### 3. Logging System **NEW** 🎉
- Multi-level structured logging (Debug, Info, Warning, Error, Critical)
- Color-coded console output
- File logging with automatic rotation
- Background log processing for performance
- Thread-safe implementation

**Key Classes:**
- `Logger` - Centralized logging with multiple outputs
- `LogLevel` - Log severity levels
- `LogEntry` - Structured log data

### 4. Event System **NEW** 🎉
- Decoupled communication between systems
- Subscribe/Unsubscribe pattern
- Immediate and queued event publishing
- 40+ predefined game events
- Type-safe event data classes

**Key Classes:**
- `EventSystem` - Centralized event bus
- `GameEvents` - Common event type definitions
- Event data classes (EntityEvent, ResourceEvent, etc.)

### 5. Persistence System **NEW** 🎉
- Save/Load game state to JSON files
- Automatic save directory management
- Quick save functionality
- Save file listing and metadata
- ISerializable interface for components

**Key Classes:**
- `SaveGameManager` - Manages save/load operations
- `SaveGameData` - Save file data structure
- `ISerializable` - Interface for serializable objects

### 6. Validation & Error Handling **NEW** 🎉
- Parameter validation utilities
- Consistent exception handling
- Defensive programming patterns
- Try-Execute helpers

**Key Classes:**
- `ValidationHelper` - Common validation operations
- `ErrorHandler` - Centralized error handling

### 7. 3D Graphics Rendering **NEW** 🎨
- Real-time OpenGL-based 3D rendering using Silk.NET
- Voxel mesh generation and rendering
- Free-look camera with WASD and mouse controls
- Phong lighting model (ambient, diffuse, specular)
- Material-based coloring for different block types
- Cross-platform windowing and input handling

**Key Classes:**
- `Camera` - 3D camera with movement and rotation
- `Shader` - OpenGL shader program wrapper
- `VoxelRenderer` - Renders voxel structures as 3D cubes
- `GraphicsWindow` - Main graphics window and rendering loop

**Features:**
- Visualize voxel ships in real-time 3D
- Navigate around structures with smooth camera controls
- Material differentiation through colors
- Integrated with ECS for seamless entity rendering

### 8. Voxel-Based Architecture
- Arbitrary-sized blocks for flexible ship and station construction
- Automatic center of mass and total mass calculation
- Collision detection between voxel blocks

**Key Classes:**
- `VoxelBlock` - Represents a single voxel with position, size, and material properties
- `VoxelStructureComponent` - Component containing voxel structure data for entities

### 9. Newtonian Physics System
- Realistic physics simulation with forces, acceleration, velocity
- Linear and rotational motion support
- Drag and collision detection
- Elastic collision response

**Key Classes:**
- `PhysicsComponent` - Component for physics properties
- `PhysicsSystem` - System that handles physics simulation

### 10. Procedural Generation
- Deterministic galaxy sector generation using seed-based algorithms
- Procedural asteroid fields with resource types
- Random station generation with various types
- Consistent generation based on coordinates

**Key Classes:**
- `GalaxyGenerator` - Generates galaxy sectors with asteroids and stations
- `GalaxySector` - Represents a sector in the galaxy
- `AsteroidData`, `StationData`, `ShipData` - Data structures for sector objects

### 11. Scripting API (Lua Integration) **ENHANCED** 🎮
- NLua-based scripting engine for comprehensive modding support
- Powerful Lua API wrapper with 30+ functions for game system access
- Automatic mod discovery from AppData/Mods directory
- Mod dependency management and load ordering
- Hot-reloading support for rapid development
- Sample mod templates and extensive documentation

**Key Classes:**
- `ScriptingEngine` - Manages Lua scripting and mod loading
- `LuaAPI` - Comprehensive API wrapper for Lua scripts
- `ModManager` - Handles mod discovery, dependencies, and loading
- `ScriptCompiler` - Runtime compilation and hot-reloading

**Lua API Features:**
- Entity management (create, destroy, query)
- Voxel system access (add blocks, materials)
- Physics control (forces, velocity, position)
- Resource management (inventory, resources)
- Event system integration
- Galaxy generation access

**See [MODDING_GUIDE.md](MODDING_GUIDE.md) for complete documentation**

### 12. Networking/Multiplayer
- TCP-based client-server architecture
- Sector-based multiplayer with server-side sector management
- Multi-threaded sector handling for scalability
- Message-based communication protocol

**Key Classes:**
- `GameServer` - Main server for handling multiplayer connections
- `ClientConnection` - Represents a connected client
- `SectorServer` - Manages a single sector on the server
- `NetworkMessage` - Message structure for network communication

### 13. Resource and Inventory Management
- Multiple resource types (Iron, Titanium, Naonite, etc.)
- Inventory system with capacity limits
- Crafting system for ship upgrades
- Subsystem upgrades (shields, weapons, cargo)

**Key Classes:**
- `Inventory` - Manages resource storage
- `InventoryComponent` - Component for entity inventory
- `CraftingSystem` - Handles crafting of upgrades
- `SubsystemUpgrade` - Represents a ship upgrade

### 14. RPG Elements
- Ship progression with experience and levels
- Faction relations and reputation system
- Loot drop system
- Trading system with buy/sell mechanics

**Key Classes:**
- `ProgressionComponent` - Manages entity progression
- `FactionComponent` - Handles faction relations
- `LootSystem` - Generates loot drops
- `TradingSystem` - Manages resource trading

### 15. Grand Strategy: Stellaris-Style Faction System **NEW** 🎭
- Comprehensive faction political simulation inspired by Stellaris
- Pop-based society with individual happiness and faction alignment
- 11 ethics types (Militarist, Pacifist, Materialist, Xenophile, etc.)
- 7 government types affecting faction behavior (Democracy, Autocracy, etc.)
- Dynamic policy system with 11+ policies affecting faction approval
- Influence resource generation based on faction approval and support
- Rebellion risk system for unhappy factions
- Planet stability calculations based on pop happiness

**Key Features:**
- **Pops (Population Units)**: Individual citizens with ethics, happiness, and faction loyalty
- **Faction Demands**: Each faction has 2-4 demands that must be met for approval
- **Policy Management**: Enact policies that please some factions while angering others
- **Approval & Influence**: Happy factions generate influence for diplomacy and expansion
- **Government Types**: Different governments (Democracy, Oligarchy, etc.) handle factions differently
- **Dynamic Support**: Pop alignment shifts based on living conditions and faction approval
- **Rebellion System**: Very unhappy factions with high unrest may rebel

**Key Classes:**
- `FactionSystem` - Main system managing all faction mechanics
- `Faction` - Individual faction with ethics, approval, demands
- `Pop` - Population unit with happiness and faction alignment
- `Planet` - Contains pops with stability calculations
- `Policy` - Game policies with faction approval modifiers
- `PolicyManager` - Manages policy enactment and effects

**Example Use Cases:**
- Build a militaristic empire by pleasing the Militarist faction
- Balance competing faction demands for stable rule
- Suppress dissenting factions in authoritarian governments
- Watch faction support shift based on your policies
- Manage planetary stability through pop happiness

### 16. AI System **NEW** 🤖
- State-based AI behavior (Idle, Patrol, Mining, Combat, Fleeing, Trading, etc.)
- AI personalities affecting decision-making (Aggressive, Defensive, Miner, Trader, etc.)
- Perception system for environmental awareness
- Intelligent decision-making based on threats, resources, and ship status
- Advanced movement behaviors and combat tactics
- Integration with combat, mining, and navigation systems

**Key Classes:**
- `AISystem` - Main AI system managing all AI entities
- `AIComponent` - AI entity properties and state
- `AIPerceptionSystem` - Environmental awareness and threat detection
- `AIDecisionSystem` - State evaluation and prioritization
- `AIMovementSystem` - Movement and combat maneuvering

**AI States:**
- Idle/Patrol - Default behavior and waypoint navigation
- Mining/Salvaging - Resource gathering operations
- Trading - Commerce at stations
- Combat - Engaging hostile entities with various tactics
- Fleeing - Retreat when severely damaged
- ReturningToBase - Navigate home when needed

**Combat Tactics:**
- Aggressive - Direct frontal assault
- Kiting - Maintain distance while attacking
- Strafing - Circle around target
- Broadsiding - Position for maximum turret coverage
- Defensive - Stay at range and evade

**See [AI_SYSTEM_GUIDE.md](AI_SYSTEM_GUIDE.md) for complete documentation**

### 17. Development Tools
- **Debug Renderer** - Debug visualization for game objects and physics
- **Performance Profiler** - FPS and frame timing tracking
- **Memory Tracker** - Memory usage monitoring (including GPU when available)
- **OpenGL Debugger** - Error detection and logging for graphics
- **Debug Console** - Runtime command console (press `` ` `` key)
- **Script Compiler** - Runtime script compilation and hot-reloading

**Key Classes:**
- `DevToolsManager` - Manages all development tools
- `DebugRenderer` - Visual debug rendering
- `PerformanceProfiler` - Performance metrics tracking
- `MemoryTracker` - Memory usage monitoring
- `OpenGLDebugger` - OpenGL error tracking
- `DebugConsole` - Interactive debug console
- `ScriptCompiler` - Runtime script execution

## Getting Started

### Prerequisites

#### For Visual Studio 2022 Users (Recommended)
- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Download: https://visualstudio.microsoft.com/vs/
  - Required Workloads: .NET desktop development
- **.NET 9.0 SDK or later** (included with Visual Studio 2022)
- **Windows** (for Visual Studio 2022)

#### For Command Line / Other IDEs
- **.NET 9.0 SDK or later**
  - Download: https://dotnet.microsoft.com/download
- **Windows, Linux, or macOS**
- Any text editor or IDE (VS Code, Rider, etc.)

**Note:** The current implementation uses a cross-platform console interface. For a Windows-specific GUI version using Windows Forms, modify the `.csproj` file to target `net9.0-windows` and enable Windows Forms by adding `<UseWindowsForms>true</UseWindowsForms>` to the PropertyGroup section. This requires building on a Windows machine.

### Building with Visual Studio 2022

1. **Clone the repository**
   ```bash
   git clone https://github.com/shifty81/AvorionLike.git
   cd AvorionLike
   ```

2. **Open the solution**
   - Open `AvorionLike.sln` in Visual Studio 2022
   - The solution will automatically restore NuGet packages

3. **Build and Run**
   - Press `F5` to build and run with debugging
   - Or press `Ctrl+F5` to run without debugging
   - Or use Build → Build Solution (Ctrl+Shift+B)

### Building with Command Line

```bash
# Clone the repository
git clone https://github.com/shifty81/AvorionLike.git
cd AvorionLike

# Navigate to project directory
cd AvorionLike

# Restore dependencies
dotnet restore

# Build the project
dotnet build

# Run the application
dotnet run
```

### Installing .NET 9.0 SDK

If you don't have .NET 9.0 SDK installed:

#### Windows
1. Download the installer from [dotnet.microsoft.com/download](https://dotnet.microsoft.com/download)
2. Run the installer
3. Restart your terminal
4. Verify installation: `dotnet --version`

#### Linux (Ubuntu/Debian)
```bash
wget https://dot.net/v1/dotnet-install.sh -O dotnet-install.sh
chmod +x dotnet-install.sh
./dotnet-install.sh --channel 9.0
export PATH="$PATH:$HOME/.dotnet"
```

Or follow the official guide: [Install .NET on Linux](https://learn.microsoft.com/en-us/dotnet/core/install/linux)

#### macOS
Using Homebrew:
```bash
brew install --cask dotnet-sdk
```

Or download from [dotnet.microsoft.com/download](https://dotnet.microsoft.com/download)

### Running the Application

The application provides an interactive console menu with various demos:

1. **Engine Demo** - Create a test ship with voxel structure, physics, and inventory
2. **Voxel System Demo** - Build custom ship structures
3. **Physics Demo** - Simulate Newtonian physics
4. **Procedural Generation** - Generate galaxy sectors
5. **Resource Management** - Manage inventory and crafting
6. **RPG Systems** - Trading, progression, and loot
7. **Scripting** - Execute Lua scripts
8. **Multiplayer** - Start multiplayer server
9. **Statistics** - View engine statistics
10. **3D Graphics Demo** - **NEW!** 🎨 Visualize voxel ships in real-time 3D
11. **Persistence Demo** - **NEW!** 💾 Save and load game state

#### Using 3D Graphics Visualization

Select option 10 from the main menu to open a 3D graphics window that visualizes your voxel ships!

**Controls:**
- **WASD** - Move camera forward/back/left/right
- **Space/Shift** - Move camera up/down
- **Mouse** - Look around (free camera)
- **ESC** - Close graphics window and return to menu

**Features:**
- Real-time 3D rendering of voxel structures
- Phong lighting with ambient, diffuse, and specular components
- Material-based colors (Iron=gray, Titanium=blue, Naonite=green, etc.)
- Smooth camera controls for navigation
- Automatic creation of demo ships if none exist

If no entities are present when you start the graphics demo, the engine will automatically create three sample ships with different designs to showcase the rendering capabilities.

#### Using Development Tools

Press the backtick key (`` ` ``) during runtime to open the debug console. Available console commands:

- `help` - Show all available commands
- `fps` - Display current FPS and frame timing
- `profile` - Generate performance profile report
- `memory` - Show memory usage statistics
- `glerrors` - Display OpenGL errors (when rendering is active)
- `scripts` - List loaded Lua scripts
- `debug` - Toggle debug rendering
- `devtools` - Show status of all development tools
- `compile <file>` - Compile and load a Lua script
- `reload <script>` - Reload a previously loaded script
- `lua <code>` - Execute Lua code directly
- `gc` - Force garbage collection
- `clear` - Clear console output
- `exit` - Close the debug console

## Architecture

```
AvorionLike/
├── Core/
│   ├── ECS/              # Entity-Component System
│   │   ├── Entity.cs
│   │   ├── IComponent.cs
│   │   ├── EntityManager.cs
│   │   └── SystemBase.cs
│   ├── Graphics/         # 3D Rendering System (NEW!)
│   │   ├── Camera.cs
│   │   ├── Shader.cs
│   │   ├── VoxelRenderer.cs
│   │   └── GraphicsWindow.cs
│   ├── Voxel/            # Voxel-based architecture
│   │   ├── VoxelBlock.cs
│   │   └── VoxelStructureComponent.cs
│   ├── Physics/          # Physics system
│   │   ├── PhysicsComponent.cs
│   │   └── PhysicsSystem.cs
│   ├── Procedural/       # Procedural generation
│   │   └── GalaxyGenerator.cs
│   ├── Scripting/        # Lua scripting API
│   │   └── ScriptingEngine.cs
│   ├── Networking/       # Multiplayer networking
│   │   └── GameServer.cs
│   ├── Resources/        # Resource and inventory management
│   │   ├── Inventory.cs
│   │   └── CraftingSystem.cs
│   ├── RPG/              # RPG elements
│   │   └── RPGSystems.cs
│   ├── DevTools/         # Development tools
│   │   ├── DevToolsManager.cs
│   │   ├── DebugRenderer.cs
│   │   ├── PerformanceProfiler.cs
│   │   ├── MemoryTracker.cs
│   │   ├── OpenGLDebugger.cs
│   │   ├── DebugConsole.cs
│   │   └── ScriptCompiler.cs
│   └── GameEngine.cs     # Main engine class
├── Program.cs            # Application entry point
└── AvorionLike.csproj    # Project configuration
```

## Example Usage

### Creating a Ship Entity

```csharp
var engine = new GameEngine(12345);
engine.Start();

// Create entity
var ship = engine.EntityManager.CreateEntity("Player Ship");

// Add voxel structure
var voxelComponent = new VoxelStructureComponent();
voxelComponent.AddBlock(new VoxelBlock(new Vector3(0, 0, 0), new Vector3(2, 2, 2), "Iron"));
engine.EntityManager.AddComponent(ship.Id, voxelComponent);

// Add physics
var physicsComponent = new PhysicsComponent
{
    Position = new Vector3(100, 100, 100),
    Mass = voxelComponent.TotalMass
};
engine.EntityManager.AddComponent(ship.Id, physicsComponent);

// Update engine (call in game loop)
engine.Update();
```

### Using the Scripting API

```csharp
var engine = new GameEngine();

// Execute Lua script
engine.ExecuteScript(@"
    function createShip(name)
        log('Creating ship: ' .. name)
        -- Access engine from Lua
        return name
    end
");

// Call Lua function
var result = engine.ScriptingEngine.CallFunction("createShip", "MyShip");
```

### Starting Multiplayer Server

```csharp
var engine = new GameEngine();
engine.StartServer(27015); // Start on port 27015
```

## Technologies Used

- **C# / .NET 9.0** - Core programming language and framework
- **NLua (v1.7.3)** - Lua scripting integration for modding
- **Silk.NET (v2.21.0)** - Cross-platform OpenGL rendering and windowing
- **System.Numerics** - Vector math for physics and positions
- **System.Net.Sockets** - TCP networking for multiplayer
- **Visual Studio 2022** - Primary development environment

For detailed credits and acknowledgments, see [CREDITS.md](CREDITS.md).

## Features

✅ Entity-Component System (ECS) architecture  
✅ Voxel-based ship/station building  
✅ Newtonian physics simulation  
✅ Procedural galaxy generation  
✅ **Enhanced Lua scripting with comprehensive API** 🎮 **NEW!**  
✅ **Automatic mod discovery and management** 🎮 **NEW!**  
✅ **Mod dependency resolution** 🎮 **NEW!**  
✅ TCP multiplayer networking  
✅ Resource and inventory management  
✅ Crafting system  
✅ RPG progression and faction systems  
✅ **Stellaris-style faction political system** 🎭 **NEW!**  
✅ **Pop-based society simulation** 🎭 **NEW!**  
✅ **Policy management with faction reactions** 🎭 **NEW!**  
✅ **Influence generation and approval mechanics** 🎭 **NEW!**  
✅ Trading system  
✅ Loot generation  
✅ Development tools (Debug Console, Profiler, Memory Tracker)  
✅ Runtime script compilation and hot-reloading  
✅ Visual Studio 2022 solution support  
✅ **Configuration management system** 🎉  
✅ **Structured logging with file output** 🎉  
✅ **Event system for decoupled communication** 🎉  
✅ **Validation and error handling utilities** 🎉  
✅ **Save/Load persistence system** 🎉 💾  
✅ **Real-time 3D graphics rendering** 🎨  
✅ **OpenGL-based voxel visualization** 🎨  
✅ **Interactive camera controls** 🎨  
✅ **ImGui.NET UI framework with HUD** 🎨  
✅ **Futuristic sci-fi HUD with radar, ship status, and target tracking** 🎨 **NEW!**  
✅ **AI system with state-based behavior** 🤖 **NEW!**  
✅ **AI perception, decision-making, and movement** 🤖 **NEW!**  

## Future Enhancements

- Advanced rendering features (textures, shadows, post-processing)
- Advanced collision detection with voxel geometry
- Spatial partitioning for physics optimization
- Client-side prediction and lag compensation
- Voxel damage and integrity system
- Ship blueprint system
- Advanced AI features (formation flying, learning behaviors)
- More complex procedural generation algorithms
- Advanced RPG features (quests, dialog systems)
- Steam Workshop integration
- Performance optimizations for large-scale multiplayer

## Documentation

### 📋 Quick Reference
- **[Playability FAQ](PLAYABILITY_FAQ.md)** - ⚡ **QUICK ANSWERS** - Is it playable? What can I do? FAQ format
- **[Playability Status](PLAYABILITY_STATUS.md)** - 📌 **IS IT PLAYABLE?** - Honest assessment of current state and what's needed
- **[Project Summary](PROJECT_SUMMARY.md)** - 📌 **START HERE** - Quick overview of current state and next steps
- **[Next Steps & Recommendations](NEXT_STEPS.md)** - 📌 **COMPREHENSIVE GUIDE** - Detailed analysis and prioritized recommendations (980+ lines)
- **[Architecture Diagram](ARCHITECTURE_DIAGRAM.md)** - Visual system architecture and component relationships

### 📚 Detailed Documentation
- **[Quick Start Guide](QUICKSTART.md)** - Get up and running in minutes
- **[Architecture Review](ARCHITECTURE.md)** - Comprehensive backend architecture analysis (540+ lines)
- **[Implementation Roadmap](IMPLEMENTATION_ROADMAP.md)** - Detailed development plan and timelines
- **[Executive Summary](EXECUTIVE_SUMMARY.md)** - Backend review summary
- **[Dependencies](DEPENDENCIES.md)** - Complete list of project dependencies
- **[Contributing](CONTRIBUTING.md)** - How to contribute to the project
- **[Credits](CREDITS.md)** - Acknowledgments and licenses
- **[Persistence Guide](PERSISTENCE_GUIDE.md)** - 💾 **NEW!** - Complete guide to save/load system
- **[AI System Guide](AI_SYSTEM_GUIDE.md)** - 🤖 **NEW!** - Complete guide to AI behavior system

## Troubleshooting

### Common Issues

#### ".NET SDK is not installed" or "dotnet command not found"
- **Solution**: Install .NET 9.0 SDK from [dotnet.microsoft.com/download](https://dotnet.microsoft.com/download)
- After installation, restart your terminal/command prompt
- Verify with: `dotnet --version`

#### "The current .NET SDK does not support targeting .NET 9.0"
- **Solution**: Your .NET SDK version is too old. Download and install .NET 9.0 SDK or later
- Check your version: `dotnet --version`
- Multiple SDK versions can coexist peacefully

#### NuGet package restore fails
- **Solution**: Check your internet connection
- Try clearing the NuGet cache: `dotnet nuget locals all --clear`
- Then run: `dotnet restore` again

#### Build errors related to NLua
- **Solution**: Make sure NuGet packages are restored correctly
- Run: `dotnet restore` in the AvorionLike project directory
- If issues persist, delete `bin` and `obj` folders and rebuild

#### Permission denied when running setup scripts (Linux/macOS)
- **Solution**: Make the script executable
- Run: `chmod +x setup.sh` or `chmod +x check-prerequisites.sh`

#### Script execution disabled (Windows PowerShell)
- **Solution**: You may need to change the execution policy
- Run PowerShell as Administrator and execute:
  ```powershell
  Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
  ```
- Then try running the setup script again

#### Application crashes or unexpected behavior
- **Solution**: Make sure you built the project successfully
- Try a clean build: Delete `bin` and `obj` folders, then run `dotnet build`
- Check that you're running .NET 9.0 or later: `dotnet --version`

### Getting Help

If you encounter issues not listed here:
1. Check if there's an existing [GitHub Issue](https://github.com/shifty81/AvorionLike/issues)
2. Review the build output for specific error messages
3. Open a new issue with:
   - Your OS and version
   - .NET SDK version (`dotnet --version`)
   - Full error message
   - Steps to reproduce

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

Inspired by the game [Avorion](https://www.avorion.net/) developed by Boxelware.

For detailed credits and acknowledgments of all libraries and inspirations used in this project, please see [CREDITS.md](CREDITS.md).

**Note:** This project is not affiliated with, endorsed by, or connected to Boxelware or the official Avorion game. This is a fan-made educational implementation.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines on:
- Setting up your development environment with Visual Studio 2022
- Code style and conventions
- How to submit changes
- Development workflow

For questions or feedback, please open an issue on GitHub.
