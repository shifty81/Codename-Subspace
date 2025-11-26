# Enhanced Texture Generation Guide

## Overview

The Enhanced Texture Generator creates procedurally generated textures with visual complexity including panel lines, greebling, wear patterns, and style-specific effects. This creates more interesting and varied ship surfaces compared to flat solid colors.

## Texture Styles

### Available Styles

| Style | Description | Key Features |
|-------|-------------|--------------|
| `Clean` | Factory-fresh appearance | Minimal wear, subtle variation |
| `Military` | Combat-ready look | Camo patterns, armor plates, warning marks |
| `Industrial` | Heavy-duty utilitarian | Dirt, oil stains, rust, exposed rivets |
| `Sleek` | High-tech aesthetic | Clean panels, glowing accents, minimal seams |
| `Pirate` | Cobbled-together look | Heavy wear, patches, mismatched panels |
| `Ancient` | Mysterious alien | Geometric patterns, glowing glyphs |
| `Organic` | Biomechanical | Vein patterns, pulsing effects |
| `Crystalline` | Crystal surfaces | Faceted appearance, prismatic colors |

## Visual Effects

### Panel Lines
Grid-based panel patterns create the appearance of hull plating. Panel size varies by style:
- **Sleek**: Large, clean panels (6 units)
- **Industrial**: Small, busy panels (3 units)
- **Military**: Medium panels (4 units)
- **Pirate**: Irregular small panels (2.5 units)

### Greebling
Multi-scale surface detail adds visual complexity:
- Multiple noise octaves at different scales
- Raised and recessed areas
- Vent/grille patterns for Industrial and Military styles

### Wear and Weathering
- **Edge Wear**: Brighter exposed edges
- **Scratches**: Long, thin scratch marks
- **Scorch Marks**: Dark burn patterns (Military, Pirate)
- **Rust/Corrosion**: Orange-brown rust stains (Industrial, Pirate)

### Emissive Accents
- **Glowing Lines**: Thin light strips (Sleek, Ancient, Organic)
- **Running Lights**: Small blinking indicators on hull surfaces
- **Pulsing Effects**: Time-based animation for organic feel

## Usage

### Basic Usage

```csharp
var texGen = new EnhancedTextureGenerator(seed: 12345);

// Generate color for a world position
Vector3 baseColor = new Vector3(0.6f, 0.6f, 0.65f);
Vector3 enhancedColor = texGen.GenerateEnhancedColor(
    worldPosition: new Vector3(10, 5, 20),
    baseColor: baseColor,
    style: EnhancedTextureGenerator.TextureStyle.Military,
    time: 0f  // For animated effects
);
```

### Per-Block Coloring

```csharp
// Apply to each block in a ship
foreach (var block in ship.Blocks)
{
    var color = texGen.GenerateEnhancedColor(
        block.Position,
        block.BaseColor,
        style,
        gameTime
    );
    block.RenderColor = color;
}
```

### Bump Mapping

```csharp
// Calculate bump/normal map value
float bumpValue = texGen.CalculateEnhancedBump(
    worldPosition,
    style
);
```

## Color Examples

Sample colors at position (10, 5, 20) with steel gray base (RGB 153, 153, 166):

| Style | Result RGB | Hex | Appearance |
|-------|------------|-----|------------|
| Clean | 114, 114, 124 | #72727C | Slightly darker, clean |
| Military | 105, 105, 113 | #696971 | Camo-tinted gray |
| Industrial | 85, 82, 83 | #555253 | Dirty, darkened |
| Sleek | 0, 164, 205 | #00A4CD | Cyan accent glow |
| Pirate | 145, 145, 154 | #91919A | Patched variation |
| Ancient | 137, 126, 104 | #897E68 | Gold-tinted |
| Organic | 130, 84, 112 | #825470 | Purple-tinted |
| Crystalline | 100, 130, 109 | #64826D | Green-shifted |

## Implementation Details

### Noise Functions

The generator uses several noise algorithms:

1. **3D Perlin Noise**: Smooth, continuous variation
2. **Voronoi Noise**: Cellular/crystal patterns
3. **High-Frequency Noise**: Fast, cheap fine detail

### Pattern Generation

Specialized pattern functions:
- `GeometricPattern`: Angular glyph patterns
- `VeinPattern`: Organic vein structures
- `CrystallinePattern`: Faceted crystal surfaces
- `VentPattern`: Grid of parallel slits
- `ScratchPattern`: Long thin scratches
- `RustPattern`: Corrosion distribution

## Integration with Modular Ships

The texture system integrates with modular ship generation:

```csharp
// Generate ship with modular system
var ship = generator.GenerateModularShip("fighter", ModuleStyle.Military);

// Match texture style to module style
var texStyle = moduleStyle switch
{
    ModuleStyle.Military => EnhancedTextureGenerator.TextureStyle.Military,
    ModuleStyle.Industrial => EnhancedTextureGenerator.TextureStyle.Industrial,
    ModuleStyle.Sleek => EnhancedTextureGenerator.TextureStyle.Sleek,
    ModuleStyle.Pirate => EnhancedTextureGenerator.TextureStyle.Pirate,
    ModuleStyle.Ancient => EnhancedTextureGenerator.TextureStyle.Ancient,
    _ => EnhancedTextureGenerator.TextureStyle.Clean
};

// Apply textures to ship blocks
foreach (var block in ship.Blocks)
{
    block.Color = texGen.GenerateEnhancedColor(block.Position, baseColor, texStyle);
}
```

## Performance Considerations

- **CPU-Based**: Calculations run on CPU, suitable for pre-generation
- **Deterministic**: Same seed + position = same result
- **Cacheable**: Pre-generate textures during ship creation
- **LOD Friendly**: Can skip detail at distance

## Future Enhancements

Potential additions:
1. GPU shader implementation for real-time rendering
2. Normal map generation from bump values
3. Additional style presets
4. User-customizable styles
5. Texture atlas generation
