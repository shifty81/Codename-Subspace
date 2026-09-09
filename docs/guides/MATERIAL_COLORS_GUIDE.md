# Material Colors Guide

This guide shows the visual appearance of all materials in Codename:Subspace's 3D rendering system.

## Material Color Palette

All ships in the game now use vibrant, distinct colors to verify the rendering system is working correctly.

### Available Materials

| Material | Base Color | Visual Description | Properties |
|----------|-----------|-------------------|------------|
| **Iron** | Grey (0.65, 0.65, 0.65) | Medium grey, metallic | Metallic: 0.8, Roughness: 0.4 |
| **Titanium** | Silver-Blue (0.75, 0.80, 0.85) | Light silver with blue tint | Metallic: 0.9, Roughness: 0.3 |
| **Naonite** | Bright Green (0.2, 0.8, 0.3) | Vibrant green with glow | Metallic: 0.5, Emissive: Green glow |
| **Trinium** | Bright Blue (0.3, 0.6, 0.9) | Electric blue with glow | Metallic: 0.6, Emissive: Blue glow |
| **Xanion** | Gold/Yellow (0.9, 0.75, 0.2) | Rich gold with strong glow | Metallic: 0.95, Emissive: Yellow glow |
| **Ogonite** | Red/Orange (0.9, 0.35, 0.2) | Warm red-orange with glow | Metallic: 0.7, Emissive: Red glow |
| **Avorion** | Purple (0.75, 0.25, 0.9) | Deep purple with strong glow | Metallic: 0.85, Emissive: Purple glow |

## Ship Examples

### Player Ship (Option 1: NEW GAME)

When you start a new game, your starter ship features a rainbow of materials:

- **Core Hull**: Titanium (Silver-Blue, Metallic)
- **Main Engines**: Ogonite (Red/Orange with Glow) - clearly visible at the back
- **Maneuvering Thrusters**: Trinium (Bright Blue with Glow) - on top and bottom
- **Generator**: Xanion (Gold/Yellow with Strong Glow) - on the right side
- **Shield Generator**: Naonite (Bright Green with Glow) - front section
- **Gyro Arrays**: Avorion (Purple with Strong Glow) - angular momentum blocks

### Demo Ships (Option 11: 3D Graphics Demo)

Three demo ships are created with distinct color schemes:

1. **Fighter** (Left): 
   - Purple Avorion core
   - Red Ogonite engines
   - Gold Xanion wings

2. **Cross Ship** (Center):
   - Purple Avorion center
   - Gold Xanion generator (right)
   - Red Ogonite engine (left)
   - Green Naonite shields (top)
   - Blue Trinium thruster (bottom)

3. **Cargo Ship** (Right):
   - Gradient from Green Naonite → Blue Trinium → Gold Xanion

## Rendering Features

### PBR Materials

All materials use Physically Based Rendering (PBR) with:
- **Base Color**: The primary color of the material
- **Metallic**: How metallic the surface appears (0.0 = non-metal, 1.0 = full metal)
- **Roughness**: Surface smoothness (0.0 = mirror, 1.0 = rough)
- **Emissive Color**: Glow color for certain advanced materials
- **Emissive Strength**: How strong the glow effect is

### Lighting

The rendering system includes:
- **3 Light Sources**: Multiple directional lights for depth perception
- **Phong Lighting**: Ambient, diffuse, and specular lighting components
- **Ambient Light**: Slight blue tint (0.25, 0.25, 0.28) for space atmosphere

## Verification

To verify the material system is working correctly:

1. Run the game: `dotnet run`
2. Select **Option 1: NEW GAME** or **Option 11: 3D Graphics Demo**
3. Look for the distinct colors listed above
4. Use WASD + mouse to move around and view materials from different angles
5. Notice how emissive materials (Naonite, Trinium, Xanion, Ogonite, Avorion) glow

## Technical Details

Material definitions can be found in: `AvorionLike/Core/Graphics/Material.cs`

The renderer uses these materials in: `AvorionLike/Core/Graphics/EnhancedVoxelRenderer.cs`

Ship definitions with materials are in: `AvorionLike/Program.cs` (StartNewGame, GraphicsDemo, etc.)

---

**Note**: If you see all grey ships, the material system may not be working correctly. Please verify:
1. The `MaterialManager` is properly initialized
2. Material names match exactly (case-insensitive but spelling must match)
3. The shader is correctly setting material properties
