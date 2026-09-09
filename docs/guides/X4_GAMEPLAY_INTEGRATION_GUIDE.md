# X4-Style Gameplay Implementation Guide

## Overview

This guide documents the X4: Foundations-inspired gameplay systems implemented in Codename:Subspace, including ship classes, map systems, controls, and FPS mechanics.

## Table of Contents

1. [Ship System](#ship-system)
2. [Station System](#station-system)
3. [Map and Navigation](#map-and-navigation)
4. [Controls](#controls)
5. [FPS/Interior System](#fps-interior-system)
6. [Gameplay Loop](#gameplay-loop)

---

## Ship System

### X4-Inspired Ship Classes

The game features a comprehensive ship classification system based on X4: Foundations:

#### Small (S) Class
- **Fighter_Light**: Fast interceptor (Scout equivalent)
- **Fighter_Heavy**: Combat fighter
- **Miner_Small**: Small mining vessel

#### Medium (M) Class
- **Corvette**: Light warship (Ulysses starter ship)
- **Frigate**: Medium warship
- **Gunboat**: Heavy gunboat
- **Miner_Medium**: Optimal mining ship
- **Freighter_Medium**: Medium cargo hauler

#### Large (L) Class
- **Destroyer**: Heavy combat ship
- **Freighter_Large**: Large cargo vessel
- **Miner_Large**: Heavy mining platform

#### Extra Large (XL) Class
- **Battleship**: Capital warship
- **Carrier**: Fleet carrier
- **Builder**: Construction ship

### Ulysses Starter Ship

The **Ulysses** is the default starter ship - a reliable Corvette-class vessel:

**Base Statistics:**
- Hull: 1500 HP
- Mass: 500 tons
- Speed: 80 m/s
- Thrust: 5000 N
- Cargo: 50 units
- Power: 500 W

**Default Equipment:**
- 2x Pulse Lasers (Primary Weapons)
- 1x Mining Laser (Utility)

**Interior Layout:**
1. **Cockpit/Bridge**: Forward command center with terminals and crew stations
2. **Crew Quarters**: Living space with beds and lockers
3. **Cargo Bay**: Storage area with containers and crates
4. **Engine Room**: Power systems and maintenance access

### Ship Interiors

All ships include procedurally generated interiors based on their class:

**Fighter Cockpit** (S-Class):
- Single pilot seat
- Compact control panel
- Emergency systems access

**Corvette Bridge** (M-Class):
- Captain's chair
- 2-3 crew stations
- Tactical display

**Capital Bridge** (XL-Class):
- Commander's deck
- Multiple crew stations
- Holographic displays
- War room

---

## Station System

### Modular Station Components

Stations are built from modular components similar to ships:

#### Hub/Command Modules
- **Basic Command Hub**: Central control (50 crew)
- **Advanced Command Center**: Enhanced systems (100 crew, +10 research)

#### Docking Modules
- **Small Docking Bay**: 2 docking ports
- **Large Hangar Bay**: 5 docking ports

#### Production Modules
- **Basic Factory**: 100 production capacity
- **Ore Refinery**: Processes raw materials (200 production)

#### Storage Modules
- **Cargo Storage**: 2000 capacity
- **Large Warehouse**: 10000 capacity

#### Defense Modules
- **Defense Turret**: Automated weapons
- **Shield Generator**: 5000 shield capacity

#### Utility Modules
- **Power Generator**: 500W generation
- **Sensor Array**: Long-range detection

#### Other Modules
- **Crew Quarters**: 100 crew capacity
- **Trading Market**: 1000 trading capacity
- **Research Laboratory**: 50 research points

### Station Types

Stations can be generated in different configurations:

1. **Trading Post**: Market + docking + storage
2. **Shipyard**: Docking + factories + storage
3. **Factory**: Production + refineries + power
4. **Mining Station**: Refineries + large storage
5. **Research Station**: Labs + habitat + sensors
6. **Defense Platform**: Turrets + shields + power
7. **Refueling Depot**: Docking + storage + power
8. **Command Center**: Advanced hub + sensors + defense
9. **Habitat**: Crew quarters + life support

---

## Map and Navigation

### X4-Style Galaxy Map

The galaxy map system provides X4-inspired navigation:

**Features:**
- Sector-based navigation (3D coordinate system)
- Material tier zones (Iron → Titanium → Naonite → Trinium → Xanion → Ogonite → Avorion)
- Distance-based difficulty scaling
- Hyperdrive travel between sectors

**Zone System:**
- **Galaxy Rim** (400+ sectors): Iron tier, beginner area
- **Outer Rim** (350-400): Titanium unlocked
- **Mid Sectors** (250-350): Naonite tier, shields available
- **Inner Core** (50-250): Trinium/Xanion, advanced systems
- **Deep Core** (25-50): Ogonite, fleet captains
- **Galactic Center** (0-25): Avorion tier, endgame

### Navigation Controls

- **M Key**: Open/close galaxy map
- **Mouse**: Pan and zoom
- **Right-click**: Select sector for jump
- **Arrow Keys**: Navigate map layers

### Fast Travel

**Hyperdrive System:**
- Jump between distant sectors
- Energy cost based on distance
- Cooldown between jumps
- Requires hyperdrive module

**Highway System** (Planned):
- High-speed travel lanes
- Reduced jump costs
- Trade route networks
- Faction-controlled gates

---

## Controls

### X4-Inspired Control Scheme

**Flight Controls:**
- **W/S**: Forward/Backward thrust
- **A/D**: Strafe left/right
- **Space/Shift**: Thrust up/down
- **Arrow Keys**: Pitch/Yaw
- **Q/E**: Roll
- **X**: Emergency brake

**Combat Controls:**
- **Left Mouse**: Fire primary weapons
- **Right Mouse**: Fire secondary/missiles
- **Tab**: Cycle targets
- **R**: Toggle auto-targeting

**Navigation:**
- **M**: Galaxy map
- **N**: Local sector map
- **H**: Hyperdrive engage

**UI:**
- **I**: Inventory
- **B**: Ship builder
- **F**: Fleet management
- **Esc**: Pause menu

**FPS Mode:**
- **WASD**: Movement
- **Mouse**: Look around
- **E**: Interact
- **F**: Enter/exit ship
- **C**: Crouch
- **Shift**: Run

### Flight Assist

**Toggle Mode** (Planned):
- **Flight Assist ON**: Automatic velocity dampening, easier control
- **Flight Assist OFF**: Full Newtonian physics, drift enabled

---

## FPS/Interior System

### First-Person Mode

Players can walk around ship interiors in first-person view:

**Movement:**
- Full 6DOF in zero-G environments
- Walking in gravity-enabled areas
- Smooth transitions between modes

**Interactions:**
- Terminals: Access ship systems
- Storage: Manage cargo
- Workbenches: Craft items
- Beds: Save/rest
- Doors: Navigate between rooms

### Ship Interior Features

**Cockpit View:**
- First-person pilot perspective
- HUD overlay with ship status
- Window view of space
- Interactive controls

**Interior Exploration:**
- Walk through ship corridors
- Visit different rooms
- Manage crew stations
- Inspect modules

### View Transitions

**Seamless Switching:**
1. **External View**: 3rd person ship flying
2. **Cockpit View**: First-person piloting
3. **Interior View**: Walking inside ship
4. **Station View**: Docking and exploring stations

**Planned Features:**
- Press **F** to exit cockpit and walk around
- Press **C** to return to pilot seat
- Auto-transition when docking
- Teleport to key locations

---

## Gameplay Loop

### Core X4-Style Gameplay

**Exploration:**
1. Start in Galaxy Rim with Ulysses
2. Scan sectors for resources
3. Discover stations and factions
4. Map trade routes

**Trading:**
1. Buy low, sell high between stations
2. Establish trade routes
3. Upgrade cargo capacity
4. Hire trade captains

**Combat:**
1. Engage pirates and enemies
2. Protect trade convoys
3. Capture sectors
4. Build fleet

**Mining:**
1. Mine asteroids for resources
2. Refine ore at stations
3. Sell refined materials
4. Upgrade mining lasers

**Building:**
1. Construct stations
2. Establish production chains
3. Create trade empire
4. Build capital ships

### Progression Path

**Early Game** (Iron Zone):
1. Complete starter missions
2. Mine asteroids
3. Trade basic goods
4. Upgrade Ulysses

**Mid Game** (Naonite/Trinium):
1. Build first station
2. Establish production
3. Purchase additional ships
4. Hire captains

**Late Game** (Xanion/Ogonite):
1. Command fleet
2. Control sectors
3. Build capital ships
4. Dominate economy

**Endgame** (Avorion):
1. Reach galactic center
2. Build superstructures
3. Command armada
4. Shape galaxy

---

## Implementation Status

### Completed ✅
- [x] X4 ship class system
- [x] Ulysses starter ship with interior
- [x] Modular station system
- [x] Station procedural generation
- [x] Galaxy map with zone system
- [x] Basic flight controls
- [x] Interior object system
- [x] Ship equipment system

### In Progress 🚧
- [ ] Cockpit view rendering
- [ ] FPS interior movement
- [ ] View transition system
- [ ] X4-style HUD
- [ ] Highway system
- [ ] Fleet automation
- [ ] Advanced AI behavior

### Planned 📋
- [ ] Station construction UI
- [ ] Complex production chains
- [ ] Faction warfare
- [ ] Diplomatic system
- [ ] Mission system
- [ ] Crew management
- [ ] Ship customization UI

---

## Developer Notes

### Adding New Ship Classes

```csharp
// Define in X4ShipClasses.cs
var newShip = new X4ShipConfig
{
    ShipClass = X4ShipClass.YourClass,
    DesignStyle = X4DesignStyle.Advanced,
    ShipName = "Your Ship Name"
};

var generator = new X4ShipGenerator(moduleLibrary);
var ship = generator.GenerateX4Ship(newShip);
```

### Creating Custom Stations

```csharp
var stationLibrary = new StationModuleLibrary();
stationLibrary.InitializeBuiltInModules();

var generator = new ModularProceduralStationGenerator(stationLibrary);
var station = generator.GenerateStation(
    StationType.TradingPost,
    materialType: "Iron",
    complexity: 3
);
```

### Adding Interior Objects

```csharp
public static InteriorObject CreateCustomObject()
{
    return new InteriorObject
    {
        Type = InteriorObjectType.Terminal,
        Name = "Custom Terminal",
        Size = new Vector3(1, 1, 1),
        IsInteractable = true,
        InteractionPrompt = "Press E to use"
    };
}
```

---

## References

- [X4: Foundations](https://www.egosoft.com/games/x4/info_en.php)
- [X4 Wiki](https://www.x4-game.com/)
- [Modular Ship System Guide](MODULAR_SHIP_SYSTEM_GUIDE.md)
- [X4 Ship System Guide](X4_SHIP_SYSTEM_GUIDE.md)
- [Galaxy Map Guide](GALAXY_MAP_GUIDE.md)

---

**Last Updated**: January 2026  
**Version**: X4 Integration v1.0  
**Status**: Active Development
