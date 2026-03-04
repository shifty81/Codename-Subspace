# Changelog

All notable changes to Codename:Subspace will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **C++ Engine: Crafting System** — New `CraftingRecipe`, `RecipeDatabase`, `CraftingComponent`, and `CraftingSystem` classes provide recipe-based item crafting with 5 crafting station types (Basic, Forge, Laboratory, Shipyard, Refinery), concurrent job queues with speed multipliers, level gating, 8 default recipes, and save-game serialization (2026-03-04)
- **C++ Engine: Reputation System** — New `FactionReputation`, `ReputationComponent`, and `ReputationSystem` classes provide per-faction reputation tracking with 5 standing levels (Hostile, Unfriendly, Neutral, Friendly, Allied), reputation decay toward neutral, event history logging, standing queries, and save-game serialization (2026-03-04)
- **C++ Engine: Formation System** — New `FormationPattern`, `FormationComponent`, and `FormationSystem` classes provide fleet formation management with 6 pattern types (Line, V, Diamond, Circle, Wedge, Column), slot-based position computation, member management, slot reassignment, and save-game serialization (2026-03-04)
- **C++ Engine: Crafting, Reputation & Formation events** — `GameEvents` namespace now includes `CraftingStarted`, `CraftingCompleted`, `CraftingFailed`, `RecipeLearned`, `ReputationModified`, `StandingChanged`, `ReputationDecayed`, `FormationCreated`, `FormationDisbanded`, `FormationChanged`, `MemberJoined`, and `MemberLeft` event constants (2026-03-04)
- **C++ Engine: 255 new unit tests** for CraftingRecipe (station names, job progress), RecipeDatabase (add, find, filter, defaults), CraftingComponent (start crafting, level/station requirements, collect completed, serialization), CraftingSystem (update jobs, speed multiplier, multiple jobs), FactionReputation (defaults, modify, standings, normalized, names, thresholds), ReputationComponent (add faction, modify rep, get standing, filter by standing, serialization), ReputationSystem (decay), FormationPattern (line, V, diamond, circle, wedge, column, names, max sizes), FormationComponent (add/remove, has member, reassign slots, serialization), FormationSystem, and event constants (2288 → 2543 total tests)
- **C++ Engine: Full Ship Editor Tooling** — Six new subsystems completing the ship editor infrastructure (2026-03-04):
  - `EditorAction` + `EditorHistory`: Undo/redo system supporting 5 action types (PlaceBlock, RemoveBlock, PaintBlock, MultiPlace, MultiRemove) with configurable max history size
  - `EditorClipboard`: Copy/paste/cut with anchor-relative block positioning for duplicating ship sections
  - `EditorSelection`: Multi-block selection with add/remove/toggle, box select, bounds computation, and block gathering from ship
  - `ShipValidator`: Ship design validation with connectivity checks (BFS), engine/generator presence warnings, mass and block count limits
  - `BlockPalette`: Categorized block catalog (Structure, Functional, Weapons) with 11 default entries, category filtering, and type lookup
  - `EditorGrid`: Grid snapping utilities with configurable cell size, world↔grid conversion, visibility toggle, and extent control
  - All systems integrated into `ShipEditorController` with undo/redo-aware Place, Remove, Paint, Cut, Paste, and RemoveSelected operations
- **C++ Engine: 118 new unit tests** for EditorHistory (push/undo/redo/clear/max-size), EditorAction factories, EditorClipboard (copy/paste/clear), EditorSelection (add/remove/toggle/box-select/bounds/gather), ShipValidator (empty/connected/disconnected/engine/generator/limits), BlockPalette (categories/filter/lookup), EditorGrid (snap/cell-to-world/visibility/extent), and integrated editor workflows (undo-place/undo-remove/undo-paint/copy-paste/cut-paste/remove-selected/validation) (2170 → 2288 total tests)
- **C++ Engine: Shield Module System** — New `ShieldModuleComponent` ECS component with 4 shield types (Standard, Hardened, Phase, Regenerative), type-specific absorption multipliers, regen delay, overcharge mechanics with decay, and save-game serialization. New `ShieldSystem` updates shield regen and overcharge decay each frame (2026-03-04)
- **C++ Engine: Status Effect System** — New `StatusEffectComponent` ECS component with 6 status effect types (EMP Disruption, Fire DOT, Radiation DOT, Shield Drain, Engine Jam, Sensor Scramble), resistance multipliers, timed expiry, tick intervals, immunity support, and save-game serialization. New `StatusEffectSystem` ticks effects and removes expired ones each frame (2026-03-04)
- **C++ Engine: Loot/Drop System** — New `LootTableEntry`, `LootTable`, `LootDrop`, `LootComponent`, and `LootSystem` classes provide deterministic seed-based loot rolling with 5 rarity tiers (Common, Uncommon, Rare, Epic, Legendary), luck multipliers, 3 preset loot tables (StandardEnemy, BossEnemy, Asteroid), and save-game serialization (2026-03-04)
- **C++ Engine: Shield, Status Effect & Loot events** — `GameEvents` namespace now includes `ShieldAbsorbed`, `ShieldDepleted`, `ShieldRestored`, `ShieldOvercharged`, `StatusEffectApplied`, `StatusEffectExpired`, `StatusEffectRemoved`, `StatusEffectTick`, `LootGenerated`, `LootCollected`, `LootDropped`, and `RareItemFound` event constants (2026-03-04)
- **C++ Engine: 163 new unit tests** for ShieldModuleComponent (defaults, absorb, hardened, inactive, overcharge, percentage, restore, multipliers, serialization), ShieldSystem (regen, overcharge decay), StatusEffect (basics, names, defaults), StatusEffectComponent (apply, immune, capacity, resistance, remove, clear, serialization), StatusEffectSystem (update, expiry), LootTableEntry (rarity names, weights), LootTable (roll, determinism, luck, presets), LootComponent (generate, looted, serialization), LootSystem, and event constants (2007 → 2170 total tests)
- **C++ Engine: Ammunition System** — New `AmmoType` enum (Standard, ArmorPiercing, Explosive, EMP, Incendiary), `AmmoPool` struct with current/max ammo tracking, reload mechanics (`StartReload`, `UpdateReload`), `CanFire`/`ConsumeAmmo` per-shot consumption, `Refill`, and `GetAmmoPercentage`. `WeaponSystem` provides `GetDefaultAmmoPool()` per weapon archetype and `GetAmmoDamageMultiplier()` per ammo type (2026-03-04)
- **C++ Engine: Target Lock System** — New `TargetLockComponent` ECS component with lock-on acquisition, `LockState` (None/Acquiring/Locked), configurable lock range, acquire time, break range, lock progress percentage, and tracked-target list. New `TargetLockSystem` updates lock state each frame with distance-based acquisition and break logic (2026-03-04)
- **C++ Engine: Sector Anomalies** — New `AnomalyType` enum (Nebula, BlackHole, RadiationZone, IonStorm, GravityWell), `AnomalyData` struct with position, radius, intensity, and generated names. `GalaxyGenerator` now spawns anomalies per sector with configurable `anomalyProbability`, deterministic seeding, and type-specific name generation (2026-03-04)
- **C++ Engine: Advanced Combat & Anomaly events** — `GameEvents` namespace now includes `AmmoDepleted`, `AmmoReloaded`, `TargetLocked`, `TargetLost`, `AnomalyDiscovered`, and `AnomalyEffect` event constants (2026-03-04)
- **C++ Engine: 95 new unit tests** for AmmoPool (CanFire, ConsumeAmmo, Reload, Refill, Percentage), DefaultAmmoPools, AmmoDamageMultiplier, TargetLockComponent (states, progress, zero acquire time), TargetLockSystem (acquire, break, out-of-range, distance, no-physics), AnomalyGeneration (spawn, determinism, probability, types), and event constants (1912 → 2007 total tests)
- **C++ Engine: Collision Layer System** — New `CollisionCategory` bitmask enum with 10 categories (Player, Enemy, Projectile, Asteroid, Station, Debris, Shield, Sensor, Pickup, Missile), bitwise operators, `CollisionPresets` with 10 ready-made presets, `ShouldCollide()` bidirectional filtering, `isTrigger` support for non-physics volumes, and `GetCategoryName()` debug helper. PhysicsSystem now checks collision layers before resolving collisions, and trigger volumes skip physics response (2026-03-02)
- **C++ Engine: 3D A* Pathfinding System** — New `NavGraph` class with add/remove nodes and edges, blocked-node support, cost-weighted edges, `FindNearest()`, and `BuildGrid()` for automatic 3D grid generation with 6-connected neighbors. New `Pathfinder` class with A* search, Euclidean/Manhattan/custom heuristics, and `FindPathByPosition()`. New `PathfindingComponent` ECS component with waypoint tracking, arrival threshold, repath timer, and target management. New `PathfindingSystem` that owns a shared NavGraph/Pathfinder and auto-repaths entities on timer. `SmoothPath()` utility removes collinear waypoints (2026-03-02)
- **C++ Engine: Collision & Pathfinding events** — `GameEvents` namespace now includes `CollisionLayerChanged`, `TriggerEntered`, `TriggerExited`, `PathFound`, `PathNotFound`, `WaypointReached`, `PathCompleted`, and `NavGridBuilt` event constants (2026-03-02)
- **C++ Engine: 218 new unit tests** for CollisionCategory bitwise ops, HasCategory, ShouldCollide, CollisionPresets, GetCategoryName, PhysicsComponent collision layers, PhysicsSystem collision layer filtering, trigger volumes, NavGraph (add/remove nodes/edges, blocking, FindNearest, Clear, BuildGrid, edge weights), Pathfinder (simple, same-node, no-path, blocked, alternate-route, by-position, grid, 3D, invalid-nodes, heuristics, node-cost), SmoothPath, NavPath, PathfindingComponent, PathfindingSystem, and event constants (1694 → 1912 total tests)
- **C++ Engine: Voxel Damage & Structural Integrity System** — Expanded `ShipDamage` with area/splash damage (Manhattan distance fall-off), directional penetrating damage (0.7× per block), repair mechanics (single-block and budget-distributed), structural integrity checking via BFS flood-fill (`StructuralIntegrity`), fragment separation, and damage queries. New `DamageComponent` ECS component with damage history, modifiers, invulnerability, and save-game serialization (2026-03-02)
- **C++ Engine: Octree Spatial Partitioning** — New `Octree` class with `AABB` bounds provides hierarchical spatial indexing for 3D entities with insert/remove, sphere query, box query, nearest-neighbor, K-nearest, subdivision (8 octants), max-depth limiting, and tree rebuild. Complements existing `SpatialHash` for non-uniform entity distributions and LOD queries (2026-03-02)
- **C++ Engine: Voxel Damage & Spatial events** — `GameEvents` namespace now includes `BlockDamaged`, `BlockDestroyed`, `BlockRepaired`, `SplashDamageApplied`, `PenetratingDamageApplied`, `StructuralCheck`, `ShipFragmented`, `IntegrityRestored`, `OctreeRebuilt`, and `SpatialQueryPerformed` event constants (2026-03-02)
- **C++ Engine: 131 new unit tests** for StructuralIntegrity, ShipDamage (splash, penetrating, repair, fragments), DamageComponent serialization, AABB, Octree (insert, remove, queries, subdivision, rebuild), and event constants (1563 → 1694 total tests)
- **C++ Engine: Particle System** — New `ParticleSystem`, `ParticleEmitter`, `ParticleComponent`, and `Particle` classes provide configurable emitters (Point, Sphere, Cone, Box shapes), color interpolation, gravity, deterministic seeding, and 5 built-in effect presets (Explosion, Engine Thrust, Shield Hit, Mining, Hyperdrive) (2026-03-01)
- **C++ Engine: Achievement System** — New `AchievementSystem`, `AchievementComponent`, and `Achievement` classes provide event-driven criteria tracking, category filtering, progress reporting, save-game serialization, and 8 template achievements (First Blood, Explorer, Shipwright, Trader, Veteran, Miner, Fleet Commander, Rich Pilot) (2026-03-01)
- **C++ Engine: Particle & Achievement events** — `GameEvents` namespace now includes `ParticleEmitted`, `ParticleBurst`, `EmitterStarted`, `EmitterStopped`, `AchievementUnlocked`, and `AchievementProgress` event constants (2026-03-01)
- **C++ Engine: 170 new unit tests** for ParticleSystem, ParticleEmitter, ParticleComponent, effect presets, AchievementSystem, AchievementComponent serialization, and templates (1393 → 1563 total tests)
- **C++ Engine: Audio System** — New `AudioSystem`, `AudioComponent`, `AudioClip`, `AudioSource`, and `MusicPlaylist` classes provide clip management, 2D/3D sound playback, fade in/out, music playlists, volume mixing via `AudioSettings`, and save-game serialization (2026-03-01)
- **C++ Engine: Dynamic Quest Generation** — New `QuestGenerator` class creates procedural quests scaled by player level and sector security, with deterministic seeding and batch generation (2026-03-01)
- **C++ Engine: Audio events** — `GameEvents` namespace now includes `SoundPlayed`, `SoundStopped`, `MusicStarted`, `MusicStopped`, and `MusicTrackChanged` event constants (2026-03-01)
- **C++ Engine: 159 new unit tests** for AudioSystem, AudioComponent serialization, MusicPlaylist, fade/volume logic, and QuestGenerator (1234 → 1393 total tests)
- **Title Screen Settings Menu** — Settings button on the title screen now opens a tabbed settings panel (Video, Audio, Controls) for pre-game configuration (2026-02-14)
- **Module Class Tag Parsing** — `FilterByShipClass` now properly parses `class:ClassName` tags to enforce ship-class module restrictions (2026-02-14)
- **Tutorial Contextual Tooltips** — New `ContextualTooltip` system in TutorialUI supports registering and rendering hover tooltips and floating hint panels during tutorials (2026-02-14)
- **C++ Engine: ECS Integration for CombatSystem** — CombatSystem now iterates all CombatComponents via EntityManager to automatically regenerate energy and shields each frame (2026-02-14)
- **C++ Engine: ECS Integration for NavigationSystem** — NavigationSystem now iterates all HyperdriveComponents via EntityManager to update hyperdrive charge and cooldown timers each frame (2026-02-14)
- **C++ Engine: 18 new unit tests** for ECS-integrated CombatSystem and NavigationSystem (1140 → 1158 total tests)
- **C++ Engine: Quest & Tutorial Persistence** — QuestComponent and TutorialComponent now support Serialize/Deserialize for save-game round-trips (2026-02-15)
- **C++ Engine: 70 new unit tests** for quest and tutorial serialization (1164 → 1234 total tests)
- **Server Discovery** — Multiplayer server browser now probes configured addresses via TCP instead of showing a static placeholder (2026-02-15)
- **Galaxy Map UI** - Interactive navigation and exploration interface (2025-11-10)
  - 2D sector grid visualization with tech level coloring (Iron to Avorion)
  - Zoom (0.2x to 5x) and pan controls for galaxy exploration
  - Z-slice navigation for 3D galaxy layers
  - Sector selection with detailed information panel
  - Real-time hyperdrive jump initiation and status tracking
  - Jump range visualization with circle overlay
  - Filters for stations, asteroids, ships, and jump range
  - Procedural sector generation with caching system
  - Current location indicator and hover tooltips
  - Right-click to jump, M key to toggle map
  - Integration with NavigationSystem and HyperdriveComponent
  - Comprehensive GALAXY_MAP_GUIDE.md documentation
- Version tracking system with VersionInfo class
- "About / Version Info" menu option (option 15) in main menu
- System requirements display in version info
- Comprehensive CHANGELOG.md documenting project history
- EditorConfig for consistent code style across contributors
- Rarity filtering in Subsystem Management UI for easier inventory management

### Changed
- Fixed all compiler warnings for cleaner build output
- Updated ImGui.NET package version to 1.91.0.1 for better compatibility
- Improved null safety in CrewSystem serialization
- Added null check in SubsystemManagementUI for safer entity operations
- Marked unused configuration fields with pragma suppressions for future feature development
- Application header now displays version information dynamically
- Extracted energy regeneration rate as named constant in CombatSystem for better maintainability

### Fixed
- Fixed 3 null reference warnings in SystemVerification.cs by adding null-forgiving operators after Assert null checks (2025-11-08)

### Performance
- Enabled OpenGL face culling for voxel rendering (~50% fewer fragments to render) (2025-11-10)

## [0.9.0] - 2025-11-05

### Added - Player UI & Full Gameplay Experience
- **Integrated Player UI System** - Complete gameplay interface with ImGui.NET
  - Main HUD showing FPS, entity count, and keyboard controls
  - Debug overlay (F1) with system stats and memory tracking
  - Entity list viewer (F2) for component inspection
  - Resource tracking panel (F3)
  - Main menu system (New Game, Continue, Load, Settings, Exit)
  - Pause menu with in-game settings
  - Settings menu with Graphics, Audio, and Controls tabs
  - Inventory management UI with resource tracking
- **6DOF Ship Controls** - Full degrees of freedom ship movement
  - Toggle between Camera and Ship control modes (C key)
  - WASD for thrust in ship mode
  - Arrow keys + Q/E for ship rotation (pitch, yaw, roll)
  - X key for emergency brake
  - Space/Shift for vertical thrust
- **Player Status HUD** - Real-time ship information display
  - Ship integrity and health display
  - Velocity and acceleration meters
  - Resource inventory tracking
  - Mission and objective tracking
- **Ship Builder UI** - In-game ship construction interface
  - Block placement and removal
  - Material selection
  - Real-time ship stats display
  - Save/load ship designs

### Improved
- Enhanced graphics window with integrated UI rendering
- Improved input handling for both 3D navigation and UI interaction
- Better game pause functionality when UI menus are open
- Performance optimizations for UI rendering over 3D graphics

## [0.8.0] - 2025-10-28

### Added - 3D Graphics & Visualization
- **3D Graphics Rendering System**
  - Real-time OpenGL rendering via Silk.NET
  - Voxel mesh generation and rendering for ship blocks
  - Phong lighting model (ambient, diffuse, specular)
  - Material-based coloring system
- **Camera System**
  - Free-look camera with WASD + mouse controls
  - Smooth camera movement and rotation
  - Multiple camera modes (free-look, follow, orbit)
- **Visual Features**
  - Depth testing and face culling
  - Real-time 3D voxel visualization
  - Multiple entity rendering support
  - Professional lighting and shading

### Added - Faction & Politics System (Stellaris-Inspired)
- **Faction Management System**
  - Dynamic faction creation and management
  - Pop-based faction support system
  - Faction ethics and political alignment
  - Influence mechanics
- **Pop System**
  - Population simulation with happiness tracking
  - Political faction affiliation
  - Job assignment and productivity
  - Unrest and rebellion mechanics
- **Policy System**
  - Government policy framework
  - Policy categories: Economy, Military, Diplomacy, Rights, Technology
  - Policy effects on factions and pops
  - Government type system (Democracy, Oligarchy, Dictatorship, etc.)
- **Planet Management**
  - Basic planet infrastructure
  - Building and district system
  - Resource production
  - Pop housing and jobs

### Added - Enhanced Lua Modding System
- **Mod Manager**
  - Automatic mod discovery from Mods/ directory
  - Mod metadata system (name, version, author, description)
  - Mod loading priority and dependency support
  - Enable/disable mods at runtime
- **Extended Lua API**
  - Ship creation and management
  - Faction and pop manipulation
  - Policy management
  - Event system integration
  - Resource management
  - Combat operations
- **Modding Features**
  - Hot-reload support for mod scripts
  - Comprehensive error handling
  - Debug logging for mod development
  - Example mod template provided

### Added - Power Management System
- **Power Distribution**
  - Real-time power generation and consumption tracking
  - Priority-based power allocation
  - Low power mode with system throttling
  - Power reserve system
- **Power Components**
  - Generator blocks with configurable output
  - Consumer blocks with priority levels
  - Battery storage system
  - Power transfer efficiency
- **Power System Integration**
  - Affects ship performance (thrust, shields, weapons)
  - Visual feedback for power status
  - Emergency power protocols
  - Power failure consequences

### Added - Block Stretching & Enhanced Voxel System
- **Block Stretching**
  - Variable block sizes (stretch blocks in X, Y, Z dimensions)
  - Size-based stat scaling (bigger blocks = more stats)
  - Improved ship design flexibility
  - Volume-based calculations for mass and stats
- **Enhanced Block Types**
  - PodDocking blocks for player pod integration
  - GyroArray blocks for improved rotation
  - Cargo blocks with dynamic capacity
  - Enhanced stat calculations per block type

### Added - Strategy Grid & Fleet Management
- **Strategy Grid System**
  - RTS-style top-down strategic view
  - Grid-based movement on 2D strategy layer
  - 3D tactical layer with full physics
  - Sector-based space partitioning
- **Fleet Management**
  - Fleet formation system
  - Command hierarchy
  - Fleet-wide orders
  - Tactical formations
- **Spatial Partitioning**
  - Octree implementation for 3D space
  - Efficient collision detection
  - Fast nearest-neighbor queries
  - LOD (Level of Detail) support

### Added - Subsystem Management
- **Subsystem Inventory System**
  - Store and manage subsystem upgrades
  - Rarity-based subsystem classification
  - Subsystem effects and bonuses
- **Subsystem UI**
  - Visual subsystem management interface
  - Drag-and-drop style equipping
  - Real-time stat preview
  - Subsystem filtering and sorting

### Added - Player Pod System
- **Pod Component**
  - Player character vessel with 50% ship efficiency
  - Upgradeable pod with 5 upgrade slots
  - Base stats: thrust, power, shields, torque
  - Pod leveling and progression
- **Pod Upgrades**
  - Rare upgradeable modules (5 rarity levels)
  - Upgrade types: Thrust, Power, Shield, Efficiency, Torque
  - Loot-based upgrade acquisition
  - Permanent pod improvements
- **Pod Docking**
  - Dock pod into ships with docking ports
  - Pod abilities affect docked ship considerably
  - Level-based bonuses to ship performance
  - Seamless pod transfer between ships
- **Skills & Abilities**
  - Skill tree system across 5 categories
  - 18+ learnable skills with prerequisites
  - Active abilities system (8+ abilities)
  - Cooldown and energy management
  - Temporary power-ups and effects

## [0.5.0] - 2025-09-15

### Added - Core Systems
- **Entity-Component System (ECS)**
  - Flexible entity management
  - Component-based architecture
  - Thread-safe concurrent dictionaries
  - Event-driven lifecycle notifications
- **Voxel Building System**
  - Dynamic ship construction
  - 10+ functional block types
  - Mass and center of mass calculations
  - Structural integrity system
- **Physics System**
  - Newtonian physics simulation
  - 6DOF movement
  - Force and torque calculations
  - Collision detection
- **Procedural Generation**
  - Deterministic sector generation
  - Asteroid field creation
  - Space station spawning
  - Resource distribution
- **Resource Management**
  - Inventory system with capacity limits
  - 10+ resource types
  - Trading system with dynamic pricing
  - Crafting system for upgrades
- **RPG Systems**
  - Experience and leveling
  - Skill points and specializations
  - Loot system with rarities
  - Character progression
- **Combat System**
  - Weapon systems
  - Shield mechanics
  - Damage calculation
  - Target locking
- **Navigation System**
  - Hyperdrive mechanics
  - Sector jumping with cooldowns
  - Jump range calculations
  - Navigation planning
- **Multiplayer Networking**
  - Client-server architecture
  - TCP/IP communication
  - Player synchronization
  - Message serialization
- **Scripting System**
  - Lua integration via NLua
  - Hot-reload support
  - Mod loading system
  - API bindings for game engine
- **Configuration Management**
  - JSON-based configuration
  - Categories: Graphics, Audio, Gameplay, Network, Development
  - Auto-save and hot-reload
  - Validation system
- **Logging System**
  - Multi-level logging (Debug, Info, Warning, Error, Critical)
  - File and console output
  - Color-coded messages
  - Background processing
- **Event System**
  - Decoupled system communication
  - Subscribe/Unsubscribe pattern
  - 40+ game events
  - Type-safe event data
- **Persistence System**
  - Save/Load game state
  - JSON serialization
  - Quick save functionality
  - Save file management
- **Development Tools**
  - Performance profiler
  - Memory tracker
  - Debug console with commands
  - OpenGL debugger
  - Script compiler for hot-reloading

## [0.1.0] - 2025-08-01

### Added
- Initial project setup
- Basic project structure
- .NET 9.0 configuration
- Core dependencies (NLua, Silk.NET, ImGui.NET)
- README and initial documentation
- License (MIT)

---

## Version History Summary

- **0.9.0** - Player UI & Full Gameplay Experience
- **0.8.0** - 3D Graphics, Factions, Enhanced Modding, Power System, Fleet Management
- **0.5.0** - Core Systems (ECS, Physics, Networking, Scripting, etc.)
- **0.1.0** - Initial Release

---

## Upgrade Notes

### From 0.8.0 to 0.9.0
- New UI system requires ImGui.NET 1.91.0+
- Player controls now toggle between Camera and Ship modes with C key
- Ship builder accessible via B key during gameplay
- Settings now accessible in-game via pause menu

### From 0.5.0 to 0.8.0
- Lua mods now auto-load from Mods/ directory
- New faction system requires pops to be created for existing saves
- Power system affects ship performance - ensure ships have generators
- Block stretching changes ship mass calculations - existing ships may perform differently

---

## Future Roadmap

See [NEXT_STEPS.md](NEXT_STEPS.md) for detailed development roadmap and priorities.

### Planned Features
- ~~**AI System**~~ ✅ NPC behaviors, pathfinding, decision making (implemented — A* pathfinding, collision layers, AI steering)
- ~~**Voxel Damage**~~ ✅ Destructible blocks, structural integrity, splash/penetrating damage, repair (implemented)
- ~~**Advanced Combat**~~ ✅ Ammunition system, target locking, ammo types and damage multipliers (implemented)
- ~~**Enhanced Procedural Generation**~~ ✅ Sector anomalies (nebulas, black holes, radiation zones, ion storms, gravity wells) (implemented)
- ~~**Performance Optimizations**~~ ✅ Spatial partitioning (SpatialHash + Octree), collision layers, collision queries (implemented)
- **Network Enhancements** - Client prediction, lag compensation, better synchronization
- ~~**Tutorial System**~~ ✅ Interactive tutorials, tooltips, help system (implemented)
- ~~**Achievement System**~~ ✅ Milestones, rewards, progression tracking (implemented)
- ~~**Audio System**~~ ✅ Sound effects, music, 3D spatial audio (framework implemented)
- ~~**Particle System**~~ ✅ Visual effects for explosions, engines, shields (implemented)
