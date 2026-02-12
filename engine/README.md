# Codename: Subspace — C++ Engine

The new C++ engine implements Avorion-style block-based and modular ship building systems. It is built with **C++17** and **OpenGL**, using **CMake** as the build system. Design documents are in [`docs/design/`](../docs/design/).

## Architecture Overview

The engine is organized into modular subsystems that follow the design philosophy: **"Ships are data first, visuals second."**

```
engine/
├── include/                    # Public headers
│   ├── core/                   # Math types, engine info
│   │   ├── Math.h              # Vector3Int, Vector3 (integer grid math)
│   │   ├── Engine.h            # Engine version info
│   │   ├── ecs/                # Entity-Component System (ported from C#)
│   │   │   ├── Entity.h        # Entity with unique ID
│   │   │   ├── IComponent.h    # Base component interface
│   │   │   ├── SystemBase.h    # Base system class
│   │   │   └── EntityManager.h # Entity/component/system management
│   │   ├── events/             # Event System (ported from C#)
│   │   │   ├── GameEvents.h    # Event types and data structs
│   │   │   └── EventSystem.h   # Pub/sub event bus (singleton)
│   │   ├── logging/            # Logging System (ported from C#)
│   │   │   └── Logger.h        # Multi-level logger (singleton)
│   │   ├── physics/            # Physics System (ported from C#)
│   │   │   ├── PhysicsComponent.h # Newtonian physics properties
│   │   │   └── PhysicsSystem.h    # Physics simulation & collision
│   │   └── resources/          # Resource System (ported from C#)
│   │       └── Inventory.h     # Resource inventory management
│   │
│   ├── ships/                  # Ship & block data model
│   │   ├── Block.h             # Block, BlockShape, BlockType, MaterialType, MaterialDatabase
│   │   ├── Ship.h              # Ship container (dual: vector + hashmap)
│   │   ├── BlockPlacement.h    # Placement validation, adjacency, overlap checks
│   │   ├── ShipStats.h         # Emergent stats from blocks (mass, thrust, power)
│   │   ├── ShipDamage.h        # Per-block damage and destruction
│   │   ├── Blueprint.h         # JSON blueprint save/load
│   │   ├── ModuleDef.h         # Modular ship modules, hardpoints, ModuleDatabase ✨ NEW
│   │   └── ShipArchetype.h     # Ship archetypes, procedural generator ✨ NEW
│   │
│   ├── ship_editor/            # Ship editor UI logic
│   │   ├── ShipEditorState.h   # Editor state machine (Place/Remove/Paint/Select)
│   │   ├── ShipEditorController.h # Editor main loop controller
│   │   └── SymmetrySystem.h    # Mirror X/Y/Z symmetry tools
│   │
│   ├── rendering/              # Rendering subsystem
│   │   ├── ShipRenderer.h      # Instanced mesh batching, chunk system, greedy meshing
│   │   └── GhostRenderer.h     # Ghost block preview rendering
│   │
│   ├── factions/               # Faction identity system
│   │   ├── SilhouetteProfile.h # 5-axis silhouette language
│   │   └── FactionProfile.h    # Faction definitions (5 factions)
│   │
│   ├── weapons/                # Weapon & turret system
│   │   └── WeaponSystem.h      # 5 weapon archetypes, hardpoints, turrets
│   │
│   ├── networking/             # Multiplayer determinism
│   │   └── BuildCommand.h      # Deterministic build commands, replication
│   │
│   └── ai/                     # AI ship generation
│       └── AIShipBuilder.h     # Procedural faction ship generator
│
├── src/                        # Implementations
│   ├── main.cpp                # Entry point
│   └── [mirrors include/ structure]
│
├── tests/
│   └── test_main.cpp           # 306 unit tests covering all systems
│
├── data/
│   └── factions/               # JSON faction definitions
│       ├── iron_dominion.json
│       ├── nomad_continuum.json
│       ├── helix_covenant.json
│       ├── ashen_clades.json
│       └── ascended_archive.json
│
└── CMakeLists.txt              # Build configuration
```

## Core Design Principles

1. **Grid-based block assembly** — Integer coordinates, 90° rotation increments, snap-to-grid
2. **Symmetry enforcement** — Mirror X/Y/Z planes for disciplined ship design
3. **Emergent stats** — Ship performance derived entirely from block composition
4. **Per-block damage** — Blocks have individual HP; destruction removes blocks and recalculates stats
5. **Deterministic builds** — Integer math, no floating-point in ship logic, seed-based generation
6. **Instanced rendering** — Blocks grouped by (shape, material) for GPU efficiency
7. **Greedy meshing** — Adjacent same-material faces merged into larger quads
8. **Data-driven factions** — JSON-defined faction profiles with silhouette language

## The 5 Factions

| Faction | Silhouette | Combat Style |
|---------|-----------|--------------|
| **Iron Dominion** | Short, chunky bricks | Slow brawler, broadside cannons |
| **Nomad Continuum** | Long, thin needles | Fast skirmisher, spinal railguns |
| **Helix Covenant** | Radial rings | Area denial, inward flak |
| **Ashen Clades** | Asymmetric raiders | Hit-and-run, burst lancers |
| **Ascended Archive** | Elegant tri-radial | Precision control, beam arrays |

## Weapon Archetypes

| Weapon | Damage | Cooldown | Arc | Best For |
|--------|--------|----------|-----|----------|
| Broadside Cannon | 120 | 4.0s | 120° | Wide ships |
| Spinal Railgun | 800 | 12.0s | 5° | Long ships |
| Inward Flak | 240 | 3.0s | 180° | Ring ships |
| Burst Lancer | 900 | 15.0s | 15° | Raiders |
| Beam Array | 35/s | 1.0s | 60° | Tech ships |

## Building

### Prerequisites

- CMake 3.16+
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- OpenGL development libraries

### Build with Visual Studio (Windows — Recommended)

The engine integrates directly into the Visual Studio solution:

1. Open `AvorionLike.sln` in Visual Studio 2022
2. In Solution Explorer, the **C++ Engine** folder contains:
   - **SubspaceEngine** — Static library with all engine systems
   - **SubspaceGame** — Game executable
   - **SubspaceTests** — 226 unit tests
3. Select **Debug | x64** or **Release | x64**
4. Build → Build Solution (Ctrl+Shift+B)
5. Right-click SubspaceTests → Set as Startup Project → F5 to run tests

**Requirements**: Visual Studio 2022 with "Desktop development with C++" workload.

### Build with CMake (Linux/macOS)

```bash
cd engine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run the game
./build/subspace_game

# Run tests (306 tests)
./build/subspace_tests
```

### Optional: GLFW for windowing

```bash
# Install GLFW
sudo apt install libglfw3-dev    # Ubuntu/Debian
brew install glfw                 # macOS

# Build with GLFW
cmake -B build -DSUBSPACE_USE_GLFW=ON
cmake --build build
```

## Key Systems

### Modular Ship System ✨ NEW

Ships can be built from **modules** that snap together via **hardpoints** — connection points with position and direction.

**Module Types:** Core, Engine, Weapon, Hull, Cargo, Shield, Utility

| Module | Mass | HP | Special |
|--------|------|----|---------|
| Core (Small) | 5 | 200 | 10 power output, 4 hardpoints |
| Core (Medium) | 12 | 400 | 25 power output, 6 hardpoints |
| Engine (Small) | 3 | 80 | 50 thrust, 5 power draw |
| Engine (Large) | 8 | 120 | 150 thrust, 12 power draw |
| Weapon Turret | 4 | 60 | 8 power draw |
| Weapon Railgun | 6 | 70 | 15 power draw |
| Hull Plate | 2 | 150 | 4 hardpoints |
| Cargo (Large) | 8 | 100 | 200 capacity, 4 hardpoints |
| Shield Generator | 5 | 90 | 100 shield, 10 power draw |

**Ship Archetypes** drive procedural generation:

| Archetype | Modules | Weapons | Engines | Aggressiveness |
|-----------|---------|---------|---------|----------------|
| Interceptor | 4–8 | 1–2 | 2 | 0.6 |
| Frigate | 8–14 | 2–4 | 2 | 0.7 |
| Freighter | 6–12 | 0–1 | 1 | 0.1 |
| Cruiser | 12–20 | 3–6 | 3 | 0.8 |
| Battleship | 18–30 | 5–10 | 4 | 0.9 |

**Generation Pipeline:** Core → BFS Hull Growth → Engines → Weapons → Fill (cargo/shields/utility)

**Key Features:**
- Graph-based module hierarchy (parent/child relationships)
- Recursive destruction (destroying a module destroys its subtree)
- Power balance validation (power generation ≥ power draw)
- Faction-aware generation (weapon bias, archetype selection)
- Deterministic seeded RNG for multiplayer-safe generation

### Block Placement System
- Blocks snap to integer grid
- Overlap detection prevents invalid placement
- Adjacency requirement (except first block)
- Symmetry-aware placement (MirrorX/Y/Z)

### Ship Editor
- State machine: Place → Remove → Paint → Select
- Ghost block preview (green=valid, red=invalid)
- Hotkey-driven (X/Y/Z symmetry, R rotate, Q/E cycle shapes)

### AI Ship Builder
- Uses the **same placement API as players**
- Seeded RNG for deterministic generation
- Faction profile drives silhouette, shape language, and system placement
- Tier-based scaling (Scout → Battleship)

### Blueprint System
- Ships serialized as JSON block data
- No meshes, no transforms, no floats
- Deterministic loading via placement API
- Multiplayer-safe, mod-friendly

### Networking
- Build commands (Place/Remove/Paint) for replication
- Server validates, applies, broadcasts
- Clients replay commands deterministically

## Ported from C# Prototype

The following core systems have been ported from the C# prototype (`AvorionLike/`) to C++:

### Entity-Component System (ECS)
- **Entity** — Lightweight ID + name + active flag
- **IComponent** — Base struct for all data components
- **SystemBase** — Abstract base for update-driven systems (enable/disable, initialize/shutdown)
- **EntityManager** — Thread-safe entity creation/destruction, typed component add/get/remove, system registration and update loop
- Equivalent to C# `EntityManager`, `Entity`, `IComponent`, `SystemBase`

### Event System
- **EventSystem** — Singleton pub/sub event bus with immediate and deferred (queued) event processing
- **GameEvents** — 30+ event type constants (entity, component, resource, physics, combat, trading, faction, network, system, sector)
- **Event data structs** — `EntityEvent`, `ResourceEvent`, `CollisionEvent`, `ProgressionEvent`
- Thread-safe with mutex locking
- Equivalent to C# `EventSystem`, `GameEvents`, `GameEvent` hierarchy

### Logger
- **Logger** — Singleton multi-level logging (Debug, Info, Warning, Error, Critical)
- Level filtering, recent log history, console output
- Thread-safe with mutex locking
- Equivalent to C# `Logger`, `LogLevel`, `LogEntry`

### Physics System
- **PhysicsComponent** — Newtonian physics: position, velocity, acceleration, rotation, angular velocity, forces, drag, mass, collision radius
- **PhysicsSystem** — Full simulation loop: force integration (F=ma), exponential drag, velocity clamping, position update, interpolation for smooth rendering, sphere-based collision detection with elastic response
- Equivalent to C# `PhysicsComponent`, `PhysicsSystem`

### Resource/Inventory System
- **Inventory** — Capacity-limited resource storage with 8 resource types (Iron through Avorion + Credits)
- Add/remove/query resources with capacity enforcement
- Equivalent to C# `Inventory`, `ResourceType`

### Migration Status

| C# System | C++ Status | Tests |
|-----------|-----------|-------|
| Entity-Component System | ✅ Ported | 24 tests |
| Event System | ✅ Ported | 12 tests |
| Logger | ✅ Ported | 4 tests |
| Physics System | ✅ Ported | 19 tests |
| Resource/Inventory | ✅ Ported | 21 tests |
| Configuration Manager | ⏳ Planned | — |
| Combat System | ⏳ Planned | — |
| Trading/Economy | ⏳ Planned | — |
| Navigation/Hyperdrive | ⏳ Planned | — |
| RPG/Progression | ⏳ Planned | — |
| Fleet/Crew Management | ⏳ Planned | — |
| AI Decision/Perception | ⏳ Planned | — |
| Procedural Generation | ⏳ Planned | — |
| Persistence/Save-Load | ⏳ Planned | — |
| Networking (full) | ⏳ Planned | — |
| Scripting/Lua | ⏳ Planned | — |
| Quest System | ⏳ Planned | — |
| Tutorial System | ⏳ Planned | — |
| Graphics/UI (ImGui) | ⏳ Planned | — |
