# Class-Specific Modular Ship System - Design Document

## Overview

This document describes the class-specific modular ship system that allows different ship classes (Fighter, Capital, Industrial, etc.) to have their own unique module sets with appropriate crossover modules.

## Goals

1. **Class Identity:** Each ship class should have a distinct appearance and feel
2. **Modularity:** Players can customize ships with compatible modules
3. **Scalability:** Easy to add new ship classes and modules
4. **Balance:** Modules appropriately sized and powered for their class
5. **Flexibility:** Some modules work across multiple classes (universal systems)

## Ship Class Hierarchy

### Combat Classes

| Class | Size | Crew | Role | Key Features |
|-------|------|------|------|--------------|
| **Fighter** | Tiny | 1-2 | Fast interceptor | Agile, light weapons, minimal systems |
| **Corvette** | Small | 3-10 | Escort/patrol | Light armor, point defense, good speed |
| **Frigate** | Medium | 20-50 | Multi-role combat | Balanced, versatile loadouts |
| **Destroyer** | Medium-Large | 100-200 | Heavy combat | Heavy weapons, strong armor |
| **Cruiser** | Large | 200-500 | Major warship | Multiple weapon batteries, strong shields |
| **Battleship** | Huge | 500-1000 | Capital warship | Massive firepower, heavy armor |
| **Carrier** | Huge | 1000+ | Fighter carrier | Hangar bays, fleet command, support |

### Industrial Classes

| Class | Size | Crew | Role | Key Features |
|-------|------|------|------|--------------|
| **Miner** | Small-Large | 5-100 | Resource extraction | Mining lasers, ore storage, refinement |
| **Hauler** | Medium-Huge | 10-200 | Cargo transport | Massive cargo bays, low combat ability |
| **Salvager** | Medium | 20-100 | Wreck recovery | Tractor beams, cutting tools, cargo space |
| **Refinery** | Large | 100-500 | Ore processing | Processing facilities, storage, labs |
| **Constructor** | Huge | 200-1000 | Station building | Construction tools, material storage |

### Special Classes

| Class | Size | Crew | Role | Key Features |
|-------|------|------|------|--------------|
| **Scout** | Tiny-Small | 1-5 | Fast exploration | High speed, long range sensors, stealth |
| **Science** | Medium-Large | 50-300 | Research | Advanced sensors, labs, data storage |
| **Support** | Medium-Large | 30-200 | Fleet support | Repair systems, supplies, medical |

## Module Size Classification

### Size Categories

| Size | Description | Typical Classes | Examples |
|------|-------------|-----------------|----------|
| **Tiny** | Minimal footprint | Fighter only | Fighter cockpit, micro thrusters |
| **Small** | Compact systems | Fighter, Corvette | Small engines, light weapons |
| **Medium** | Standard size | Frigate, Destroyer, Miner | Standard engines, medium cargo |
| **Large** | Heavy systems | Cruiser, Hauler, Science | Large weapons, heavy cargo bays |
| **Huge** | Capital-scale | Battleship, Carrier, Constructor | Capital engines, massive turrets |
| **Massive** | Super-capital | Special/Stations | Station cores, super weapons |

### Size Compatibility Rules

- **Tiny modules:** Fighter only
- **Small modules:** Fighter, Corvette, (Scout)
- **Medium modules:** Corvette, Frigate, Destroyer, (most industrial)
- **Large modules:** Cruiser, Battleship, (large industrial)
- **Huge modules:** Battleship, Carrier, (capital industrial)
- **Massive modules:** Special cases only

**Note:** Parentheses indicate occasional use, not primary size for that class.

## Module Visibility System

### External Modules
**Visible on ship exterior, affect appearance**

- Hull sections (cockpits, body, connections)
- Wings and stabilizers
- Engines and thrusters
- Weapon mounts and turrets
- Antennas and sensors (some)
- Docking ports

**Purpose:** Define the ship's visual identity and silhouette

### Internal Modules
**Hidden inside ship, shown in fitting screen only**

- Power cores and generators
- Shield generators
- Cargo bays and storage
- Crew quarters and life support
- Medical bays
- Engineering sections
- Computer systems
- Internal sensors

**Purpose:** Functional systems that don't need visual representation

### Both (Hybrid)
**Can be external or internal depending on design**

- Sensor arrays (can be external dishes or internal systems)
- Power cores (small ships may have external, large ships internal)
- Specialized systems (mining arrays, science labs)

## Module Categories by Ship Class

### Fighter Class Modules

**Unique to Fighters:**
- Tiny Cockpit (single-seat, streamlined)
- Fighter Wings (small, agile design)
- Light Engine (high thrust-to-weight ratio)
- Light Weapon Mount (fixed or gimbal)
- Micro Thruster (directional control)

**Shared with Other Classes:**
- Small Cargo Bay (emergency supplies)
- Basic Sensors (targeting only)
- Small Power Core (minimal power)

**Example Fighter Build:**
```
1x Tiny Cockpit (required)
2x Fighter Wing
1x Light Engine
2x Light Weapon Mount
4x Micro Thruster
1x Small Power Core
```

### Capital Class Modules (Battleship/Carrier)

**Unique to Capitals:**
- Capital Bridge (war room, extensive crew)
- Capital Hull Sections (massive plating)
- Capital Engine Banks (multiple huge engines)
- Capital Weapon Batteries (massive turrets)
- Hangar Bays (fighter storage - Carrier only)
- Capital Shield Arrays (multiple huge generators)
- Command & Control (fleet coordination)

**Shared with Cruisers:**
- Large Cargo Bays
- Advanced Sensors
- Large Power Cores

**Example Battleship Build:**
```
1x Capital Bridge (required)
8x Capital Hull Section
4x Capital Engine Bank
6x Capital Weapon Battery
2x Capital Shield Array
1x Command & Control
4x Large Cargo Bay
```

### Industrial Class Modules

**Unique to Industrial:**
- Industrial Command Pod (utilitarian bridge)
- Industrial Frame (heavy structural, modular)
- Mining Laser Array (multiple beams)
- Ore Storage (specialized cargo)
- Refinery Module (processing equipment)
- Salvage Tools (tractor beam, cutting laser)
- Constructor Arms (building equipment)

**Shared with Combat:**
- Standard Engines (lower performance)
- Light Weapons (self-defense)
- Basic Shields

**Example Miner Build:**
```
1x Industrial Command Pod (required)
4x Industrial Frame
2x Standard Engine
2x Mining Laser Array
4x Ore Storage
1x Refinery Module (optional)
2x Light Turret (defense)
```

## Universal/Shared Modules

These modules work across multiple or all ship classes:

### Power Systems
- **Tiny Power Core:** Fighter, Scout
- **Small Power Core:** Fighter, Corvette, Scout
- **Medium Power Core:** Corvette, Frigate, Destroyer, Miner
- **Large Power Core:** Cruiser, Battleship, Hauler, Refinery
- **Huge Power Core:** Battleship, Carrier, Constructor
- **Fusion Reactor:** Large ships (Advanced power generation)
- **Antimatter Reactor:** Capital ships (Extreme power generation)

### Defense Systems
- **Light Shield Generator:** Small ships
- **Medium Shield Generator:** Medium ships
- **Heavy Shield Generator:** Large ships
- **Capital Shield Array:** Capital ships only

### Sensors & Electronics
- **Basic Sensors:** All classes (short range)
- **Standard Sensors:** Most classes (medium range)
- **Advanced Sensors:** Large ships, Science (long range)
- **Tactical Array:** Combat ships (targeting enhancement)
- **Science Suite:** Science vessels (research equipment)

### Life Support & Crew
- **Crew Quarters (S/M/L):** Based on crew size
- **Medical Bay (S/M/L):** Based on crew size
- **Life Support System:** All manned ships
- **Escape Pods:** All ships (safety requirement)

### Cargo & Storage
- **Small Cargo Bay:** 50 units capacity
- **Medium Cargo Bay:** 200 units capacity
- **Large Cargo Bay:** 500 units capacity
- **Huge Cargo Bay:** 2000 units capacity
- **Specialized Storage:** Ore, liquids, gases, etc.

## Module Compatibility Matrix

### By Ship Class

| Module Type | Fighter | Corvette | Frigate | Destroyer | Cruiser | Battleship | Carrier | Miner | Hauler |
|-------------|---------|----------|---------|-----------|---------|------------|---------|-------|--------|
| Tiny Cockpit | ✓ | - | - | - | - | - | - | - | - |
| Fighter Wings | ✓ | - | - | - | - | - | - | - | - |
| Small Weapons | ✓ | ✓ | ✓ | - | - | - | - | ✓ | ✓ |
| Medium Weapons | - | ✓ | ✓ | ✓ | ✓ | - | - | - | - |
| Large Weapons | - | - | - | ✓ | ✓ | ✓ | - | - | - |
| Capital Weapons | - | - | - | - | - | ✓ | ✓ | - | - |
| Mining Arrays | - | - | - | - | - | - | - | ✓ | - |
| Hangar Bays | - | - | - | - | - | - | ✓ | - | - |
| Basic Sensors | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Huge Cargo | - | - | - | - | ✓ | ✓ | ✓ | ✓ | ✓ |

**Legend:** ✓ = Compatible, - = Not compatible

## Implementation Architecture

### Code Structure

```
AvorionLike/Core/Modular/
├── ModuleClassification.cs      # NEW: Ship class and size enums
├── ShipModuleDefinition.cs      # MODIFIED: Added Classification property
├── ModuleLibrary.cs              # TO MODIFY: Set classifications
├── ModularShipComponent.cs       # EXISTING: Ship assembly
└── ModularProceduralShipGenerator.cs  # TO MODIFY: Class-aware generation
```

### Key Classes

**ModuleClassification.cs:**
- `ShipClass` enum (14 ship types)
- `ModuleSize` enum (6 sizes)
- `ModuleVisibility` enum (External/Internal/Both)
- `ModuleClassificationInfo` class
- `ModuleClassificationHelper` static utilities

**ShipModuleDefinition.cs:**
```csharp
public class ShipModuleDefinition
{
    // ... existing properties ...
    
    /// NEW: Classification information
    public ModuleClassificationInfo Classification { get; set; } = new();
}
```

**ModuleClassificationInfo:**
```csharp
public class ModuleClassificationInfo
{
    public ShipClass CompatibleClasses { get; set; }  // Which classes can use this
    public ModuleSize Size { get; set; }              // Size category
    public ModuleVisibility Visibility { get; set; }   // External/Internal/Both
    public string StyleVariant { get; set; }          // "military", "industrial", etc.
    public int MaxPerShip { get; set; }               // Quantity limit (0 = unlimited)
    public int MinPerShip { get; set; }               // Minimum required
    
    public bool IsCompatibleWith(ShipClass shipClass) { ... }
}
```

## Usage Examples

### Example 1: Define a Fighter-Only Module

```csharp
var fighterCockpit = new ShipModuleDefinition
{
    Id = "cockpit_fighter",
    Name = "Fighter Cockpit",
    Category = ModuleCategory.Hull,
    ModelPath = "ships/modules/fighters/cockpit_fighter.obj",
    
    Classification = new ModuleClassificationInfo
    {
        CompatibleClasses = ShipClass.Fighter,
        Size = ModuleSize.Tiny,
        Visibility = ModuleVisibility.External,
        StyleVariant = "military",
        MinPerShip = 1,
        MaxPerShip = 1
    }
};
```

### Example 2: Define a Universal Power Core

```csharp
var mediumPowerCore = new ShipModuleDefinition
{
    Id = "power_core_medium",
    Name = "Medium Fusion Reactor",
    Category = ModuleCategory.PowerCore,
    
    Classification = new ModuleClassificationInfo
    {
        CompatibleClasses = ShipClass.Corvette | ShipClass.Frigate | 
                           ShipClass.Destroyer | ShipClass.Miner |
                           ShipClass.Salvager,
        Size = ModuleSize.Medium,
        Visibility = ModuleVisibility.Internal,
        MinPerShip = 1,  // Required for ship to function
        MaxPerShip = 3   // Can have up to 3 for redundancy
    }
};
```

### Example 3: Define a Capital-Only Weapon

```csharp
var capitalTurret = new ShipModuleDefinition
{
    Id = "weapon_capital_turret",
    Name = "Capital Plasma Cannon Battery",
    Category = ModuleCategory.WeaponMount,
    ModelPath = "ships/modules/capital/turret_plasma_battery.obj",
    
    Classification = new ModuleClassificationInfo
    {
        CompatibleClasses = ShipClass.AllCapital,  // Battleship | Carrier
        Size = ModuleSize.Huge,
        Visibility = ModuleVisibility.External,
        StyleVariant = "military",
        MaxPerShip = 8  // Capital ships can have up to 8 main batteries
    }
};
```

### Example 4: Filter Modules for a Ship Class

```csharp
var library = new ModuleLibrary();
library.InitializeBuiltInModules();

// Get all modules compatible with Frigates
var frigateModules = library.AllDefinitions
    .Where(m => m.Classification.IsCompatibleWith(ShipClass.Frigate))
    .ToList();

// Get only external modules for ship appearance
var externalModules = ModuleClassificationHelper.GetExternalModules(frigateModules);

// Get only internal modules for fitting screen
var internalModules = ModuleClassificationHelper.GetInternalModules(frigateModules);
```

## Development Roadmap

### Phase 1: Foundation (Complete) ✅
- [x] Create ModuleClassification system
- [x] Extend ShipModuleDefinition with Classification
- [x] Add helper utilities
- [x] Build verification

### Phase 2: Basic Implementation (In Progress)
- [ ] Update existing 10 modules with classifications
- [ ] Create size variants (Small/Medium/Large) for existing modules
- [ ] Test compatibility filtering

### Phase 3: Fighter Class Expansion
- [ ] Fighter Cockpit model
- [ ] Fighter Wings (agile design)
- [ ] Light Engine model
- [ ] Light Weapon Mounts
- [ ] Micro Thruster models
- **Estimated:** 15-20 new modules

### Phase 4: Capital Class Expansion
- [ ] Capital Bridge model
- [ ] Capital Hull Sections
- [ ] Capital Engine Banks
- [ ] Capital Weapon Batteries
- [ ] Hangar Bay models (Carrier)
- [ ] Shield Array models
- **Estimated:** 20-25 new modules

### Phase 5: Industrial Class Expansion
- [ ] Industrial Command Pod
- [ ] Industrial Frame sections
- [ ] Mining Laser Arrays
- [ ] Ore Storage modules
- [ ] Refinery modules
- [ ] Salvage Tools
- **Estimated:** 15-20 new modules

### Phase 6: Universal/Internal Systems
- [ ] Power Core variants (all sizes)
- [ ] Shield Generator variants
- [ ] Cargo Bay variants
- [ ] Crew Quarter variants
- [ ] Life Support systems
- [ ] Medical Bay variants
- **Estimated:** 25-30 new modules

### Phase 7: UI Integration
- [ ] Ship Fitting Screen
- [ ] Module filter by class
- [ ] External/Internal separation
- [ ] Drag-and-drop module placement
- [ ] Compatibility validation

### Phase 8: Polish & Testing
- [ ] Balance module stats
- [ ] Test all class combinations
- [ ] Performance optimization
- [ ] Visual polish

## Module Count Projections

| Category | Current | Fighter | Capital | Industrial | Universal | Total Target |
|----------|---------|---------|---------|------------|-----------|--------------|
| Cockpits/Bridges | 1 | +3 | +2 | +2 | 0 | 8 |
| Hull Sections | 1 | +5 | +6 | +4 | 0 | 16 |
| Engines | 1 | +3 | +4 | +2 | 0 | 10 |
| Thrusters | 1 | +4 | +3 | +2 | 0 | 10 |
| Wings | 2 | +4 | +2 | 0 | 0 | 8 |
| Weapons | 1 | +5 | +8 | +2 | 0 | 16 |
| Power Cores | 1 | 0 | 0 | 0 | +7 | 8 |
| Shields | 0 | 0 | 0 | 0 | +6 | 6 |
| Cargo | 1 | 0 | 0 | +3 | +4 | 8 |
| Sensors | 1 | 0 | 0 | 0 | +5 | 6 |
| Specialized | 0 | 0 | +3 | +5 | 0 | 8 |
| Life Support | 0 | 0 | 0 | 0 | +8 | 8 |
| **TOTAL** | **10** | **+24** | **+28** | **+20** | **+30** | **~112** |

## Technical Considerations

### Performance
- Each module: < 100 faces recommended
- Total ship: Target < 10,000 faces for complex capitals
- LOD system for distant ships

### Memory
- Module definitions: ~2KB each
- 100 modules: ~200KB total (negligible)
- 3D models: ~50KB average per module
- 100 models: ~5MB total (acceptable)

### Rendering
- Batch rendering by material
- Instancing for repeated modules
- Frustum culling for off-screen modules

## Future Enhancements

### Advanced Features (Post-Launch)
1. **Module Upgrades:** Improved versions of modules
2. **Tech Tiers:** Higher tier modules with better stats
3. **Faction Variants:** Faction-specific module styles
4. **Animated Modules:** Rotating turrets, opening hangar doors
5. **Module Damage:** Visual damage states
6. **Custom Paint:** Player-applied colors and decals
7. **Module Crafting:** Build modules from resources
8. **Module Salvage:** Recover modules from destroyed ships

---

**Document Version:** 1.0  
**Date:** January 4, 2026  
**Status:** Design Document - Implementation in Progress  
**Author:** Copilot AI Agent
