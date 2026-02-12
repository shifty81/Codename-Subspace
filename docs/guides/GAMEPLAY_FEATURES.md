# Codename:Subspace - Gameplay Features Status

**Last Updated:** December 2024  
**Version:** v0.9.0+

---

## Overview

This document provides a comprehensive overview of the currently implemented gameplay features, focusing on the **NEW GAME** experience (Option 1).

---

## Core Gameplay Loop ✅

The game is **fully playable** with a complete gameplay loop:

### Starting a New Game

1. Run `dotnet run` from the `AvorionLike` directory
2. Select **Option 1: Start New Game**
3. **Choose your starting ship** from 12 procedurally generated options:
   - Fighters (small, agile)
   - Corvettes (versatile)
   - Frigates (larger, powerful)
4. Enter the 3D game world

---

## Ship Selection System ✅

### Features
- **12 starter ships** procedurally generated
- **Multiple roles**: Combat, Exploration, Mining, Trading, Multipurpose
- **Different hull shapes**: Angular, Sleek, Blocky, Cylindrical
- **Material tiers**: Iron, Titanium, Trinium, Naonite, Ogonite, Xanion, Avorion
- **Detailed statistics** for each ship (mass, thrust, shields, power, weapons, cargo)

### Ship Selection Options
- **Enter ship number** (1-12) to select
- **V** - View ships in 3D before choosing (camera controls: WASD + Mouse)
- **D** - View detailed stats for a specific ship
- **R** - Regenerate new ship options
- **0** - Cancel and return to main menu

---

## Player Controls ✅

### Movement Controls
- **C** - Toggle between Camera Mode and Ship Control Mode
- **WASD** - Forward/Back/Strafe left/right
- **Space** - Thrust up
- **Shift** - Thrust down
- **Arrow Keys + Q/E** - Ship rotation (when in Ship Control Mode)

### Mouse Controls ✅ **WORKING**
- **Mouse Look** - Free camera rotation (in Camera Mode)
- **ALT + Mouse** - Show cursor and interact with UI
- **Automatic cursor management**:
  - Cursor hidden during normal gameplay (Raw mode)
  - Cursor visible when menus/UI are open
  - Cursor visible when ALT is held

### UI Controls
- **ESC** - Open/Close pause menu
- **TAB** - Player Status screen
- **I** - Inventory screen
- **B** - Ship Builder screen
- **M** - Galaxy Map ✅ **FULLY IMPLEMENTED**
- **~** (Tilde) - In-game testing console
- **F1** - Toggle Ship Status HUD
- **F2** - Toggle Radar
- **F3** - Toggle Resource Panel

---

## Galaxy Map System ✅ **FULLY IMPLEMENTED**

### Opening the Map
Press **M** at any time during gameplay to open the interactive Galaxy Map.

### Map Features
- **2D Sector Grid** - Visual representation of galaxy sectors
- **Current Location** - Highlighted in bright blue
- **Tech Level Colors** - Sectors colored by distance from galaxy center:
  - Red (Core) - Avorion regions
  - Orange/Yellow - Ogonite/Xanion regions
  - Green - Trinium regions
  - Blue - Naonite regions
  - Dark Blue - Titanium regions
  - Gray (Outer) - Iron regions
- **Jump Range Circle** - Blue circle shows reachable sectors
- **Content Indicators**:
  - Green dot = Station present
  - Yellow dot = Rich asteroid field

### Map Controls
- **Mouse Scroll** - Zoom in/out (0.2x to 5x)
- **Left Click + Drag** - Pan around galaxy
- **+/-** Buttons - Change Z-slice (vertical layer)
- **Reset View** Button - Center on current location
- **Left Click on Sector** - Select and view sector info
- **Right Click on Sector** - Initiate hyperdrive jump (if in range)
- **Hover over Sector** - Quick preview

### Map Filters
- **Stations Checkbox** - Show/hide station indicators
- **Asteroids Checkbox** - Show/hide asteroid indicators  
- **Ships Checkbox** - Show/hide ship indicators
- **Jump Range Checkbox** - Show/hide jump range circle

---

## Sector Travel & Hyperdrive ✅ **FULLY IMPLEMENTED**

### Hyperdrive Jump System
1. Open Galaxy Map (M key)
2. Select a sector within jump range (highlighted by blue circle)
3. Right-click target sector OR click "Initiate Jump" button
4. Jump charging begins automatically
5. Ship teleports to new sector when charged

### Jump Status
- **Ready** - Green "WITHIN JUMP RANGE" - can initiate jump
- **Charging** - Yellow progress bar showing charge percentage
- **Cooldown** - Gray progress bar showing time until next jump
- **Out of Range** - Red "OUT OF JUMP RANGE" - need hyperdrive upgrade

### Jump Requirements
- Target sector must be within jump range
- Hyperdrive must not be on cooldown
- No active jump in progress

### Canceling Jumps
While charging, click "Cancel Jump" button in the info panel.

---

## Galaxy Progression System ✅

### Starting Location
- **Galaxy Rim** - 400 sectors from center
- **Iron Zone** - Basic materials available
- **Difficulty: 1.0x**

### Progression Mechanics
- **Distance from Center** - Determines available materials and difficulty
- **Material Tiers** - Unlock better materials by traveling toward center:
  1. **Iron** - Outer rim (400+ sectors)
  2. **Titanium** - < 350 sectors
  3. **Naonite** - < 250 sectors (Shields unlock)
  4. **Trinium** - < 200 sectors (Faster engines)
  5. **Xanion** - < 150 sectors (Better weapons)
  6. **Ogonite** - < 50 sectors (Captains, fleet automation)
  7. **Avorion** - < 25 sectors (Galactic core, most powerful)

### Zone Features
- **Tech Level** - Higher tier materials available closer to center
- **Difficulty Multiplier** - Increases as you approach center
- **Feature Unlocks** - New capabilities at each zone
- **Dynamic Content** - Asteroids, stations, enemies scale with zone

---

## Ship Building System ✅

### Build Mode
- Access via **B** key or pause menu
- Real-time voxel-based construction
- Place, rotate, and remove blocks
- 9 block types: Armor, Engine, Thruster, Generator, Shield, Gyro, Weapon, Cargo, Crew
- 7 material types with different properties

### Block Stretching
- Blocks can be resized for custom designs
- Stats scale based on block size
- Allows for elaborate ship designs

---

## Combat System ✅

### Weapons
- 6 weapon types available
- Mounted on weapon blocks
- Energy consumption based
- Auto-targeting system

### Defensive Systems
- **Shields** - Energy-based protection
- **Structural Integrity** - Hull damage system
- **Evasive Maneuvers** - Player-controlled dodging

### Enemy AI
- State-based behavior (Patrol, Combat, Fleeing)
- Multiple tactics (Aggressive, Kiting, Strafing, Broadsiding, Defensive)
- Dynamic threat assessment
- Retreat when heavily damaged

---

## Resource & Trading System ✅

### Resources
- 12 resource types (Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion, Silicon, Uranium, etc.)
- Mining from asteroids
- Trading at stations
- Crafting system for upgrades

### Inventory
- Capacity-based system
- Resource storage
- Credits (currency)
- Upgrade items

### Trading
- Buy/Sell at stations
- Dynamic pricing
- Supply/demand simulation

---

## Fleet & Crew Management ✅

### Fleet System
- Command multiple ships
- Assign captains (unlocked in Ogonite zone)
- Automated fleet behaviors
- Formation flying

### Crew System
- Hire crew at stations
- Crew affects ship performance
- Crew management UI
- Morale system

---

## Save/Load System ✅

### Persistence
- Save game state anytime
- Multiple save slots
- Quick save functionality
- Auto-save option (configurable)

### Save Data
- Player ship and inventory
- Galaxy state
- Progression data
- Fleet and crew

---

## In-Game Testing Console ✅

### Access
Press **~** (Tilde) key during gameplay

### 40+ Commands Available
- **Demo commands**: `demo_quick`, `demo_combat`, `demo_mining`, `demo_world`
- **Spawn commands**: `spawn_ship`, `spawn_enemy`, `spawn_asteroid`, `spawn_station`
- **Resource commands**: `credits [amount]`, `add_resource [type] [amount]`
- **Testing commands**: `tp [x y z]`, `velocity [x y z]`, `heal`, `damage [amount]`
- **System commands**: `fps`, `entities`, `clear`, `help`

See [IN_GAME_TESTING_GUIDE.md](IN_GAME_TESTING_GUIDE.md) for full command list.

---

## Mouse Interaction Status ✅ **VERIFIED WORKING**

### Implementation Details
The mouse system is **fully functional** with the following features:

1. **Automatic Cursor Management**
   - Cursor hidden (Raw mode) during normal gameplay for free-look camera
   - Cursor visible (Normal mode) when:
     - ESC menu is open
     - Galaxy Map (M) is open
     - ALT key is held down
     - Any UI window is active

2. **Menu Interaction** ✅
   - `HandleMouseMove()` - Tracks mouse position for menu highlighting
   - `HandleMouseClick()` - Processes left-click on menu items
   - Hover effects on menu buttons
   - Click-to-select functionality

3. **Galaxy Map Interaction** ✅
   - Mouse scroll for zoom
   - Left-click + drag to pan
   - Left-click to select sector
   - Right-click to initiate jump
   - Hover for sector preview

4. **ImGui UI** ✅
   - Full mouse support in all ImGui windows
   - `io.WantCaptureMouse` respected to prevent conflicts
   - Proper cursor mode switching

### Code Locations
- **GraphicsWindow.cs** (lines 703-742): Mouse event handlers
- **GraphicsWindow.cs** (lines 247-269): Cursor mode management
- **GameMenuSystem.cs** (lines 95-122): Menu mouse handling
- **GalaxyMapUI.cs**: Galaxy map mouse interaction

---

## What's Working vs. What Needs Attention

### ✅ Fully Working
- Player ship controls (6DOF movement)
- Mouse handling (free-look, UI interaction)
- Galaxy map and sector travel
- Hyperdrive jump system
- Ship selection and generation
- Combat, mining, trading
- Save/load system
- In-game testing console
- 3D graphics rendering
- ImGui UI integration

### ⚠️ Planned/In Development
- Quest/Mission system (not yet implemented)
- Tutorial system (no onboarding yet)
- Sound/Music (no audio engine)
- Multiplayer client UI (server works, client UI partial)
- Advanced rendering (shadows, post-processing)

---

## Quick Start Summary

1. **Launch Game**: `dotnet run` → Select Option 1
2. **Choose Ship**: Pick from 12 options (or press V to view in 3D first)
3. **Control Ship**: 
   - **C** to toggle Camera/Ship control
   - **WASD + Space/Shift** for movement
   - **Arrow Keys + Q/E** for rotation
4. **Explore Galaxy**:
   - **M** to open Galaxy Map
   - Right-click sectors to jump
   - Travel toward center (0,0,0) for better materials
5. **Build & Upgrade**:
   - **B** for Ship Builder
   - **I** for Inventory
   - Mine asteroids, trade at stations
6. **Combat**:
   - Find enemy ships
   - Use weapons (auto-targeting)
   - Manage shields and energy
7. **Test Features**:
   - **~** for Testing Console
   - Type `help` for command list

---

## Additional Documentation

- **[QUICK_STATUS.md](QUICK_STATUS.md)** - Super quick status overview
- **[PLAYABILITY_STATUS.md](PLAYABILITY_STATUS.md)** - Detailed playability assessment
- **[GALAXY_MAP_GUIDE.md](GALAXY_MAP_GUIDE.md)** - Complete galaxy map documentation
- **[IN_GAME_TESTING_GUIDE.md](IN_GAME_TESTING_GUIDE.md)** - Testing console guide
- **[CORE_GAMEPLAY_MECHANICS.md](CORE_GAMEPLAY_MECHANICS.md)** - Game design and mechanics
- **[HOW_TO_BUILD_AND_RUN.md](HOW_TO_BUILD_AND_RUN.md)** - Build and run instructions

---

## Conclusion

**Codename:Subspace is fully playable** with Option 1 (NEW GAME). All core features are implemented:

✅ **Mouse in menus** - Working perfectly  
✅ **Galaxy map** - Fully implemented with sector visualization  
✅ **Sector travel** - Hyperdrive jump system functional  
✅ **Player controls** - Complete 6DOF ship control  
✅ **Gameplay loop** - Mining, trading, combat, exploration, building

The game provides a complete sandbox space exploration experience. Start your journey at the galaxy rim and work your way to the core to unlock the most powerful materials and features!

---

**Ready to Play?** Run `dotnet run` and select Option 1!
