# Main Menu System - User Guide

## Overview

The Main Menu System provides a comprehensive graphical interface for configuring and launching new games, managing save files, and setting up multiplayer sessions. It offers extensive customization options for every aspect of the game world generation and gameplay experience.

## Accessing the Main Menu

### From Console Menu
1. Run the game
2. Select option **28 - GRAPHICAL MAIN MENU** from the console menu
3. The graphical menu will open in a new window

### Direct Launch (Future)
The main menu can be configured to launch automatically on game startup.

## Main Menu Options

### 1. New Game
Create a new game with extensive customization options across multiple configuration tabs:

#### Presets Tab
Quick-start configurations for common playstyles:
- **Easy**: Relaxed gameplay with abundant resources
- **Normal**: Balanced gameplay experience
- **Hard**: Challenging with scarce resources
- **Ironman**: Hardcore mode with permadeath
- **Sandbox**: Creative mode with unlimited resources
- **Dense Galaxy**: Many sectors and factions
- **Sparse Galaxy**: Fewer sectors, more exploration

#### Galaxy Tab
Configure the overall galaxy structure:
- **Galaxy Seed**: Set a specific seed or use random (-1)
- **Galaxy Radius**: Size from center (100-1000 sectors)
- **Galaxy Density**: Sector population density (0.1x - 5.0x)
- **Total Sectors**: Approximate populated sectors (1,000 - 100,000)

#### Sectors Tab
Fine-tune asteroid fields and sector content:
- **Asteroids Per Belt**: Average number (10-200)
- **Resource Richness**: Resource multiplier (0.1x - 10.0x)
- **Asteroid Size Variation**: Size diversity (0.1x - 3.0x)
- **Min/Max Asteroids Per Sector**: Control sector density
- **Special Features**: Toggle massive asteroids, stations, anomalies, wormholes

#### Factions Tab
Control AI factions and pirates:
- **Faction Count**: Number of AI factions (1-50)
- **Faction War Frequency**: How often wars occur (0x - 3.0x)
- **Enable Pirates**: Toggle pirate presence
- **Pirate Aggression**: Low, Normal, or High

#### AI & Difficulty Tab
Configure AI behavior and game difficulty:
- **AI Difficulty**: Easy, Normal, Hard, Very Hard
- **AI Competence**: Decision-making quality (0.1x - 5.0x)
- **AI Reaction Speed**: Response time to threats
- **AI Economic Advantage**: Resource generation multiplier
- **AI Behaviors**: Toggle expansion, trading, mining
- **Player Difficulty**: Easy, Normal, Hard
- **Enemy Strength**: Combat difficulty multiplier
- **Special Modes**: Permadeath, Ironman

#### Starting Conditions Tab
Set your initial state:
- **Player Name**: Your commander name
- **Starting Region**: Rim (Iron), Mid (Titanium), or Core (Avorion)
- **Starting Credits**: Initial money (0 - 1,000,000)
- **Starting Ship Class**: Starter Pod, Fighter, Miner, or Trader
- **Resource Gathering Speed**: Multiplier for mining/salvage

#### Summary Tab
Review all settings before starting the game.

### 2. Load Game
Manage and load saved games:
- View list of all saved games
- See save date/time and metadata
- Select and load a save
- Delete unwanted saves (with confirmation)
- Right-click for quick options

### 3. Host Multiplayer
Set up a multiplayer server:
- **Server Name**: Visible name for your server
- **Server Port**: Network port (default 27015)
- **Max Players**: Player limit (2-50)
- Links to new game configuration for world setup

### 4. Join Multiplayer
Connect to multiplayer games:
- **Server Browser**: List available servers (future feature)
- **Direct Connect**: Enter IP address and port
- **Player Name**: Your in-game name
- **Refresh**: Update server list

### 5. Settings
Access game settings (links to existing settings menu):
- Graphics settings
- Audio settings
- Gameplay preferences
- Controls configuration

### 6. Quit
Exit the game

## Controls

### Mouse
- Click buttons to select options
- Click and drag sliders to adjust values
- Right-click save games for quick options
- Scroll in list views

### Keyboard
- Tab: Navigate between tabs
- Arrow keys: Adjust sliders
- Enter: Confirm selection
- ESC: Go back/cancel

## Tips

### Galaxy Configuration
- **For Beginners**: Use "Easy" or "Normal" presets
- **For Challenge**: Try "Hard" or "Ironman" modes
- **For Creativity**: Use "Sandbox" mode with unlimited resources
- **For Exploration**: Choose "Sparse Galaxy" for more isolated sectors
- **For Action**: Choose "Dense Galaxy" for frequent encounters

### AI Settings
- **AI Competence** affects strategic decision-making quality
- **AI Reaction Speed** affects how quickly AI responds to threats
- **AI Economic Advantage** gives AI factions resource bonuses
- Lower AI settings if you want a more relaxed experience
- Higher AI settings create more challenging opponents

### Starting Conditions
- **Rim Start** (Iron tier): Safest, slowest progression
- **Mid Start** (Titanium tier): Balanced difficulty and progression
- **Core Start** (Avorion tier): Dangerous but fastest access to best materials
- **Starter Pod**: Smallest, most limited ship
- **Fighter**: Combat-ready from the start
- **Miner**: Best for resource gathering
- **Trader**: Largest cargo capacity

### Save Management
- Save frequently to protect progress
- Use descriptive save names
- Delete old saves to free disk space
- Ironman mode restricts save/load functionality

## Configuration Details

### Galaxy Generation
The galaxy is generated procedurally based on your seed and settings:
- **Seed** determines the layout of all sectors
- **Radius** determines the physical size
- **Density** affects resource availability and encounters
- **Total Sectors** approximates how many sectors contain content

### Asteroid Configuration
Asteroids are the primary source of resources:
- **Per Belt** controls abundance in asteroid fields
- **Richness** multiplies resource yields
- **Size Variation** creates visual diversity
- **Min/Max Per Sector** prevents empty or overcrowded sectors

### Faction System
AI factions control territory and interact with each other:
- More factions create more complex politics
- War frequency affects galactic stability
- Pirates add danger but also loot opportunities
- Starting relations affect early game difficulty

### Difficulty Modifiers
Multiple systems affect overall difficulty:
- **Player Difficulty**: Affects your ship performance
- **AI Difficulty**: Affects enemy ship quality
- **AI Competence**: Affects strategic intelligence
- **Enemy Strength**: Direct combat multiplier
- **Resource Gathering**: Affects mining/salvage speed

## Multiplayer

### Hosting
1. Click "Host Multiplayer"
2. Configure server name, port, and player limit
3. Proceed to configure new game settings
4. Start server - other players can join

### Joining
1. Click "Join Multiplayer"
2. Enter server address and port (or select from list)
3. Enter your player name
4. Click "Connect"

## Troubleshooting

### Menu Not Appearing
- Ensure graphics drivers are up to date
- Check that OpenGL 3.3+ is supported
- Try running in windowed mode
- Check console for error messages

### Settings Not Saving
- Verify write permissions in game directory
- Check disk space availability
- Ensure configuration file isn't read-only

### Game Crashes After Start
- Review settings for extreme values
- Try a preset configuration first
- Check system meets minimum requirements
- Report issue with seed and settings used

## Advanced Usage

### Custom Seeds
- Use specific seeds to replay interesting galaxies
- Share seeds with other players
- Document seeds that produce unique scenarios

### Balanced Configurations
- Match AI difficulty to player difficulty
- Scale resource richness with gathering speed
- Balance economy scale with starting credits

### Extreme Configurations
- Maximum density + maximum sectors = huge galaxy
- Minimum resources + hard difficulty = survival challenge
- No AI + sandbox mode = creative building
- Ironman + very hard = ultimate challenge

## Future Features

Planned enhancements to the main menu system:
- Mod selection and configuration
- Custom faction creation
- Advanced galaxy shape options
- Terrain/environment presets
- Achievement tracking
- Friend system for multiplayer
- Cloud save support
- Screenshot/video sharing

## Feedback

The main menu system is designed to be comprehensive yet intuitive. If you encounter issues or have suggestions:
- Report bugs via GitHub issues
- Request features via discussions
- Share interesting configurations with the community

---

For more information, see:
- [Quick Start Guide](QUICKSTART.md)
- [Gameplay Guide](GAMEPLAY_LOOP_GUIDE.md)
- [Multiplayer Guide](README.md#multiplayer)
