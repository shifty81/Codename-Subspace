# Codename:Subspace — Space Game Engine

An Avorion-inspired space exploration and combat game engine, actively transitioning to a **C++17 / OpenGL** core. The new C++ engine (`engine/`) implements high-performance block-based and modular ship construction, while the C# prototype (`AvorionLike/`) provides a playable reference with 19+ backend systems.

> **🎮 Project Status:** Active Development — C++ engine conversion in progress
> **🌌 Inspired by:** Avorion, EVE Online, X4: Foundations
> **🎯 Goal:** Performant, moddable space game with modular ship construction and salvage-driven gameplay
> **🔧 C++ Engine:** `engine/` — see [Engine README](engine/README.md)
> **📂 Documentation:** Reorganized into [`docs/`](docs/) — guides, architecture, implementation notes

> **✨ LATEST (Mar 2026):**
> - 🔨 **C++ CRAFTING SYSTEM** — 5 station types, 8 default recipes, concurrent job queues, level gating, speed multipliers ✨ **NEW!**
> - 🤝 **C++ REPUTATION SYSTEM** — Per-faction reputation tracking, 5 standing levels, reputation decay, event history ✨ **NEW!**
> - 🎖️ **C++ FORMATION SYSTEM** — 6 formation patterns (Line, V, Diamond, Circle, Wedge, Column), slot-based positioning ✨ **NEW!**
> - 🛡️ **C++ SHIELD MODULE SYSTEM** — 4 shield types with absorption multipliers, overcharge mechanics, regen delay
> - ⚡ **C++ STATUS EFFECT SYSTEM** — 6 effect types (EMP, Fire, Radiation, Shield Drain, Engine Jam, Sensor Scramble) with timed expiry
> - 💎 **C++ LOOT/DROP SYSTEM** — 5 rarity tiers, deterministic seeded rolling, luck multipliers, 3 preset loot tables
> - 🎯 **C++ AMMUNITION SYSTEM** — AmmoPool with reload mechanics, 5 ammo types, per-weapon-archetype defaults
> - 🔒 **C++ TARGET LOCK SYSTEM** — Lock-on acquisition, distance-based tracking, configurable break range
> - 🌌 **C++ SECTOR ANOMALIES** — Nebulas, black holes, radiation zones, ion storms, gravity wells in galaxy generation
> - 💥 **C++ VOXEL DAMAGE SYSTEM** — Splash/penetrating damage, structural integrity (BFS flood-fill), repair mechanics, fragment separation
> - 🌳 **C++ OCTREE SPATIAL PARTITIONING** — Hierarchical spatial indexing with sphere/box/nearest-neighbor queries
> - 🎆 **C++ PARTICLE SYSTEM** — Configurable emitters with 5 effect presets (Explosion, Engine Thrust, Shield Hit, Mining, Hyperdrive)
> - 🏆 **C++ ACHIEVEMENT SYSTEM** — Event-driven milestones with criteria tracking, serialization, and 8 template achievements
> - 🧪 **2543 unit tests** passing across all C++ engine systems
> - 📁 **Repository restructured** — docs, scripts, and assets organized into dedicated directories
> - 🔧 **C++ MODULAR SHIP SYSTEM** — Hardpoint-based module assembly with 12 module types, 5 ship archetypes, and faction-aware procedural generation
> - 🚀 **MODULAR SHIP SYSTEM** — Ships built from pre-defined modular parts with hardpoint attachment
> - 🔨 **BLOCK-BASED CONSTRUCTION** — Voxel system for ship building, damage visualization, and mining
> - 🎮 **C# PROTOTYPE PLAYABLE** — Full gameplay loop with mining, combat, trading, navigation
> - 🎭 **5 Factions** — Iron Dominion, Nomad Continuum, Helix Covenant, Ashen Clades, Ascended Archive

## 📁 Repository Structure

```
Codename-Subspace/
├── engine/              # C++ engine (primary development focus)
│   ├── include/         #   Public headers (ships, rendering, factions, weapons, AI, networking)
│   ├── src/             #   Implementations
│   ├── tests/           #   1563 unit tests
│   ├── data/            #   JSON faction definitions
│   └── CMakeLists.txt   #   CMake build configuration
├── AvorionLike/         # C# prototype (playable reference)
├── docs/                # Documentation
│   ├── guides/          #   Feature guides and tutorials
│   ├── architecture/    #   Architecture documents
│   ├── implementation/  #   Implementation summaries
│   ├── design/          #   Design documents and brainstorming
│   └── images/          #   Screenshots and diagrams
├── scripts/             # Build and test scripts
├── tools/               # Development tools
│   └── blender/         #   Blender addons (NovaForge, PCGExporter)
├── assets/              # Game assets and module packs
├── GameData/            # Game data files
└── README.md            # This file
```

## 🔧 C++ Engine (Primary Focus)

The C++ engine implements high-performance ship systems with data-first design. See [engine/README.md](engine/README.md) for full details.

### Building the C++ Engine

```bash
cd engine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run tests (2543 tests)
./build/subspace_tests

# Run the game
./build/subspace_game
```

### Key C++ Systems

| System | Status | Description |
|--------|--------|-------------|
| Block-based Ships | ✅ Complete | Integer grid, 7 materials, 5 shapes, per-block damage |
| **Modular Ships** | ✅ **New** | **Hardpoint-based modules, 12 module types, graph-based hierarchy** |
| **Ship Archetypes** | ✅ **New** | **5 archetypes (Interceptor→Battleship), procedural BFS assembly** |
| Ship Editor | ✅ Complete | Ghost preview, symmetry tools, state machine |
| Weapon System | ✅ Complete | 5 weapon archetypes, hardpoint validation, turrets |
| AI Ship Builder | ✅ Complete | Faction-aware procedural generation, tier-based scaling |
| Faction System | ✅ Complete | 5 factions with silhouette profiles and shape language |
| Blueprint System | ✅ Complete | JSON serialization, deterministic loading |
| Rendering | ✅ Complete | Instanced batching, greedy meshing, chunk system |
| Networking | ✅ Complete | Deterministic build commands for multiplayer replication |
| **Particle System** | ✅ **New** | **Configurable emitters, 4 shapes, 5 effect presets, gravity, color interpolation** |
| **Achievement System** | ✅ **New** | **Event-driven criteria tracking, save-game persistence, 8 template achievements** |
| **Voxel Damage System** | ✅ **New** | **Splash/penetrating damage, structural integrity (BFS), repair, fragmentation** |
| **Octree Spatial Partitioning** | ✅ **New** | **Hierarchical spatial indexing, sphere/box/nearest queries, subdivision** |
| **Ammunition System** | ✅ **New** | **AmmoPool with 5 ammo types, reload mechanics, per-weapon defaults, damage multipliers** |
| **Target Lock System** | ✅ **New** | **Lock-on acquisition/tracking, distance-based break, configurable range/timing** |
| **Sector Anomalies** | ✅ **New** | **5 anomaly types (Nebula, BlackHole, RadiationZone, IonStorm, GravityWell) in galaxy gen** |
| **Shield Module System** | ✅ **New** | **4 shield types (Standard, Hardened, Phase, Regenerative), overcharge, regen delay, absorption multipliers** |
| **Status Effect System** | ✅ **New** | **6 effect types (EMP, Fire, Radiation, Shield Drain, Engine Jam, Sensor Scramble), resistance, timed expiry** |
| **Loot/Drop System** | ✅ **New** | **5 rarity tiers, deterministic seeded rolling, luck multipliers, 3 preset loot tables** |
| **Crafting System** | ✅ **New** | **5 station types, 8 default recipes, concurrent job queues, level gating, speed multipliers** |
| **Reputation System** | ✅ **New** | **Per-faction reputation tracking, 5 standing levels, reputation decay, event history** |
| **Formation System** | ✅ **New** | **6 formation patterns (Line, V, Diamond, Circle, Wedge, Column), slot-based positioning** |

## 🎮 C# Prototype

The C# prototype remains playable with 19+ backend systems. See [docs/guides/HOW_TO_BUILD_AND_RUN.md](docs/guides/HOW_TO_BUILD_AND_RUN.md).

```bash
dotnet run
# Select Option 1: NEW GAME - Start Full Gameplay Experience
```

**Controls:**
- **C** - Toggle between Camera and Ship Control
- **WASD + Space/Shift** - Movement/Thrust
- **Arrow Keys + Q/E** - Ship rotation
- **M** - Galaxy Map | **~** - Testing Console
- **TAB** - Player Status | **I** - Inventory | **B** - Ship Builder
- **ESC** - Pause Menu

## 🌟 Overview

Codename:Subspace features two parallel systems: a high-performance **C++ engine** for ship construction and a **C# prototype** for full gameplay. The C++ engine uses data-first design with integer math, deterministic builds, and GPU-optimized rendering. Ships are built from modular parts that snap together via hardpoints, enabling both player building and AI procedural generation.

### Ship Design Systems

**Modular Ship Construction (C++ Engine):** Ships assembled from modules (Core, Engine, Weapon, Hull, Cargo, Shield, Utility) connected via hardpoints. Supports procedural generation with faction-specific silhouettes.

**Block-based Construction (C++ Engine):** Voxel grid system for fine-grained ship building with 7 material tiers (Iron → Avorion).

For more information, see [docs/guides/MODULAR_SHIP_SYSTEM_GUIDE.md](docs/guides/MODULAR_SHIP_SYSTEM_GUIDE.md)

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

### 7. 3D Graphics Rendering **ENHANCED** 🎨

Real-time OpenGL rendering with enhanced visual quality:
- **PBR Materials**: Metallic, roughness, and emissive properties for realistic surfaces
- **Procedural Textures**: Dynamic texture generation with multiple pattern types
  - Paneled hulls with panel lines, seams, and highlights
  - Hexagonal honeycomb patterns for armor plating
  - Cracked patterns for asteroids, crystalline for special materials
  - 10 different procedural patterns to choose from
- **Two-Sided Rendering**: Fixed hollow-looking blocks - now solid from all angles
- **Voxel-Based Rendering**: Optimized mesh generation with greedy meshing and face culling
- **Camera System**: Free-look and ship-follow modes with smooth interpolation
- **Starfield**: Dynamic star rendering with parallax effects
- **Debug Visualization**: AABB rendering, wireframes, and performance stats

**Texture Customization**: 
- See [TEXTURE_CUSTOMIZATION_GUIDE.md](TEXTURE_CUSTOMIZATION_GUIDE.md) for how to customize colors and patterns
- Built-in material library with Hull, Armor, Rock, Ice, Metal, and more
- Easy-to-edit color values and pattern parameters

**Key Classes:**
- `GraphicsWindow` - Main graphics window and rendering loop
- `ProceduralTextureGenerator` - Pattern generation algorithms
- `TextureMaterial` - Material definitions and properties
- `GreedyMeshBuilder` - Optimized voxel mesh generation
- `Camera` - 3D camera with movement and rotation
- `Shader` - OpenGL shader program wrapper
- `VoxelRenderer` - Renders voxel structures as 3D cubes
- `DebugConfig` - Rendering debug options (F7-F12 hotkeys)

**Features:**
- Visualize voxel ships in real-time 3D with enhanced textures
- Navigate around structures with smooth camera controls
- Material differentiation through procedural textures and colors
- Integrated with ECS for seamless entity rendering

### 8. Modular Ship Design System **NEW** 🚀
- **Pre-defined Modules**: Ships built from modular parts (cockpits, engines, wings, weapons, etc.)
- **Attachment System**: Modules connect via attachment points with validation
- **Material Scaling**: Stats scale with material tier (Iron through Avorion)
- **Procedural Generation**: Automatic ship generation from module library
- **Voxel Damage Overlay**: Voxels used ONLY for damage visualization on modules

**Key Classes:**
- `ModularShipComponent` - Ship built from modules (replaces VoxelStructureComponent for ships)
- `ShipModulePart` - Individual module instance with position, health, stats
- `ShipModuleDefinition` - Module type definition with attachment points
- `ModuleLibrary` - Registry of available modules (17 built-in types)
- `ModularProceduralShipGenerator` - Generates ships from modules
- `VoxelDamageSystem` - Creates voxel damage overlays on damaged modules

**Module Categories:**
- Hull (cockpit, sections, corners)
- Engines (main engines, nacelles, thrusters)
- Wings (wings, stabilizers)
- Weapons (mounts, turrets)
- Utility (power, shields, cargo, crew quarters, hyperdrive, sensors, mining)
- Decorative (antennas, details)

**See [MODULAR_SHIP_SYSTEM_GUIDE.md](MODULAR_SHIP_SYSTEM_GUIDE.md) for complete documentation**

### 9. Voxel Architecture (Limited Use)
**Voxels NOW ONLY used for:**
- 🔥 **Damage Visualization**: Shows destroyed sections on modular ships
- ⛏️ **Asteroid Mining**: Asteroids use voxels for mining/deformation
- 💥 **Destruction Effects**: Visual debris and damage

**NOT used for ship construction** (now modular)

**Key Classes:**
- `VoxelBlock` - Individual voxel with position, size, material
- `VoxelStructureComponent` - Used for asteroids only (NOT ships)
- `VoxelDamageComponent` - Damage voxel overlay for modular ships

### 10. Newtonian Physics System
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
- **In-Game Testing Console** - Comprehensive testing tools (press `~` during gameplay) ✨ **NEW!**
- **Script Compiler** - Runtime script compilation and hot-reloading

**Key Classes:**
- `DevToolsManager` - Manages all development tools
- `DebugRenderer` - Visual debug rendering
- `PerformanceProfiler` - Performance metrics tracking
- `MemoryTracker` - Memory usage monitoring
- `OpenGLDebugger` - OpenGL error tracking
- `DebugConsole` - Interactive debug console
- `InGameTestingConsole` - In-game testing and entity spawning ✨ **NEW!**
- `ScriptCompiler` - Runtime script execution

**In-Game Testing Console Features:** ✨ **NEW!**
- 40+ commands for rapid testing and iteration
- Entity spawning (ships, asteroids, enemies, stations)
- Combat and resource testing
- Physics manipulation (teleport, velocity)
- AI behavior control
- Real-time system testing without restart
- See [IN_GAME_TESTING_GUIDE.md](IN_GAME_TESTING_GUIDE.md) for complete documentation

### 18. Blender Integration Tools ✨ **NEW!**

Production-ready Blender addons for asset creation and procedural content generation:

**NovaForge Asset Generator:**
- Project profile management with JSON-based configurations
- Procedural panel and door generators with customizable parameters
- Material presets (Metal, Wood, etc.) with diffuse and specular properties
- Animation presets with automatic keyframe generation
- FBX and JSON export for engine integration

**PCG Exporter:**
- Automatic LOD (Level of Detail) generation with configurable levels
- Collision mesh export with triangle optimization
- Snap point JSON generation for modular construction
- Metadata export (bounding boxes, volume, custom properties)
- Warp mining zone support for space game mechanics
- Adjacency matrix export for structural intelligence learning
- Thumbnail generation for asset preview
- Project profile support with customizable export settings

**Key Features:**
- Seamless integration with C++/OpenGL engine
- Export folder structure optimized for engine consumption
- Support for planets, moons, asteroid belts, and ship modules
- Procedural asset generation with profile-based defaults
- Full documentation in [tools/blender/README.md](tools/blender/README.md)

**Export Structure:**
```
Export_Project/
├── Meshes/         # FBX meshes organized by type
├── LOD/            # Progressive LOD levels
├── Collision/      # Collision meshes
├── SnapPoints/     # Snap point JSON files
├── Metadata/       # Asset metadata and matrices
└── Thumbnails/     # Preview images
```

**See [tools/blender/README.md](tools/blender/README.md) for installation and usage**

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

The application now features a streamlined menu focused on the playable game:

**Main Menu:**
1. **Start New Game** - Full gameplay experience with ship selection, exploration, combat, and trading
2. **About / Version Info** - View game information and system requirements
0. **Exit** - Close the application

> **🎮 For detailed gameplay features, controls, and systems, see [GAMEPLAY_FEATURES.md](GAMEPLAY_FEATURES.md)**

#### Starting Your Journey

When you select **Start New Game**, you will:
1. **Choose your starting ship** from 12 procedurally generated options
   - View detailed stats for each ship
   - Press **V** to preview ships in 3D before choosing
   - Different roles: Combat, Exploration, Mining, Trading, Multipurpose
2. **Enter the game world** at the galaxy rim (Iron Zone)
3. **Control your ship** with full 6DOF movement
4. **Explore the galaxy** - Use **M** key to open the Galaxy Map
5. **Travel to new sectors** via hyperdrive jumps
6. **Mine, trade, fight, and build** your way to the galactic core

#### Game Controls Summary

**Movement:**
- **C** - Toggle between Camera Mode and Ship Control Mode
- **WASD** - Forward/Back/Strafe
- **Space/Shift** - Thrust Up/Down
- **Arrow Keys + Q/E** - Ship Rotation

**UI & Navigation:**
- **M** - Galaxy Map (sector travel, navigation)
- **ESC** - Pause Menu
- **TAB** - Player Status
- **I** - Inventory
- **B** - Ship Builder
- **~** - In-Game Testing Console (40+ commands)

**Mouse:**
- Free-look camera in Camera Mode
- Hold **ALT** to show cursor for UI interaction
- Automatic cursor management in menus

#### Key Features You Can Use Now

✅ **Galaxy Exploration** - Press **M** to open the Galaxy Map
- View your current sector and surrounding regions
- Right-click sectors to initiate hyperdrive jumps
- Travel toward the galactic center (0,0,0) to unlock better materials
- Tech zones: Iron → Titanium → Naonite → Trinium → Xanion → Ogonite → Avorion

✅ **Ship Building** - Press **B** to modify your ship
- 9 block types: Armor, Engine, Thruster, Generator, Shield, Gyro, Weapon, Cargo, Crew
- 7 material types with different properties
- Real-time voxel construction

✅ **Combat & Mining**
- Find and engage enemy ships
- Mine asteroids for resources
- Manage shields and energy

✅ **Trading & Economy**
- Visit stations to buy/sell resources
- 12 resource types available
- Dynamic pricing system

✅ **Fleet Management** - Unlocks as you progress
- Hire captains in Ogonite zones
- Command multiple ships
- Automated fleet behaviors

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
- **[Development Status](DEVELOPMENT_STATUS.md)** - 📌 **START HERE** - Current state, priorities, and what's working
- **[Gameplay Features](GAMEPLAY_FEATURES.md)** - 🎮 **COMPLETE GUIDE** - All gameplay features, controls, and systems
- **[What's Left to Implement](WHATS_LEFT_TO_IMPLEMENT.md)** - 📝 **DETAILED** - Breakdown of remaining work and priorities
- **[Roadmap Status](ROADMAP_STATUS.md)** - 📊 **CURRENT STATUS** - Feature completeness and verification results
- **[Next Steps & Recommendations](NEXT_STEPS.md)** - 📌 **COMPREHENSIVE GUIDE** - Detailed analysis and prioritized recommendations
- **[Architecture Diagram](ARCHITECTURE_DIAGRAM.md)** - Visual system architecture and component relationships

### 📚 Detailed Documentation
- **[Quick Start Guide](QUICKSTART.md)** - Get up and running in minutes
- **[How to Build and Run](HOW_TO_BUILD_AND_RUN.md)** - Complete build instructions
- **[Core Gameplay Mechanics](CORE_GAMEPLAY_MECHANICS.md)** - 🎮 Avorion-inspired design and implementation
- **[Galaxy Map Guide](GALAXY_MAP_GUIDE.md)** - 🗺️ Complete galaxy map and sector travel documentation
- **[In-Game Testing Guide](IN_GAME_TESTING_GUIDE.md)** - 🧪 Testing console commands and features
- **[Quest System Guide](QUEST_SYSTEM_GUIDE.md)** - 📋 Quest creation and management **NEW!**
- **[Architecture Review](ARCHITECTURE.md)** - Comprehensive backend architecture analysis (540+ lines)
- **[Implementation Roadmap](IMPLEMENTATION_ROADMAP.md)** - Detailed development plan and timelines
- **[Executive Summary](EXECUTIVE_SUMMARY.md)** - Backend review summary
- **[Dependencies](DEPENDENCIES.md)** - Complete list of project dependencies
- **[Contributing](CONTRIBUTING.md)** - How to contribute to the project
- **[Credits](CREDITS.md)** - Acknowledgments and licenses
- **[Persistence Guide](PERSISTENCE_GUIDE.md)** - 💾 Complete guide to save/load system
- **[AI System Guide](AI_SYSTEM_GUIDE.md)** - 🤖 Complete guide to AI behavior system
- **[Modding Guide](MODDING_GUIDE.md)** - 🎮 Complete Lua modding documentation

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
