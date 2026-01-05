# Ulysses Starter Ship - Source Files

This directory contains the source Blender file(s) for the Ulysses starter ship.

## File Placement

Place the Ulysses model files here:
- `ulysses.blend` - Main Blender source file
- `ulysses.obj` - Exported OBJ (if available)
- `ulysses.fbx` - Exported FBX (if available)
- `ulysses.gltf` / `ulysses.glb` - Exported glTF (if available)

## Automatic Loading

The game will automatically detect and load the Ulysses model from this location:
1. Checks for `ulysses.blend` first (Blender format, loaded via Assimp)
2. Falls back to `.obj`, `.fbx`, `.gltf`, or `.glb` formats
3. If no model found, generates a procedural ship

## Model Requirements

### Scale & Orientation
- **Scale**: 1 Blender unit = 1 meter in-game
- **Length**: ~15-20 meters (corvette-class)
- **Origin**: Ship center at (0, 0, 0)
- **Forward**: Positive Z-axis (+Z points forward)
- **Up**: Positive Y-axis (+Y points up)
- **Right**: Positive X-axis (+X points right)

### Recommended Geometry
- **Polygon Count**: 500-5000 triangles (optimized for real-time)
- **Normals**: Include normals for proper lighting
- **UVs**: Include UV coordinates if using textures

### Ship Features
The Ulysses should include:
- **Cockpit**: Forward section with visible canopy/windows
- **Hull**: Main body with panel details
- **Engines**: Rear thruster section
- **Wings/Stabilizers**: Optional but recommended
- **Equipment Hardpoints**: Visual locations for weapons/tools (not required)

### Visual Style
Follow the "Balanced" design philosophy:
- Industrial/functional aesthetic (like X4's Argon ships)
- Angular, practical design
- Medium detail level
- Visible panel lines and surface details
- Cohesive corvette-class appearance

## Blender Tips

### Before Export
1. **Apply Transformations**: Ctrl+A → All Transforms
2. **Correct Orientation**: Ensure ship faces +Z
3. **Scale Check**: Measure length should be 15-20 units
4. **Clean Mesh**: Remove doubles, fix normals
5. **Triangulate** (if exporting non-.blend): Triangulate modifier

### Blender to Game
The game uses Assimp to load `.blend` files directly:
- **No export needed** for .blend format
- Assimp reads Blender meshes, materials, and UV data
- Most Blender features are supported

### Optional Export Formats
If you want to provide fallback formats:

**OBJ Export** (simple, widely compatible):
- File → Export → Wavefront (.obj)
- Options: Triangulate Faces ✓, Write Normals ✓, Include UVs ✓
- Forward: Z Forward, Up: Y Up

**FBX Export** (for animations/complex data):
- File → Export → FBX (.fbx)
- Apply Scalings: FBX Units Scale
- Forward: Z Forward, Up: Y Up

**glTF Export** (modern, efficient):
- File → Export → glTF 2.0 (.gltf/.glb)
- Format: Binary (.glb) for smaller size
- Include: Normals ✓, Textures ✓, Materials ✓

## Materials & Textures

### Material Setup (Optional)
The game supports basic PBR materials:
- **Base Color**: RGB color or texture
- **Metallic**: 0.0-1.0 (0.7 recommended for metal hull)
- **Roughness**: 0.0-1.0 (0.5 recommended)
- **Emissive**: For glowing parts (engines, lights)

### Paint System
The game applies paint colors dynamically:
- Your material colors will be used as defaults
- Players can customize via the paint system
- Multiple color channels: Primary, Secondary, Accent, Glow

## Testing Your Model

1. Place model file in this directory
2. Build and run the game:
   ```bash
   dotnet build AvorionLike/AvorionLike.csproj
   dotnet run --project AvorionLike/AvorionLike.csproj
   ```
3. Create a new game or load save
4. Check console for "UlyssesLoader" messages
5. If loaded successfully, you'll see:
   ```
   [UlyssesLoader] Found Ulysses model: Models/ships/Ulysses/source/ulysses.blend (Blender)
   [UlyssesLoader] Successfully loaded Ulysses model
   [UlyssesLoader] Model contains X mesh(es): ...
   ```

## Troubleshooting

### Model Not Found
- Check filename is exactly `ulysses.blend` (lowercase)
- Verify file is in `Assets/Models/ships/Ulysses/source/`
- Check console output for search paths tried

### Model Loads But Looks Wrong
- Check orientation (Forward = +Z, Up = +Y)
- Verify scale (15-20 meters length)
- Apply all transforms in Blender before saving
- Check normals are correct (not inverted)

### Model Won't Load
- Verify file isn't corrupted (open in Blender)
- Check file permissions
- Try exporting to `.obj` as fallback
- Check console for error messages

## Equipment Integration

The ship will automatically get equipment slots:
- 2x Primary Weapon slots (forward guns)
- 3x Utility slots (mining laser, scanner, etc.)
- Default loadout: 2x Pulse Lasers, 1x Mining Laser

Equipment is positioned automatically and doesn't require hardpoints in the model.

## Interior System

If implementing interior (optional):
- Create interior spaces inside the hull
- Follow game's interior cell generation
- Or leave empty for procedural interior generation

## Credits & License

If using a custom Ulysses model:
- Document your authorship
- Specify license/usage rights
- Include any attribution required

---

**Directory**: `Assets/Models/ships/Ulysses/source/`  
**Last Updated**: January 2026  
**Game Version**: X4 System v1.0
