# Galaxy Map Guide

## Overview

The Galaxy Map is an interactive navigation interface that allows players to explore the procedurally generated galaxy, view sector information, and plan hyperdrive jumps between solar systems.

## Opening the Map

Press **M** at any time during gameplay to toggle the Galaxy Map.

## Interface Layout

The Galaxy Map consists of two main sections:

### Map View (Left)
- **2D Sector Grid**: Visual representation of galaxy sectors
- **Current Location**: Highlighted in bright blue
- **Tech Level Colors**: Sectors are colored based on distance from galaxy center
  - Red (Core): Avorion regions
  - Orange/Yellow: Ogonite/Xanion regions  
  - Green: Trinium regions
  - Blue: Naonite regions
  - Dark Blue: Titanium regions
  - Gray (Outer): Iron regions
- **Jump Range Circle**: Blue circle shows reachable sectors
- **Content Indicators**: 
  - Green dot = Station present
  - Yellow dot = Rich asteroid field

### Info Panel (Right)
- **Sector Coordinates**: Current or selected sector position
- **Distance from Center**: How far from galaxy core (0,0,0)
- **Tech Level**: Material tier available in this region
- **Contents**: Stations, asteroids, and ships in the sector
- **Distance to Target**: Sectors from your current location
- **Jump Status**: Can jump, charging, or on cooldown

## Controls

### Navigation
- **Mouse Scroll**: Zoom in/out (0.2x to 5x)
- **Left Click + Drag**: Pan around the galaxy
- **+/- Buttons**: Change Z-slice (vertical layer)
- **Reset View Button**: Center on current location at default zoom

### Interaction
- **Left Click on Sector**: Select sector and view detailed information
- **Right Click on Sector**: Initiate hyperdrive jump (if in range)
- **Hover over Sector**: Quick preview of sector info

### Filters
- **Stations Checkbox**: Show/hide station indicators
- **Asteroids Checkbox**: Show/hide asteroid indicators
- **Ships Checkbox**: Show/hide ship indicators (if present)
- **Jump Range Checkbox**: Show/hide jump range circle

## Hyperdrive Jumps

### Initiating a Jump
1. Open Galaxy Map (M key)
2. Select a sector within your jump range (highlighted by blue circle)
3. Right-click the target sector OR click "Initiate Jump" button in info panel
4. Jump charging will begin automatically

### Jump Status
- **Ready**: Green "WITHIN JUMP RANGE" - can initiate jump
- **Charging**: Yellow progress bar showing charge percentage
- **Cooldown**: Gray progress bar showing time until next jump available
- **Out of Range**: Red "OUT OF JUMP RANGE" - need to upgrade hyperdrive

### Jump Requirements
- Target sector must be within jump range (shown by blue circle)
- Hyperdrive must not be on cooldown
- No active jump in progress

### Canceling a Jump
While charging, click the "Cancel Jump" button in the info panel.

## Understanding Sectors

### Tech Levels
The galaxy is organized in concentric rings around the center (0,0,0):
- **Level 7 (Core)**: Avorion - Most valuable, highest danger
- **Level 6**: Ogonite
- **Level 5**: Xanion
- **Level 4**: Trinium
- **Level 3**: Naonite
- **Level 2**: Titanium
- **Level 1 (Outer)**: Iron - Safest, least valuable

Distance determines which materials and technologies are available.

### Sector Contents
Each sector may contain:
- **Stations**: Trading posts, military bases, shipyards, etc.
- **Asteroids**: Resource-rich mining opportunities
- **Ships**: NPC vessels, pirates, or other players (multiplayer)

Contents are procedurally generated based on sector coordinates and remain consistent.

### Empty Sectors
Not all sectors contain points of interest. Some are empty space useful for:
- Safe navigation routes
- Strategic positioning
- Establishing new outposts

## Strategic Tips

### Exploration
- Work outward from starting position systematically
- Check adjacent sectors for stations and resources
- Note tech levels for upgrade planning

### Jump Planning
- Plan routes through sectors with stations for refueling
- Avoid dangerous high-tech sectors early game
- Use empty sectors as waypoints for long-distance travel

### Resource Gathering
- Yellow-highlighted sectors have rich asteroid fields
- Higher tech levels yield better materials
- Stations provide trading opportunities

### Combat Avoidance
- Outer sectors (Iron regions) are generally safer
- Core sectors have stronger enemies but better rewards
- Use empty sectors to avoid confrontation

## Upgrading Jump Range

To reach distant sectors:
1. Collect resources through mining and trading
2. Visit shipyards or equipment stations
3. Purchase hyperdrive upgrades with credits
4. Improved jump range expands the blue circle

## Legend Reference

**Sector Colors:**
- 🔴 Red: Avorion (Core, Level 7)
- 🟠 Orange: Ogonite (Level 6)
- 🟡 Yellow: Xanion (Level 5)
- 🟢 Green: Trinium (Level 4) or Station present
- 🔵 Blue: Naonite (Level 3) or Current location
- ⚫ Gray: Iron (Outer, Level 1)

**Visual Indicators:**
- Blue Square: Your current sector
- Yellow Border: Selected sector
- White Border: Hovered sector
- Blue Circle: Jump range boundary
- Green Dot: Station
- Yellow Dot: Rich asteroids
- Dark Overlay: Out of jump range

## Performance Notes

- Sector data is cached for recently viewed areas
- Zoom out for overview, zoom in for details
- Cache automatically manages memory usage
- Procedural generation is fast and deterministic

## Keyboard Shortcuts Summary

- **M**: Toggle Galaxy Map
- **ESC**: Close Galaxy Map (if no other menus open)
- **Mouse Scroll**: Zoom
- **Left Click**: Select sector
- **Right Click**: Jump to sector
- **+**: Next Z-slice
- **-**: Previous Z-slice

## Troubleshooting

**Galaxy Map won't open?**
- Make sure you're not in a menu
- Check that you have a player ship
- Verify M key is not bound elsewhere

**Can't jump to sector?**
- Check if sector is within blue circle (jump range)
- Ensure hyperdrive is not on cooldown (gray bar)
- Wait for current jump to finish charging

**Sectors appear empty?**
- Some sectors are naturally empty
- Rich sectors are less common
- Try exploring different Z-slices

**Performance issues?**
- Reduce zoom level when panning large distances
- Close and reopen map to clear cache
- Disable filters you don't need

## Future Features (Planned)

- Multi-jump waypoint system
- Mission markers on map
- Faction territory overlays
- Sector bookmarks and notes
- Auto-navigation
- Trade route visualization
- Multiplayer ship tracking

---

For more information, see:
- [Navigation System Guide](NAVIGATION_GUIDE.md) (if exists)
- [Quick Start Guide](QUICKSTART.md)
- [Main README](README.md)
