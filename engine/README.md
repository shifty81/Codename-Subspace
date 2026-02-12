# Codename: Subspace — C++ Engine

The new C++ engine implements the Avorion-style block-based ship building system described in the design document (`chat/avorion ship.rtf`). It is built with **C++17** and **OpenGL**, using **CMake** as the build system.

## Architecture Overview

The engine is organized into modular subsystems that follow the design philosophy: **"Ships are data first, visuals second."**

```
engine/
├── include/                    # Public headers
│   ├── core/                   # Math types, engine info
│   │   ├── Math.h              # Vector3Int, Vector3 (integer grid math)
│   │   └── Engine.h            # Engine version info
│   │
│   ├── ships/                  # Ship & block data model
│   │   ├── Block.h             # Block, BlockShape, BlockType, MaterialType, MaterialDatabase
│   │   ├── Ship.h              # Ship container (dual: vector + hashmap)
│   │   ├── BlockPlacement.h    # Placement validation, adjacency, overlap checks
│   │   ├── ShipStats.h         # Emergent stats from blocks (mass, thrust, power)
│   │   ├── ShipDamage.h        # Per-block damage and destruction
│   │   └── Blueprint.h         # JSON blueprint save/load
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
│   └── test_main.cpp           # 118 unit tests covering all systems
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

### Build & Run

```bash
cd engine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run the game
./build/subspace_game

# Run tests (118 tests)
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
