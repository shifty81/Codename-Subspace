# Consolidated Visual Testing Guide

## Overview
All test implementations have been consolidated into **Option 1 (NEW GAME)** for easy visual testing.

## What's Included

When you select Option 1, you now get a comprehensive visual testing environment that includes:

### Ship Implementations (17+ test ships)
- **Fighters** - Military and Scout variants
- **Corvettes** - Combat and Mining variants  
- **Frigates** - Military, Trading, and Explorer variants
- **Destroyers** - Heavy Combat and Salvage variants
- **Cruisers** - Battle and Trade variants
- **Battleships** - Dreadnought class
- **Carriers** - Fleet Carrier

### Faction Styles
All ships are generated with different faction styles for visual comparison:
- Military (Angular, aggressive designs)
- Traders (Cylindrical, cargo-focused)
- Explorers (Sleek, aerodynamic)
- Miners (Blocky, industrial)
- Pirates (Irregular, salvaged parts)

### Station Implementations (3 stations)
- Trading Station
- Military Station
- Industrial Station

### Visual Enhancements
All ships and stations include:
- Material-based coloring (Avorion, Xanion, Titanium, etc.)
- Functional block placement
- Procedural generation with cohesive designs
- Faction-specific styling

### Additional Tests
- Ship connectivity verification (no floating blocks)
- Movement and shape tests
- AI-driven voxel construction examples

## How to Use

1. **Run the Application**
   ```bash
   cd AvorionLike
   dotnet run
   ```

2. **Select Option 1**
   - Choose "1. NEW GAME - Start Full Gameplay Experience"

3. **Visual Testing**
   - The 3D graphics window will open
   - Your player ship is at the center (0,0,0)
   - Test ships are arranged in a grid pattern around you:
     - Right side (+X): Fighters, Corvettes, Frigates
     - Far right (+X): Destroyers, Cruisers, Battleships, Carrier
     - Left side (-X, +Z): Different hull shape comparisons
     - Above (+Y, +Z): Trading, Military, and Industrial stations

4. **Controls**
   - **C** - Toggle between Camera and Ship Control mode
   - **WASD** - Movement/Thrust
   - **Arrow Keys** - Pitch/Yaw rotation
   - **Q/E** - Roll rotation
   - **Mouse** - Free look (in Camera mode)
   - **ESC** - Exit to menu

5. **Navigation Tips**
   - Press **C** to enter Camera mode for free-look
   - Use mouse to look around and find ships
   - Fly towards ships to inspect them up close
   - Ships range from 150 to 1000 units away from spawn point

## Ship Positions

### Main Test Grid (Right Side)
- 150 units: Fighters
- 200 units: Corvettes  
- 300 units: Frigates (3 variants in Y axis)
- 450 units: Destroyers
- 600 units: Cruisers
- 800 units: Battleship
- 1000 units: Carrier

### Hull Shape Comparison (Left Rear)
- (-150, 0, 100): Angular Fighter
- (-150, 0, 150): Sleek Fighter
- (-150, 0, 200): Blocky Trader
- (-150, 0, 250): Cylindrical Hauler

### Stations (Above and Forward)
- (0, 200, 400): Trading Station
- (0, 350, 400): Military Station
- (0, 500, 400): Industrial Station

## What to Look For

### Ship Quality Checklist
- ✅ All blocks connected (no floating pieces)
- ✅ Proper material colors visible
- ✅ Functional components placed logically
- ✅ Faction-appropriate styling
- ✅ Size-appropriate block counts
- ✅ Smooth rendering without gaps

### Station Quality Checklist
- ✅ Modular architecture visible
- ✅ Communication arrays/antennas present
- ✅ Docking points identifiable
- ✅ Proper scale and presence
- ✅ Type-appropriate design (trading vs military vs industrial)

## Troubleshooting

### Graphics Window Won't Open
- Ensure you have OpenGL support
- Try running on a system with graphics capabilities
- Check logs in `~/.config/Codename-Subspace/Logs/`

### Can't See All Ships
- Press C to enter Camera mode
- Rotate view with mouse
- Fly around using WASD
- Ships are spread across 1000+ units, so exploration is needed

### Performance Issues
- Reduce the number of test ships in `CreateComprehensiveTestShowcase()` 
- Ships are generated on-demand, so performance should be acceptable
- Check system requirements in README.md

## Customization

To modify the test showcase, edit the `CreateComprehensiveTestShowcase()` method in `Program.cs`:

```csharp
// Add more ship types
new { Name = "Custom Ship", Size = ShipSize.Frigate, Role = ShipRole.Combat, 
      Material = "Avorion", Style = "Military", Position = new Vector3(500, 0, 0) }

// Change positions
Position = new Vector3(x, y, z)  // Adjust coordinates

// Change ship properties
Material = "Ogonite"  // Red/orange
Material = "Xanion"   // Gold
Material = "Trinium"  // Blue
Material = "Naonite"  // Green
Material = "Avorion"  // Purple
```

## Benefits of Consolidated Testing

1. **Single Entry Point** - No need to run 20+ different menu options
2. **Visual Comparison** - See all implementations side-by-side
3. **Complete Testing** - All features visible in one session
4. **Easy Navigation** - Ships arranged logically by type and size
5. **Quick Iteration** - Make changes and see results immediately

## Development Notes

The showcase generation happens in `CreateComprehensiveTestShowcase()` which is called from `StartNewGame()` before launching the graphics window. This ensures all test entities are present when the 3D view opens.

The showcase includes error handling, so if individual ships fail to generate, the rest will still be created and displayed.
