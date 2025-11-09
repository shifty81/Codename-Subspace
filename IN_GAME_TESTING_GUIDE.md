# In-Game Testing Console Guide

## Overview

The **In-Game Testing Console** is a powerful developer tool that allows you to test and interact with all game systems during live gameplay without restarting. It provides commands to spawn entities, modify resources, test AI behaviors, and much more.

## Activating the Console

**Press `~` (tilde key)** while in-game to toggle the console on/off.

- The console appears as an overlay at the bottom of the screen
- Previous command history is displayed
- Type commands and press **Enter** to execute
- Press **ESC** or **~** again to close the console

## How to Use

1. Start a new game (Option 1 from main menu)
2. Once in the 3D view, press `~` to open the console
3. Type `help` to see all available commands
4. Type any command and press Enter
5. View results in the console output

## Command Categories

### 🚀 Entity Spawning

Spawn various entities near your position for testing.

#### `spawn_ship [material] [x] [y] [z]`
Spawns a basic test ship.
- **material**: Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion (default: Titanium)
- **x y z**: Position coordinates (default: near player)
- **Example**: `spawn_ship Avorion 100 50 0`

#### `spawn_fighter`
Spawns a fighter-class ship with engines and thrusters near the player.
- **Example**: `spawn_fighter`

#### `spawn_cargo`
Spawns a large cargo ship near the player.
- **Example**: `spawn_cargo`

#### `spawn_enemy [personality]`
Spawns an AI-controlled enemy ship.
- **personality**: aggressive, defensive, miner (default: aggressive)
- **Example**: `spawn_enemy aggressive`

#### `spawn_asteroid [resourceType] [x] [y] [z]`
Spawns a mineable asteroid.
- **resourceType**: Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion
- **x y z**: Position coordinates (default: near player)
- **Example**: `spawn_asteroid Titanium`

#### `spawn_station [type]`
Spawns a space station.
- **type**: trading, mining, military (default: trading)
- **Example**: `spawn_station military`

#### `populate_sector [count]`
Fills the current sector with a mix of entities.
- **count**: Number of entities to spawn (default: 10)
- **Example**: `populate_sector 20`

#### `clear_entities`
Removes all entities except the player ship.
- **Example**: `clear_entities`

### ⚔️ Combat Testing

Test combat systems and damage mechanics.

#### `damage [amount]`
Deals damage to the player ship.
- **amount**: Damage to deal (default: 100)
- **Example**: `damage 500`

#### `heal`
Fully restores player shields and energy.
- **Example**: `heal`

#### `godmode`
Toggle invincibility mode (placeholder - currently shows message).
- **Example**: `godmode`

### 💰 Resource & Economy

Manage resources and credits for testing.

#### `credits [amount]`
Adds credits to player inventory.
- **amount**: Credits to add (default: 10000)
- **Example**: `credits 50000`

#### `add_resource <type> <amount>`
Adds specific resources to inventory.
- **type**: Iron, Titanium, Naonite, Trinium, Xanion, Ogonite, Avorion, Credits
- **amount**: Quantity to add
- **Example**: `add_resource Avorion 1000`

#### `clear_inventory`
Removes all resources from player inventory.
- **Example**: `clear_inventory`

### 🎯 Physics & Movement

Control player position and velocity.

#### `tp <x> <y> <z>`
Teleports player to specified coordinates.
- **x y z**: Target coordinates
- **Example**: `tp 500 100 -200`

#### `velocity <x> <y> <z>`
Sets player velocity vector.
- **x y z**: Velocity components (m/s)
- **Example**: `velocity 50 0 0`

#### `stop`
Stops all player movement (linear and angular).
- **Example**: `stop`

### 🤖 AI Testing

Test AI behaviors and states.

#### `ai_state <state>`
Changes the state of the nearest AI entity.
- **state**: idle, combat, mining, fleeing, patrol, trading
- **Example**: `ai_state combat`

#### `ai_attack_player`
Makes the nearest AI entity target and attack the player.
- **Example**: `ai_attack_player`

### 💾 Save/Load

Quick save and load during testing.

#### `quicksave`
Saves the current game state to quicksave slot.
- **Example**: `quicksave`

#### `quickload`
Loads from quicksave slot.
- **Example**: `quickload`

### ℹ️ Information

Display information about the game state.

#### `pos`
Shows player's current position.
- **Example**: `pos`

#### `stats`
Displays detailed player ship statistics:
- Position and velocity
- Mass
- Shield and energy levels
- Block count and structural integrity
- **Example**: `stats`

#### `list_entities`
Lists all entities currently in the world (shows first 20).
- **Example**: `list_entities`

#### `help`
Shows all available commands with brief descriptions.
- **Example**: `help`

### 🔧 Console Management

Built-in console commands.

#### `clear`
Clears the console output.
- **Example**: `clear`

#### `history`
Shows command history.
- **Example**: `history`

#### `echo <text>`
Echoes text to console output.
- **Example**: `echo Hello World`

#### `lua <script>`
Executes Lua script code directly.
- **Example**: `lua log('Testing from console')`

#### `gc`
Forces garbage collection and shows memory freed.
- **Example**: `gc`

#### `mem`
Shows current memory usage.
- **Example**: `mem`

#### `exit`
Closes the console (same as ESC or ~).
- **Example**: `exit`

## Testing Workflows

### Testing Combat

```
# Spawn enemy ships
spawn_enemy aggressive
spawn_enemy aggressive

# Make them attack you
ai_attack_player

# Damage yourself to test
damage 500

# Heal when needed
heal

# Clear when done
clear_entities
```

### Testing Economy

```
# Add lots of credits
credits 100000

# Add various resources
add_resource Iron 500
add_resource Titanium 300
add_resource Avorion 100

# Check stats
stats

# Clear to start fresh
clear_inventory
```

### Testing World Population

```
# Create a busy sector
populate_sector 30

# List all entities
list_entities

# Clear when done
clear_entities
```

### Testing Movement

```
# See current position
pos

# Teleport far away
tp 1000 500 -800

# Check position again
pos

# Set high velocity
velocity 100 50 0

# Stop immediately
stop
```

### Testing AI Behaviors

```
# Spawn different AI types
spawn_enemy aggressive
spawn_enemy defensive
spawn_enemy miner

# Change AI states
ai_state patrol
ai_state mining
ai_state combat
```

## Tips & Best Practices

1. **Use `help` frequently** - The console has many commands, use help to remind yourself
2. **Save before major tests** - Use `quicksave` before spawning lots of entities or testing dangerous scenarios
3. **Clear entities regularly** - Use `clear_entities` to reset the world when it gets cluttered
4. **Check `stats` often** - Monitor your ship's condition during combat tests
5. **Combine commands** - Chain multiple commands for complex scenarios
6. **Use `populate_sector`** - Quick way to create a realistic environment
7. **Test incrementally** - Start with one or two entities before spawning many

## Keyboard Shortcuts

- **`~` (Tilde)** - Toggle console on/off
- **Enter** - Execute command
- **Backspace** - Delete character
- **ESC** - Close console
- **Shift** - Uppercase letters and symbols

## Troubleshooting

**Console won't open:**
- Make sure you're in the gameplay view (not main menu)
- Try pressing the key next to the "1" key (keyboard layout may vary)
- Check that no other UI elements are blocking input

**Commands not working:**
- Check spelling - commands are case-insensitive
- Check syntax - some commands require arguments
- Use `help` to verify command names
- Check console output for error messages

**Spawned entities not visible:**
- They may be spawned far from camera
- Use camera mode (press C) to look around
- Check with `list_entities` to confirm they exist
- Try spawning closer with explicit coordinates

**Console output is too long:**
- Use `clear` to clear the history
- Only the last 20 lines are shown

## Integration with Other Systems

The console integrates with:
- **Entity Manager** - Spawn and manage entities
- **Physics System** - Control movement and position
- **Combat System** - Deal damage and test shields
- **AI System** - Control AI behaviors
- **Resource System** - Manage inventory and credits
- **Save System** - Quick save/load functionality
- **Procedural Generation** - Populate sectors

## Future Enhancements

Planned features for future versions:
- Command auto-completion
- Command history navigation (up/down arrows)
- Variable system (save values for reuse)
- Macro recording and playback
- Time manipulation commands
- Weather/environment commands
- Spawn templates for complex scenarios

## Example Session

Here's a complete testing session:

```
> help
Available Commands:
  [list of commands...]

> spawn_fighter
Spawned fighter ship at (30, 10, 0)

> spawn_enemy aggressive
Spawned aggressive enemy at (50, 0, 0)

> ai_attack_player
AI entity Enemy Ship now targeting player

> stats
=== Player Ship Stats ===
Position: (0, 0, 0)
Velocity: (0, 0, 0) (0.0 m/s)
Mass: 1500 kg
Shields: 500/500
Energy: 200/200
Blocks: 9
Integrity: 100.0%

> damage 200
Dealt 200 damage. Shields: 300/500

> heal
Fully healed. Shields: 500, Energy: 200

> clear_entities
Cleared 2 entities

> quicksave
Game saved
```

## Conclusion

The In-Game Testing Console is an essential tool for rapid development and testing. It allows you to exercise all game systems without tedious setup or restarts. Use it to:

- **Test new features** quickly
- **Debug issues** in a live environment
- **Create scenarios** for screenshots or videos
- **Learn the systems** by experimenting
- **Verify fixes** immediately

Happy testing! 🚀
