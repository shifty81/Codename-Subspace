# Custom Ship Designer & FPS Interior System - Design Document

## Overview

This document describes the custom ship designer system that allows skilled players to create their own ship classes by snapping modules together, with fully explorable interiors where modules have functional FPS interactions.

## Core Concepts

### 1. Ship Building Progression
Players unlock ship design capabilities through experience:

**Skill Levels:**
- **Novice (Level 0-2):** Can only use pre-designed ships
- **Apprentice (Level 3-5):** Can modify existing ship designs (swap modules)
- **Journeyman (Level 6-8):** Can create custom ships using templates
- **Expert (Level 9-12):** Full custom ship designer access
- **Master (Level 13+):** Advanced features (custom module positions, complex shapes)

### 2. Module Snapping System
**Grid-Based Assembly:**
- Modules snap to a 3D grid (1 unit = 1 meter)
- Each module has defined connection points
- Modules automatically align and connect
- Structural integrity validation ensures ship is viable

**Connection Types:**
- **Structural:** Hull connections, must be continuous from cockpit
- **Power:** Power lines automatically route through connected modules
- **Data:** Computer network for ship systems
- **Life Support:** Atmosphere and gravity systems
- **Fuel/Resource:** Pipes for fuel, ore, cargo transfer

### 3. Interior Generation
**Automatic Interior Creation:**
- Each module contains pre-defined interior space
- Corridors automatically generated between connected modules
- Airlocks at exterior access points
- Interior matches module function and size

**Interior Elements:**
- **Walls/Floors/Ceilings:** Based on module style (military, industrial, sleek)
- **Doors:** Between modules, with access controls
- **Lighting:** Functional lights, emergency lighting
- **Equipment:** Interactive consoles, machines, terminals
- **Details:** Pipes, cables, panels, storage lockers

## Custom Ship Designer Interface

### Main Components

#### 1. 3D Workspace
```
┌─────────────────────────────────────────┐
│  3D View - Ship Assembly Area           │
│                                          │
│    ┌──┐                                 │
│    │  │  ← Cockpit (required)           │
│    └──┘                                 │
│     ║                                    │
│    ┌──┐  ← Hull Section                 │
│    │  │                                  │
│    └──┘                                 │
│   ╱    ╲ ← Wings                        │
│  ╱      ╲                                │
│ ┌────────┐ ← Engine                     │
│                                          │
│  [Rotate] [Pan] [Zoom] [Toggle Interior]│
└─────────────────────────────────────────┘
```

#### 2. Module Palette
```
┌──────────────────────┐
│ Module Library       │
├──────────────────────┤
│ [Filter: All ▾]      │
│ [Search: _______]    │
├──────────────────────┤
│ ▣ Cockpits (5)       │
│ ▣ Hull Sections (12) │
│ ▣ Engines (8)        │
│ ▣ Weapons (15)       │
│ ▣ Power (6)          │
│ ▣ Cargo (5)          │
│ ▣ Utility (20)       │
└──────────────────────┘
```

#### 3. Ship Stats Panel
```
┌──────────────────────┐
│ Ship Statistics      │
├──────────────────────┤
│ Name: [My Ship____]  │
│ Class: Custom        │
│ Size: 350m³          │
│ Mass: 1,250 tons     │
│                      │
│ ⚡ Power: 850/1000   │
│ 🛡️  Shield: 5,000    │
│ 🎯 Weapons: 8 slots  │
│ 📦 Cargo: 500 units  │
│ 👥 Crew: 45/50       │
│                      │
│ ✓ Structurally Sound │
│ ✓ Power Sufficient   │
│ ✓ Life Support OK    │
│ ⚠ No Hyperdrive      │
└──────────────────────┘
```

#### 4. Interior View Toggle
```
[External View] ←→ [Interior View] ←→ [Cutaway View]
```

## Module Interior Design

### Module Interior Components

Each module contains:
1. **Exterior Model** - The visible outside
2. **Interior Space** - Walkable area inside
3. **Interactive Elements** - Equipment, consoles, stations
4. **Connection Points** - Doorways to other modules
5. **Collision Mesh** - For FPS movement
6. **Navigation Mesh** - For NPC pathfinding

### Interior Templates by Module Type

#### Cockpit/Bridge Interior
```
        Door
         ↓
    ┌─────────┐
    │  Screen │  ← Main viewscreen
    └─────────┘
    [●] [●]      ← Pilot seats
    │││││││      ← Control consoles
    ┌───────┐
    │ Floor │
    └───────┘
```

**Interactive Elements:**
- Pilot seat (sit, view controls)
- Navigation console (set destinations)
- Weapons console (target enemies)
- Systems console (manage ship)

#### Engine Room Interior
```
       Door
        ↓
    ╔═══════╗
    ║ ═══ ║  ← Reactor core (glow)
    ║  ┼  ║
    ╚═══════╝
    
    [⚙️] [⚙️]  ← Control panels
    ││   ││   ← Power conduits
```

**Interactive Elements:**
- Reactor control panel (adjust power output)
- Emergency shutdown (safety)
- Diagnostic terminal (view status)
- Maintenance access (repairs)

#### Cargo Bay Interior
```
       Door
        ↓
    ┌────┬────┐
    │▢▢▢│▢▢▢│ ← Storage containers
    │▢▢▢│▢▢▢│
    ├────┴────┤
    │   ▬▬▬  │ ← Loading ramp
    └─────────┘
```

**Interactive Elements:**
- Cargo manifest terminal (inventory)
- Storage containers (access items)
- Loading controls (transfer cargo)
- Security panel (lock/unlock)

#### Weapon Mount Interior
```
       Door
        ↓
    ╔════╗
    ║ ╬╬ ║  ← Turret mechanism
    ║ ║║ ║
    ╚════╝
    
    [◉]      ← Gunner station
    [📊]     ← Targeting computer
```

**Interactive Elements:**
- Gunner seat (manual control)
- Targeting computer (auto-target settings)
- Ammo loader (reload, swap ammo types)
- Maintenance panel (repairs, upgrades)

#### Power Core Interior
```
       Door
        ↓
    ╔═══════╗
    ║ ◉◉◉◉ ║  ← Power core
    ║ ◉◉◉◉ ║     (glowing)
    ╚═══════╝
    
    [░░░░░]  ← Status displays
    [⚡⚡⚡]   ← Power routing
```

**Interactive Elements:**
- Power distribution panel (route power)
- Core status monitor (health, output)
- Emergency power controls (battery backup)
- Overload controls (temporary boost)

#### Refinery/Processing Module Interior
```
       Door
        ↓
    ┌──────────┐
    │ ╔══════╗ │ ← Processing tank
    │ ║      ║ │
    │ ╚══════╝ │
    │          │
    │ [▓▓] [▓▓]│ ← Input/Output
    │ [▓▓] [▓▓]│    conveyors
    └──────────┘
```

**Interactive Elements:**
- Refinery control panel (start/stop processing)
- Input hopper (load raw ore)
- Output collector (collect refined materials)
- Recipe selector (choose what to produce)
- Efficiency monitor (view yields)

#### Medical Bay Interior
```
       Door
        ↓
    ┌──────────┐
    │  [▬▬▬]   │ ← Medical bed
    │           │
    │ ◇ ◇ ◇    │ ← Medical supplies
    │           │
    │ [▣]      │ ← Medical console
    └──────────┘
```

**Interactive Elements:**
- Medical console (diagnose, treat)
- Surgery table (heal critical injuries)
- Medicine cabinet (access med supplies)
- Clone bay (respawn point if available)

#### Crew Quarters Interior
```
       Door
        ↓
    ┌─────┬─────┐
    │[▭▭] │[▭▭] │ ← Bunks
    │     │     │
    │[◉]  │[◉]  │ ← Personal lockers
    └─────┴─────┘
```

**Interactive Elements:**
- Bunks (rest, save game)
- Lockers (personal storage)
- Crew terminal (check assignments)
- Recreation (morale, crew happiness)

## FPS Interaction System

### Interaction Types

#### 1. Direct Interactions
**Press [E] to interact with:**
- Consoles (open interface)
- Doors (open/close)
- Seats (sit/stand)
- Terminals (view data)
- Buttons/Switches (activate)
- Storage (access inventory)

#### 2. Menu-Based Interactions
**Craft/Process/Manage:**
```
╔═══════════════════════════════════╗
║  Refinery Control Panel           ║
╠═══════════════════════════════════╣
║  Input: Iron Ore (500 units)      ║
║  Output: Iron Ingots (450 units)  ║
║                                   ║
║  [Start Processing]               ║
║  [Stop]                           ║
║                                   ║
║  Efficiency: 90%                  ║
║  Time Remaining: 2:35             ║
╚═══════════════════════════════════╝
```

#### 3. Skill-Based Interactions
**Different skills unlock different options:**
- **Engineering Skill:** Repair/upgrade equipment
- **Science Skill:** Optimize processes, unlock recipes
- **Medical Skill:** Better healing, advanced treatments
- **Piloting Skill:** Advanced navigation, better handling

### Interactive Features by Module

| Module Type | Primary Interaction | Secondary Interaction | Skill Required |
|-------------|--------------------|-----------------------|----------------|
| Cockpit | Pilot controls | Navigation | Piloting |
| Engine | Power adjustment | Repairs | Engineering |
| Weapons | Fire control | Load ammo | Gunnery |
| Power Core | Power routing | Overload boost | Engineering |
| Cargo | Inventory access | Organization | None |
| Refinery | Process ore | Recipe selection | Science |
| Medical Bay | Healing | Surgery | Medical |
| Crew Quarters | Rest/save | Crew management | Leadership |
| Science Lab | Research | Scanning | Science |
| Hangar | Launch fighters | Repairs | Piloting/Eng |

## Module Connection & Interior Flow

### Corridor Generation

**Automatic Corridor Rules:**
1. Corridors connect adjacent modules
2. Minimum width: 2 meters
3. Includes doorways (slide open/close)
4. Lighting along ceiling
5. Access panels for maintenance

**Corridor Types:**
```
Straight:     L-Corner:      T-Junction:    Cross:
────────      ┐              ┬              ┼
              │              │
              └              
```

### Airlock Generation

**External Access Points:**
- Automatically placed at exterior module connections
- Double-door system (pressure seal)
- Emergency controls
- Suit storage lockers

```
Airlock Layout:
┌────────┐
│ Outer  │ ← Outer door (to space)
│  Door  │
├────────┤
│ ◇  ◇  │ ← Suit lockers
│        │
│ [▣]   │ ← Control panel
├────────┤
│ Inner  │ ← Inner door (to ship)
│  Door  │
└────────┘
```

## Ship Designer Workflow

### Step-by-Step Process

#### Step 1: Start New Design
```
1. Enter Ship Designer
2. Select base template (optional)
   - Blank (start from scratch)
   - Fighter template
   - Freighter template
   - Custom saved design
3. Name your ship
4. Set design goals (combat/cargo/exploration)
```

#### Step 2: Place Core Module (Required)
```
1. Select cockpit/bridge module
2. Place in center of workspace
3. This becomes ship center and required module
```

#### Step 3: Add Hull Sections
```
1. Select hull modules from palette
2. Click to place, drag to position
3. Modules snap to grid and connection points
4. Green outline = valid connection
5. Red outline = invalid (no connection/collision)
```

#### Step 4: Add Propulsion
```
1. Add engine modules to rear
2. Add thrusters for maneuvering
3. System calculates thrust/mass ratio
4. Warns if insufficient for ship size
```

#### Step 5: Add Systems
```
1. Add power core (required)
2. Add weapons (optional)
3. Add cargo (optional)
4. Add utility modules (shields, sensors, etc.)
```

#### Step 6: Validate Design
```
System checks:
✓ Structural integrity (all modules connected)
✓ Power sufficient (cores provide enough power)
✓ Life support (crew quarters for crew)
✓ Propulsion (can move)

Warnings (non-critical):
⚠ No hyperdrive (can't jump between systems)
⚠ Low cargo capacity
⚠ Weak shields
```

#### Step 7: Preview Interior
```
1. Click "Interior View"
2. System generates interior automatically
3. Walk through as FPS character
4. Verify all areas accessible
5. Check equipment placement
```

#### Step 8: Save & Build
```
1. Save design to blueprint
2. Calculate build cost (resources needed)
3. Build at shipyard/constructor
4. Time to construct based on complexity
```

## Technical Implementation

### Data Structures

#### CustomShipDesign Class
```csharp
public class CustomShipDesign
{
    public Guid Id { get; set; }
    public string Name { get; set; }
    public string Description { get; set; }
    public string DesignerName { get; set; }  // Player who designed it
    public DateTime CreatedDate { get; set; }
    
    // Module placement
    public List<PlacedModule> Modules { get; set; } = new();
    
    // Calculated stats
    public ShipStats Stats { get; set; }
    public bool IsValid { get; set; }
    public List<string> ValidationErrors { get; set; } = new();
    
    // Interior data
    public InteriorLayout Interior { get; set; }
    
    // Build requirements
    public Dictionary<string, int> RequiredResources { get; set; }
    public float BuildTime { get; set; }  // in hours
    
    // Classification (generated from modules)
    public ShipClass SuggestedClass { get; set; }
    public ShipSize Size { get; set; }
}
```

#### PlacedModule Class
```csharp
public class PlacedModule
{
    public Guid InstanceId { get; set; }
    public string ModuleDefinitionId { get; set; }  // Reference to ModuleLibrary
    public Vector3 Position { get; set; }            // Grid position
    public Quaternion Rotation { get; set; }         // Orientation
    public string Material { get; set; }             // Iron, Titanium, etc.
    
    // Connections to other modules
    public List<ModuleConnection> Connections { get; set; } = new();
    
    // Interior customization (optional)
    public InteriorCustomization InteriorOverride { get; set; }
}
```

#### InteriorLayout Class
```csharp
public class InteriorLayout
{
    public List<InteriorRoom> Rooms { get; set; } = new();
    public List<Corridor> Corridors { get; set; } = new();
    public List<Door> Doors { get; set; } = new();
    public List<InteractiveElement> InteractiveElements { get; set; } = new();
    
    // Navigation for AI
    public NavigationMesh NavMesh { get; set; }
}
```

#### InteriorRoom Class
```csharp
public class InteriorRoom
{
    public Guid RoomId { get; set; }
    public Guid ParentModuleId { get; set; }  // Which module contains this room
    public Vector3 Center { get; set; }
    public Vector3 Size { get; set; }
    public RoomType Type { get; set; }  // Cockpit, Engine, Cargo, etc.
    
    // Visual elements
    public List<WallSegment> Walls { get; set; } = new();
    public List<FloorPanel> Floor { get; set; } = new();
    public List<CeilingPanel> Ceiling { get; set; } = new();
    public List<Prop> Props { get; set; } = new();  // Furniture, equipment
    
    // Functional elements
    public List<InteractiveElement> InteractiveElements { get; set; } = new();
    public LightingSetup Lighting { get; set; }
}
```

#### InteractiveElement Class
```csharp
public class InteractiveElement
{
    public Guid ElementId { get; set; }
    public string Name { get; set; }
    public InteractionType Type { get; set; }
    public Vector3 Position { get; set; }
    public Quaternion Rotation { get; set; }
    
    // What happens when player interacts
    public Action<Player> OnInteract { get; set; }
    
    // UI to show when interacted
    public string InterfaceId { get; set; }  // References a UI panel
    
    // Requirements
    public string RequiredSkill { get; set; }
    public int RequiredSkillLevel { get; set; }
    
    // Visual
    public string ModelPath { get; set; }
    public bool IsHighlightedWhenNear { get; set; } = true;
}
```

### Module Interior Definition

Each module definition includes interior data:

```csharp
public class ShipModuleDefinition
{
    // ... existing properties ...
    
    /// Interior template for this module
    public ModuleInteriorTemplate InteriorTemplate { get; set; }
}

public class ModuleInteriorTemplate
{
    // Basic room dimensions (relative to module size)
    public Vector3 RoomSize { get; set; }
    public float CeilingHeight { get; set; } = 3.0f;  // meters
    
    // Doorway positions (where corridors connect)
    public List<DoorwayDefinition> Doorways { get; set; } = new();
    
    // Interactive elements to spawn
    public List<InteractiveElementDefinition> InteractiveElements { get; set; } = new();
    
    // Props and decorations
    public List<PropDefinition> Props { get; set; } = new();
    
    // Lighting
    public LightingDefinition Lighting { get; set; }
    
    // Style (affects wall/floor textures)
    public InteriorStyle Style { get; set; } = InteriorStyle.Military;
}
```

## UI/UX Features

### Quality of Life Features

1. **Symmetry Mode:** Mirror modules on opposite sides
2. **Copy/Paste:** Duplicate module arrangements
3. **Templates:** Save common module groups
4. **Undo/Redo:** Full history
5. **Snap Settings:** Adjust grid size, rotation increments
6. **Camera Presets:** Front, side, top, isometric views
7. **Module Search:** Filter by name, category, stats
8. **Comparison Mode:** Compare module stats side-by-side
9. **Cost Calculator:** Real-time resource requirements
10. **Performance Estimator:** Predict ship performance

### Interior Editor (Advanced)

**For Master-level builders:**
- Custom prop placement
- Lighting adjustments
- Texture/color customization
- Custom interactive element positioning
- Room divider placement
- Furniture arrangement

## Progression & Unlocks

### Ship Building Skill Tree

```
Level 1: Basic Ship Modification
  └─ Can swap modules on existing ships
  
Level 3: Template Designer
  └─ Can use ship templates
  └─ Can modify module positions slightly
  
Level 6: Custom Builder
  └─ Full ship designer access
  └─ Can create ships from scratch
  └─ Module library: Basic modules only
  
Level 9: Expert Designer
  └─ Access to advanced modules
  └─ Can create complex shapes
  └─ Interior preview
  
Level 12: Master Architect
  └─ Access to all modules
  └─ Custom module positioning
  └─ Interior editor
  └─ Can design module interiors
  └─ Save designs as blueprints (shareable)
  
Level 15: Legendary Shipwright
  └─ Can design custom modules (model import)
  └─ Advanced interior scripting
  └─ Can sell designs to other players
```

### Skill Progression

**Gain XP from:**
- Building ships (+100 XP per ship)
- Modifying ships (+50 XP per modification)
- Flying custom-designed ships (+10 XP per hour)
- Selling ship designs (+500 XP per sale)
- Other players using your designs (+50 XP per use)

## Example: Complete Custom Ship Creation

### "The Prospector" - Custom Mining Ship

**Designer Intent:**
- Small crew (5 people)
- Focus on mining and ore processing
- Moderate cargo capacity
- Light defenses
- Efficient design for solo/small group play

**Module Layout:**
```
        [Cockpit]
            ║
        [Hull 1]
       ╱   ║   ╲
   [Wing] [Hull 2] [Wing]
            ║
     [Refinery Module]
            ║
        [Cargo Bay]
       ╱         ╲
  [Cargo Bay] [Cargo Bay]
            ║
        [Engine]
       ╱      ╲
  [Thruster] [Thruster]
```

**Modules Used:**
- 1x Small Cockpit (2 crew)
- 2x Hull Section
- 2x Small Wings (mining laser mounts)
- 1x Refinery Module (ore processing)
- 3x Medium Cargo Bay
- 1x Medium Engine
- 2x Small Thruster
- 1x Small Power Core
- 1x Small Shield Generator
- 2x Mining Laser Array

**Interior Layout:**
```
Deck 1 (Top):
- Cockpit (pilot, co-pilot seats)
  - Navigation console
  - Mining control console
  - Ship systems

Deck 2 (Middle):
- Refinery Control Room
  - Processing controls
  - Input hopper
  - Output collector
  - Recipe selector
  
- Corridor to cargo access

Deck 3 (Lower):
- Cargo Bay Access
  - Cargo manifest terminal
  - 3 large cargo holds
  - Loading controls
  
- Engine Room
  - Reactor controls
  - Emergency systems

Connecting Corridors:
- Vertical ladder between decks
- Horizontal corridors
- 2 external airlocks (port, starboard)
```

**FPS Gameplay Loop:**
1. Pilot flies to asteroid field
2. Uses mining lasers (from cockpit) to break asteroids
3. Collect ore automatically (tractor beam)
4. Walk to refinery room
5. Load ore into input hopper
6. Select refining recipe
7. Start processing
8. Wait for completion (or continue mining)
9. Collect refined materials
10. Walk to cargo bay
11. Organize inventory
12. Return to station to sell

**Build Cost:**
- Iron: 5,000 units
- Titanium: 2,000 units
- Credits: 150,000
- Build Time: 4 hours (game time)

## Benefits of System

### For Players
- ✓ True customization and creativity
- ✓ Ships that match playstyle perfectly
- ✓ Immersive interior exploration
- ✓ Functional equipment interactions
- ✓ Skill-based progression
- ✓ Community sharing (blueprints)

### For Gameplay
- ✓ Deepens ship investment (emotional attachment)
- ✓ Provides end-game content (perfecting designs)
- ✓ Encourages experimentation
- ✓ Supports multiple playstyles
- ✓ Creates economy (blueprint trading)

### For Development
- ✓ Player-generated content extends game life
- ✓ Modular system easy to expand
- ✓ Clear structure for adding features
- ✓ Performance-friendly (instanced interiors)

---

**Document Version:** 1.0  
**Date:** January 4, 2026  
**Status:** Design Document  
**Author:** Copilot AI Agent
