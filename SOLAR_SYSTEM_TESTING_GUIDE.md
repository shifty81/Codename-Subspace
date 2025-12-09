# Solar System Testing Guide

This guide explains the two main gameplay options available for testing all implemented features in Codename: Subspace.

## Menu Options

### Option 1: Start New Game - Full Gameplay Experience
Complete gameplay experience with ship selection and comprehensive test environment.

**Features:**
- Interactive ship selection from 12 procedurally generated options
- Visual 3D preview of ships before selection
- Full galaxy progression system (starts at Galaxy Rim)
- Zone-appropriate content generation
- 18+ diverse test ships around you with **varied designs each playthrough**
- 4 space stations (Trading, Military, Industrial, Research)
- AI ships, asteroids, and full game systems

**Ship Variety (NEW):**
- Ships now generate with different designs each time you play
- Explicit hull shapes: Angular, Sleek, Blocky, Cylindrical, Irregular
- Multiple materials from Iron to Avorion
- All ship sizes from Fighter to Carrier
- Different roles: Combat, Trading, Mining, Exploration, Salvage

**What's Different Each Time:**
- Ship internal layouts vary
- Component placements differ
- Block arrangements change
- Visual appearance evolves
- Same ship type won't look identical between playthroughs

### Option 2: Experience Full Solar System - Complete Test Environment
Comprehensive showcase of ALL implemented features in a single solar system.

**Features:**
- Pre-configured player ship (Cruiser-class, Level 10, 1M credits)
- Starts in Naonite Zone (mid-galaxy) for variety
- All resources available in inventory
- Full capabilities enabled (hyperdrive, weapons, cargo)

**Content Generated:**
- **18 Ships**: Diverse fleet from Fighters to Capital ships
  - 3 Fighters at close range
  - 4 Corvettes at medium distance  
  - 5 Frigates further out
  - 2 Destroyers distant
  - 2 Cruisers very distant
  - 2 Capital ships (Battleship, Carrier) extremely distant

- **75+ Asteroids**: 7 distinct fields by material type
  - Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion
  - Properly colored and positioned

- **4 Space Stations**: Different types and architectures
  - Trading Station (Titanium, Modular)
  - Military Station (Ogonite, Modular)
  - Industrial Station (Iron, Sprawling)
  - Research Station (Avorion, Modular)

- **7 AI Ships**: With proper behaviors
  - 2 Traders near trading station
  - 2 Miners near asteroid fields
  - 2 Pirates in distant areas
  - 1 Explorer ship

## Testing Features

### Console Commands (Press ~)
Available in both options for testing:
- `help` - List all available commands
- `demo_quick` - Quick demo of features
- `demo_combat` - Combat demonstration
- `demo_mining` - Mining demonstration
- `demo_world` - World generation demo
- `spawn_ship` - Spawn additional ships
- `spawn_enemy` - Spawn enemy ships
- `spawn_asteroid` - Spawn asteroids
- `credits [amount]` - Add credits
- `add_resource [type] [amount]` - Add resources
- `tp [x y z]` - Teleport to coordinates
- `velocity [x y z]` - Set velocity
- `heal` - Heal ship
- `damage [amount]` - Damage ship

### UI Controls
- **M** - Open Galaxy Map
- **I** - Open Inventory
- **B** - Open Ship Builder
- **TAB** - Player Status Panel
- **ESC** - Pause Menu
- **~** - Testing Console
- **F1-F4** - Toggle debug panels

### Camera Controls
- **C** - Toggle between camera/ship control mode
- **WASD** - Movement/Thrust
- **Space/Shift** - Up/Down
- **Arrow Keys** - Pitch/Yaw
- **Q/E** - Roll
- **X** - Emergency Brake
- **Mouse** - Free Look (in camera mode)

## Systems Demonstrated

Both options showcase:
- ✅ Procedural Ship Generation (with variety!)
- ✅ Procedural Station Generation
- ✅ Procedural Asteroid Generation
- ✅ Galaxy Progression System
- ✅ Fleet Automation System
- ✅ Material Tier System (Iron → Avorion)
- ✅ Combat System
- ✅ Inventory & Resources
- ✅ Hyperdrive Navigation
- ✅ AI Behaviors (Traders, Miners, Pirates)
- ✅ Physics System (Newtonian)
- ✅ Power & Shield Systems
- ✅ Progression & Leveling

## Differences Between Options

| Feature | Option 1: Start New Game | Option 2: Full Solar System |
|---------|--------------------------|----------------------------|
| Ship Selection | Interactive, 12 choices | Pre-configured Cruiser |
| Starting Level | Level 1 | Level 10 |
| Starting Credits | 10,000 | 1,000,000 |
| Starting Zone | Galaxy Rim (Iron) | Mid-Galaxy (Naonite) |
| Test Ships | 18 ships (varied each time) | 18 ships (fixed layout) |
| Focus | Gameplay progression | Complete feature showcase |
| Resources | Starter amount | All types, abundant |

## Recommended Testing Workflow

### First Time Players:
1. Try **Option 2** first to see everything
2. Fly around to inspect all ships and stations
3. Use console commands to experiment
4. Then try **Option 1** for the full gameplay experience

### Testing Ship Variety:
1. Run **Option 1** multiple times
2. Note how ships look different each playthrough
3. Compare hull shapes: Angular vs Sleek vs Blocky vs Cylindrical vs Irregular
4. Observe different component placements

### Feature Testing:
1. Use **Option 2** for comprehensive feature validation
2. All implemented features are visible and testable
3. Positioned for easy exploration
4. Pre-configured for immediate testing

## Tips

- **Ship Variety**: In Option 1, restart the game to see completely different ship designs
- **Exploration**: Fly to different distances to see ships of varying sizes
- **Materials**: Each ship uses different materials with distinct colors
- **AI Testing**: Watch AI ships perform their roles (traders, miners, pirates)
- **Stations**: Approach stations to see different architectural styles
- **Asteroids**: Each asteroid field has a different material type
- **Console**: Use the testing console for quick experimentation

## Known Features

- Ships in Option 1 now generate with **genuine variety** each playthrough
- Explicit hull shapes ensure visual diversity
- All ship sizes from Fighter (smallest) to Carrier (largest)
- Multiple faction styles: Military, Explorers, Traders, Miners, Pirates
- Material progression from Iron (basic) to Avorion (advanced)
- Stations have different architectures: Modular vs Sprawling

## Future Enhancements

The solar system testing environment will continue to expand as new features are implemented:
- More ship types and variations
- Additional station types
- Dynamic AI behaviors
- Quest and mission systems
- Multiplayer testing scenarios

---

**Enjoy testing Codename: Subspace!**

For more information, see:
- `IN_GAME_TESTING_GUIDE.md` - Detailed testing procedures
- `CONSOLE_INTEGRATION_GUIDE.md` - Console command reference
- `GAMEPLAY_FEATURES.md` - Complete feature list
