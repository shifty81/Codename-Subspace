# Ship Decal System Guide

## Overview

The Decal System allows you to apply decorative patterns to ship blocks during construction and editing. Decals are purely visual and don't affect block functionality. They're inspired by the industrial spaceship aesthetic seen in games like Avorion (reference: 1234.PNG).

## What Are Decals?

Decals are decorative patterns applied to the faces of voxel blocks. Think of them as stickers or paint schemes you can add to your ship:

- **Hazard Stripes**: Yellow/black diagonal warning stripes (like on industrial equipment)
- **Racing Stripes**: Horizontal stripes along the ship's length
- **Red Accents**: Bright red accent stripes for visual pop
- **Engine Glow**: Cyan/blue glowing stripes for engine blocks
- **Faction Markings**: Custom emblems and colors
- **Checker Patterns**: Checkerboard designs
- **Weathering Marks**: Battle damage and wear effects

## Applying Decals to Blocks

### Basic Usage

```csharp
// Get a block
var block = ship.Structure.Blocks[0];

// Add a hazard stripe decal
block.AddDecal(DecalLibrary.HazardStripes());

// Add an engine glow decal
block.AddDecal(DecalLibrary.EngineGlowStripes());

// Add a racing stripe with custom color
block.AddDecal(DecalLibrary.RacingStripes(0xFF0000)); // Red
```

### Using the Decal Library

The `DecalLibrary` provides predefined decals for common patterns:

```csharp
// Hazard stripes (yellow/black diagonal)
var hazard = DecalLibrary.HazardStripes();

// Red accent stripes
var redAccent = DecalLibrary.RedAccentStripe();

// Racing stripes (custom color)
var racing = DecalLibrary.RacingStripes(0x00FF00); // Green

// Faction marking
var faction = DecalLibrary.FactionMarking(0x0000FF); // Blue

// Engine glow (cyan)
var engineGlow = DecalLibrary.EngineGlowStripes();

// Checker pattern (two colors)
var checker = DecalLibrary.CheckerPattern(0xFFFFFF, 0x000000);

// Weathering/damage marks
var weathering = DecalLibrary.WeatheringMarks();
```

### Creating Custom Decals

```csharp
var customDecal = new BlockDecal
{
    Pattern = DecalPattern.HazardStripes,
    PrimaryColor = 0xFFCC00,    // Yellow-orange
    SecondaryColor = 0x000000,   // Black
    Scale = 1.5f,                // Larger pattern
    Rotation = 60f,              // Rotate 60 degrees
    Opacity = 0.9f,              // Slightly transparent
    TargetFace = BlockFace.Top | BlockFace.Right // Apply to top and right faces
};

block.AddDecal(customDecal);
```

## Managing Decals

### Removing Decals

```csharp
// Remove a specific decal by ID
bool removed = block.RemoveDecal(decalId);

// Remove all decals of a specific pattern
int count = block.RemoveDecalsByPattern(DecalPattern.HazardStripes);

// Clear all decals from a block
block.ClearDecals();
```

### Querying Decals

```csharp
// Check if block has decals
if (block.HasDecals())
{
    // Get all decals
    var allDecals = block.Decals;
    
    // Get decals of a specific pattern
    var hazardDecals = block.GetDecalsByPattern(DecalPattern.HazardStripes);
}
```

## Decal Properties

### Pattern Types

- `None`: No pattern
- `HazardStripes`: Yellow/black diagonal warning stripes
- `RacingStripes`: Horizontal stripes along length
- `FactionMarking`: Custom faction emblem/color
- `RedAccent`: Red accent stripes
- `CheckerPattern`: Checkerboard pattern
- `CamoPattern`: Camouflage pattern
- `GlowStripes`: Glowing stripes (for engines)
- `NumberMarking`: Hull numbers/identification
- `WeatheringMarks`: Battle damage/wear decals

### Color Properties

```csharp
decal.PrimaryColor = 0xFFCC00;   // Main decal color (RGB hex)
decal.SecondaryColor = 0x000000; // Secondary color for patterns
decal.AccentColor = 0xFF0000;    // Optional accent color
```

### Pattern Properties

```csharp
decal.Scale = 1.0f;              // Pattern scale (0.1 - 10.0)
decal.Rotation = 45f;            // Rotation in degrees (0 - 360)
decal.Offset = new Vector2(0, 0);// Offset on block surface
decal.Opacity = 1.0f;            // Transparency (0.0 - 1.0)
```

### Face Targeting

```csharp
// Apply to specific faces
decal.TargetFace = BlockFace.Top;                    // Top face only
decal.TargetFace = BlockFace.Top | BlockFace.Right;  // Top and right
decal.TargetFace = BlockFace.All;                    // All faces

// Or use the flag
decal.ApplyToAllFaces = true;  // Apply to all faces
```

## Block Face Flags

- `BlockFace.None`: No faces
- `BlockFace.Top`: +Y face (top)
- `BlockFace.Bottom`: -Y face (bottom)
- `BlockFace.Front`: +Z face (front)
- `BlockFace.Back`: -Z face (back)
- `BlockFace.Right`: +X face (right)
- `BlockFace.Left`: -X face (left)
- `BlockFace.All`: All faces

## Procedural Generation Integration

The decal system is integrated into ship generation for certain faction styles:

### Industrial/Republic Faction Ships

Ships with "Industrial", "Republic", or "Thule" faction styles automatically get:
- Hazard stripes on wing blocks (every 3rd wing block)
- Red accent stripes on selected wings (every 7th wing block)
- Engine glow decals on all engine/thruster blocks

```csharp
var config = new ShipGenerationConfig
{
    Size = ShipSize.Frigate,
    Role = ShipRole.Combat,
    Material = "Titanium",
    Style = FactionShipStyle.GetDefaultStyle("Industrial")
};

var ship = generator.GenerateShip(config);
// Ship will have hazard stripes and accents applied automatically
```

### Speed-Focused Ships

Ships with `SpeedFocused` design philosophy get:
- Racing stripes along the top centerline
- Applied to every 5th top hull block

### All Ships

All ships automatically get:
- Engine glow decals on engine/thruster blocks
- Optional faction markings on side armor blocks (30% chance)

## Example: Applying Decals to Existing Ships

```csharp
// Find all wing blocks
var wingBlocks = ship.Structure.Blocks
    .Where(b => b.BlockType == BlockType.Hull)
    .Where(b => Math.Abs(b.Position.X) > 10) // Outer blocks
    .ToList();

// Apply hazard stripes to wings
foreach (var wing in wingBlocks)
{
    wing.AddDecal(DecalLibrary.HazardStripes());
}

// Add red accents to trailing edges
var trailingBlocks = ship.Structure.Blocks
    .Where(b => b.Position.Z < -5)
    .ToList();

foreach (var block in trailingBlocks.Where((_, i) => i % 3 == 0))
{
    block.AddDecal(DecalLibrary.RedAccentStripe());
}

// Add engine glow to all engines
var engines = ship.Structure.Blocks
    .Where(b => b.BlockType == BlockType.Engine)
    .ToList();

foreach (var engine in engines)
{
    engine.AddDecal(DecalLibrary.EngineGlowStripes());
}
```

## Example: Creating a Custom Industrial Look

```csharp
// Create a ship with industrial dark colors
var config = new ShipGenerationConfig
{
    Size = ShipSize.Corvette,
    Role = ShipRole.Combat,
    Material = "Titanium",
    Style = new FactionShipStyle
    {
        FactionName = "Custom Industrial Corp",
        PrimaryColor = 0x3A4A3A,   // Dark olive-green
        SecondaryColor = 0x4A3F30, // Dark brown
        AccentColor = 0x00CED1,    // Cyan
        PreferredHullShape = ShipHullShape.Angular,
        Philosophy = DesignPhilosophy.CombatFocused
    }
};

var ship = generator.GenerateShip(config);

// Manually add more hazard stripes
var hullBlocks = ship.Structure.Blocks
    .Where(b => b.BlockType == BlockType.Hull)
    .ToList();

foreach (var block in hullBlocks.Where((_, i) => i % 4 == 0))
{
    block.AddDecal(DecalLibrary.HazardStripes());
}
```

## Best Practices

1. **Use decals sparingly**: Too many decals can make ships look cluttered
2. **Match faction style**: Choose decals that fit your faction's aesthetic
3. **Layer decals**: You can apply multiple decals to the same block
4. **Consider block shapes**: Some patterns look better on flat surfaces
5. **Test different rotations**: Diagonal stripes (45°) often look more dynamic
6. **Use opacity**: Subtle decals (70-90% opacity) can add depth without overwhelming

## Reference: 1234.PNG Aesthetic

The decal system is designed to replicate the industrial ship aesthetic from the reference image:

- **Dark green/brown hulls**: Use Industrial faction style colors
- **Yellow/black hazard stripes**: Use `DecalLibrary.HazardStripes()`
- **Red accent stripes**: Use `DecalLibrary.RedAccentStripe()`
- **Cyan engine glow**: Use `DecalLibrary.EngineGlowStripes()`
- **Angular wings**: Built-in to Industrial faction ship generation

## Future Enhancements

Planned features for the decal system:
- Visual editor for applying decals in-game
- Custom decal texture uploads
- Animated decals (scrolling patterns, pulsing glows)
- Decal presets for quick application
- Decal symmetry tools for consistent application
- Decal export/import for sharing designs

## See Also

- `BlockDecal.cs` - Core decal implementation
- `DecalLibrary` - Predefined decal patterns
- `FactionShipStyle.cs` - Faction style definitions
- `ProceduralShipGenerator.cs` - Ship generation with decals
- `TEXTURE_CUSTOMIZATION_GUIDE.md` - Material and texture guide
