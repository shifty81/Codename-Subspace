# Console Integration Guide

## Overview
The in-game testing console has been fully integrated into the GUI, making all gameplay features easily testable during the Option 1 (NEW GAME) experience.

## What's New? ✨

### 1. Console Toggle Button
- **Always Visible**: A console toggle button now appears in the bottom-left corner of the screen
- **Location**: Bottom-left corner (10px from left, 320px from bottom)
- **Visual**: Styled button with cyan theme matching the game's futuristic HUD
- **Text**: Shows "▼ CONSOLE" when open, "▲ CONSOLE" when closed
- **No More Hidden Hotkeys**: You don't need to remember the `~` key anymore!

### 2. Enhanced Console UI
The console window now features:
- **Dark Theme**: Semi-transparent dark background (rgba 0, 5, 8, 95%)
- **Cyan Border**: Bright cyan border (3px) matching the HUD aesthetic
- **Color-Coded Output**:
  - ✅ Green text for success messages (commands that worked)
  - ❌ Red text for errors and failures
  - 💠 Cyan text for user commands
  - ⚪ White text for general information
- **Help Text**: Shows quick command overview at the top
- **Auto-Scroll**: Automatically scrolls to show latest output
- **Title**: "⬡ IN-GAME TESTING CONSOLE" with futuristic styling

### 3. Welcome Message
When you open the console, you're greeted with:
```
=== In-Game Testing Console ===
Type 'help' for all commands
Quick Commands: demo_quick, demo_combat, demo_mining, demo_world, demo_economy
Spawning: spawn_ship, spawn_enemy, spawn_asteroid, spawn_station
Testing: heal, damage, tp, velocity, credits, add_resource
Info: stats, pos, list_entities
```

## Quick Demo Commands

### `demo_quick` - Fast Setup
Instantly spawns a basic test scenario:
- 2 asteroids (Iron and Titanium)
- 1 friendly ship
- 1 enemy ship
Perfect for quick iteration and testing!

### `demo_combat` - Combat Testing
Spawns 3 aggressive enemy fighters in formation around your position.
Tests:
- AI combat behavior
- Targeting systems
- Weapon systems (when implemented)
- Shield/damage mechanics

### `demo_mining` - Mining Testing
Spawns 8 resource-rich asteroids in a circle around you.
Resources included: Iron, Titanium, Naonite, Trinium
Tests:
- Mining mechanics
- Resource collection
- Inventory systems

### `demo_economy` - Economy Testing
Instantly adds:
- 100,000 credits
- 1,000 Iron
- 500 Titanium
- 200 Naonite
- 100 Trinium
Tests:
- Resource management
- Trading systems (when implemented)
- Crafting systems (when implemented)

### `demo_world` - World Population
Spawns 20 mixed entities randomly distributed around you:
- Asteroids with various resources
- Neutral NPC ships
- Enemy ships with AI
Tests:
- World population
- Performance with multiple entities
- Entity variety
- Spatial distribution

## All Console Commands

### Entity Spawning
- `spawn_ship [material] [x y z]` - Spawn a test ship
- `spawn_fighter` - Spawn a fighter near player
- `spawn_cargo` - Spawn a cargo ship
- `spawn_enemy [aggressive/defensive/miner]` - Spawn AI enemy
- `spawn_asteroid [resourceType] [x y z]` - Spawn asteroid
- `spawn_station [trading/mining/military]` - Spawn a station
- `populate_sector [count]` - Populate with mixed entities
- `clear_entities` - Remove all non-player entities

### Combat Testing
- `damage [amount]` - Damage player ship
- `heal` - Restore player shields
- `godmode` - Toggle invincibility (placeholder)

### Resources/Economy
- `add_resource [type] [amount]` - Add specific resource
- `credits [amount]` - Add credits (default: 10000)
- `clear_inventory` - Remove all resources

### Physics/Movement
- `tp [x y z]` - Teleport player
- `velocity [x y z]` - Set player velocity
- `stop` - Stop all movement

### AI Testing
- `ai_state [state]` - Set AI state for nearest entity
- `ai_attack_player` - Make nearest AI attack player

### World/Save
- `quicksave` - Quick save game
- `quickload` - Quick load game

### Information
- `pos` - Show player position
- `stats` - Show detailed player ship stats
- `list_entities` - List all entities in world

### Utility
- `help` - Show all commands
- `clear` - Clear console output
- `exit` - Close console

## Option 1 Integration

### Startup Instructions
When starting Option 1 (NEW GAME), you'll see:
```
💡 TESTING FEATURES:
  • Press ~ or click Console button to open testing console
  • Type 'help' in console for all commands
  • Quick demos: demo_quick, demo_combat, demo_mining, demo_world
  • Spawn entities: spawn_ship, spawn_enemy, spawn_asteroid
  • Resources: credits [amount], add_resource [type] [amount]
  • Testing: tp [x y z], velocity [x y z], heal, damage [amount]
```

### Visual Testing Workflow
1. **Start Option 1** - Enter the full gameplay experience
2. **Click Console Button** - Open console (bottom-left corner)
3. **Type Command** - E.g., `demo_quick` or `spawn_enemy aggressive`
4. **See Results Visually** - Watch entities spawn in 3D view
5. **Test Interactions** - Fly around, test combat, gather resources
6. **Iterate Quickly** - Type more commands without leaving the game

## Benefits

### For Developers
- ✅ **Rapid Iteration**: Test features without restarting the game
- ✅ **Visual Feedback**: See results immediately in 3D view
- ✅ **Comprehensive Testing**: All systems testable through commands
- ✅ **Easy Debugging**: Spawn scenarios, test edge cases instantly

### For Testers
- ✅ **No Learning Curve**: Button is always visible, no hotkeys to remember
- ✅ **Guided Experience**: Welcome message and help text guide you
- ✅ **Quick Demos**: Pre-configured scenarios for common tests
- ✅ **Visual Testing**: All features testable in real gameplay context

### For Players
- ✅ **Accessible**: Easy to find and use
- ✅ **Helpful**: Clear instructions and command categories
- ✅ **Powerful**: Full control over game state for experimentation
- ✅ **Non-Intrusive**: Button is small, console hidden by default

## Technical Details

### UI Positioning
- Console Button: (10, window.height - 320), Size: (150, 30)
- Console Window: (10, window.height - 310), Size: (window.width - 20, 300)
- Z-Order: Rendered after game HUD, before ImGui overlay

### Styling
- Window Background: rgba(0, 5, 8, 0.95) - Nearly opaque dark background
- Border: rgba(0, 230, 255, 0.9) - Bright cyan border, 3px
- Title Background: rgba(0, 51, 64, 0.9) - Dark cyan title bar
- Button: rgba(0, 77, 102, 0.7) - Semi-transparent cyan button
- Button Hover: rgba(0, 128, 153, 0.9) - Brighter on hover

### Input Handling
- Keyboard input captured when console is open
- ESC key closes console
- Enter executes command
- Backspace removes last character
- All printable characters supported (a-z, 0-9, symbols)
- Shift key for uppercase

## Future Enhancements

### Potential Additions
- [ ] Command history navigation (Up/Down arrows)
- [ ] Auto-complete for commands
- [ ] Customizable console size/position
- [ ] Console color themes
- [ ] Save/load console command macros
- [ ] Command aliases for frequently used commands
- [ ] Visual command builder (GUI for complex commands)

## Screenshots

### Console Closed
```
┌─────────────────────────────────────┐
│  [Game HUD, Ship Status, etc.]      │
│                                     │
│         [3D Gameplay View]          │
│                                     │
│  ┌──────────────┐                  │
│  │ ▲ CONSOLE    │ ← Button visible │
│  └──────────────┘                  │
└─────────────────────────────────────┘
```

### Console Open
```
┌─────────────────────────────────────┐
│  [Game HUD, Ship Status, etc.]      │
│         [3D Gameplay View]          │
│  ┌──────────────────────────────────┐
│  │ ⬡ IN-GAME TESTING CONSOLE       │
│  │ Type 'help' for commands...     │
│  ├──────────────────────────────────┤
│  │ === In-Game Testing Console === │
│  │ Quick Commands: demo_quick...   │
│  │ > spawn_enemy aggressive        │
│  │ ✓ Spawned enemy at (50, 0, 0) │
│  │ > _                             │
│  └──────────────────────────────────┘
│  ┌──────────────┐                  │
│  │ ▼ CONSOLE    │ ← Click to close │
│  └──────────────┘                  │
└─────────────────────────────────────┘
```

## Conclusion

The console is now fully integrated into the GUI, making Option 1 (NEW GAME) a comprehensive testing environment where all gameplay features can be visually tested without leaving the 3D view. The button makes the console discoverable, the welcome message guides users, and the demo commands provide instant testing scenarios.

**Ready to test? Start Option 1 and click the Console button!** 🚀
