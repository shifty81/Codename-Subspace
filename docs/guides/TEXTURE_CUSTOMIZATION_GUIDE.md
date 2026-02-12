# Texture Customization Guide

## Overview

This guide explains how to customize the procedural textures used for blocks in Codename-Subspace. The texture system uses procedural generation to create varied, interesting surfaces without requiring external texture files.

## Quick Start: Changing Material Colors

All material definitions are in `AvorionLike/Core/Graphics/TextureMaterial.cs`.

### Example: Customize Hull Color

```csharp
_materials[MaterialType.Hull] = new TextureMaterial
{
    Type = MaterialType.Hull,
    Name = "Hull Plating",
    BaseColor = new Vector3(0.55f, 0.58f, 0.62f),      // Main color (RGB 0-1)
    SecondaryColor = new Vector3(0.35f, 0.38f, 0.42f), // Accent color
    Roughness = 0.45f,        // 0 = mirror, 1 = matte
    Metallic = 0.85f,         // 0 = plastic, 1 = metal
    NoiseScale = 3.0f,        // Size of surface detail
    NoiseStrength = 0.25f,    // Amount of variation
    BumpStrength = 0.4f,      // Depth of surface features
    Pattern = TexturePattern.Paneled,  // Pattern type
    PatternScale = 3.5f       // Pattern size
};
```

### Color Examples

```csharp
// Red hull
BaseColor = new Vector3(0.8f, 0.2f, 0.2f),

// Blue hull  
BaseColor = new Vector3(0.2f, 0.4f, 0.9f),

// Green hull
BaseColor = new Vector3(0.2f, 0.8f, 0.3f),

// Gold hull
BaseColor = new Vector3(1.0f, 0.82f, 0.15f),
```

## Available Texture Patterns

Change the `Pattern` property to any of these:

1. **Uniform** - Solid color with noise variation
2. **Striped** - Horizontal or vertical stripes
3. **Banded** - Concentric bands (good for planets)
4. **Paneled** - Hull panels with rivets and seams (default for ships)
5. **Hexagonal** - Honeycomb pattern (good for armor)
6. **Cracked** - Cracked/fractured surface (good for asteroids)
7. **Crystalline** - Crystal structure
8. **Swirled** - Turbulent swirl (good for gas clouds)
9. **Spotted** - Random spots/patches (good for organic)
10. **Weathered** - Wear and tear effects

### Example: Change Armor to Paneled

```csharp
_materials[MaterialType.Armor] = new TextureMaterial
{
    // ... other properties ...
    Pattern = TexturePattern.Paneled,  // Changed from Hexagonal
    PatternScale = 2.0f
};
```

## Material Properties Explained

### Visual Properties

- **BaseColor**: Main RGB color (0.0 to 1.0 for each channel)
- **SecondaryColor**: Accent color for patterns
- **Roughness**: Surface smoothness (0 = mirror-like, 1 = completely matte)
- **Metallic**: How metallic the surface appears (0 = non-metallic, 1 = full metal)
- **Emissive**: Self-illumination amount (only for glowing materials)

### Pattern Properties

- **Pattern**: Which procedural pattern to use (see list above)
- **PatternScale**: How large the pattern features are (smaller = more detail)
- **NoiseScale**: Size of noise/detail overlay
- **NoiseStrength**: How much the noise affects the color (0 = none, 1 = maximum)
- **BumpStrength**: Depth of surface relief/bumps

## Creating New Materials

To add a completely new material type:

1. **Add to MaterialType enum** in `TextureMaterial.cs`:
```csharp
public enum MaterialType
{
    // ... existing types ...
    MyCustomMaterial  // Add your new type
}
```

2. **Define the material** in `MaterialLibrary.InitializeMaterials()`:
```csharp
_materials[MaterialType.MyCustomMaterial] = new TextureMaterial
{
    Type = MaterialType.MyCustomMaterial,
    Name = "My Custom Material",
    BaseColor = new Vector3(1.0f, 0.5f, 0.0f),  // Orange
    SecondaryColor = new Vector3(0.8f, 0.3f, 0.0f),
    Roughness = 0.5f,
    Metallic = 0.7f,
    Pattern = TexturePattern.Paneled,
    PatternScale = 4.0f
};
```

3. **Use it in your code**:
```csharp
var material = MaterialLibrary.GetMaterial(MaterialType.MyCustomMaterial);
```

## Advanced: Modifying Pattern Algorithms

Pattern generation algorithms are in `AvorionLike/Core/Graphics/ProceduralTextureGenerator.cs`.

### Example: Modify Hexagonal Pattern

```csharp
private float HexagonalPattern(Vector3 pos, float scale)
{
    float x = pos.X * scale;
    float y = pos.Y * scale;
    
    // Create hexagonal grid
    float hex1 = MathF.Sin(x);
    float hex2 = MathF.Sin(x * 0.5f + y * 0.866f);
    float hex3 = MathF.Sin(x * 0.5f - y * 0.866f);
    
    float hexPattern = (hex1 + hex2 + hex3) / 3.0f;
    
    // Customize: Change these thresholds for different effects
    float cellBorder = MathF.Abs(hexPattern);
    if (cellBorder > 0.7f)
    {
        return -0.5f; // Darker borders
    }
    else
    {
        return 0.1f;  // Lighter cells
    }
}
```

## Testing Your Changes

1. **Build the project**:
```bash
cd AvorionLike
dotnet build
```

2. **Run the game**:
```bash
dotnet run
```

3. **Select Option 1 or 2** from the menu to see ships/structures with your new textures

## Tips for Great Textures

1. **Use contrasting colors**: Make BaseColor and SecondaryColor noticeably different
2. **Balance metallic and roughness**: Shiny metals look best with low roughness (0.1-0.3)
3. **Don't overdo noise**: NoiseStrength of 0.15-0.3 is usually enough
4. **Scale matters**: PatternScale of 2-4 works well for most ship-sized objects
5. **Test different patterns**: Try multiple pattern types to see what looks best

## Troubleshooting

### Textures look too plain
- Increase `NoiseStrength` (try 0.3-0.4)
- Decrease `PatternScale` (try 2.0-3.0 for more detail)
- Choose a more complex pattern (Hexagonal, Paneled, Cracked)

### Textures are too busy
- Decrease `NoiseStrength` (try 0.1-0.2)
- Increase `PatternScale` (try 5.0-8.0 for larger features)
- Use simpler patterns (Uniform, Striped)

### Colors look washed out
- Increase color intensity: use higher values (0.7-1.0)
- Increase `Metallic` property (try 0.8-0.95)
- Add emissive glow for special materials

### Blocks still look hollow
- The issue is fixed by default with two-sided rendering enabled
- Press F7 to toggle two-sided rendering if needed
- Check that `DebugConfig.TwoSidedRendering` is set to `true`

## Default Material Colors

Current defaults after enhancement:

- **Hull**: Steel blue-gray, paneled pattern
- **Armor**: Slate blue-gray, hexagonal honeycomb pattern
- **Rock**: Gray with brown tint, cracked pattern
- **Ice**: Light blue-white, cracked pattern
- **Metal**: Silver-gray, paneled pattern

## Related Files

- `AvorionLike/Core/Graphics/TextureMaterial.cs` - Material definitions
- `AvorionLike/Core/Graphics/ProceduralTextureGenerator.cs` - Pattern algorithms
- `AvorionLike/Core/Graphics/Material.cs` - Material rendering properties
- `AvorionLike/Core/Config/DebugConfig.cs` - Rendering debug options

## Further Reading

- See `ENHANCED_TEXTURE_GUIDE.md` for advanced procedural generation
- See `MATERIAL_COLORS_GUIDE.md` for color theory and material properties
- See `SHIP_GENERATION_TEXTURE_GUIDE.md` for ship-specific texture application
