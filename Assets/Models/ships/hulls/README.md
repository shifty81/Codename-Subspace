# Ulysses Starter Ship

This directory contains documentation for the Ulysses starter ship 3D model.

**Note**: The actual Ulysses model is located at: `Assets/Models/ships/Ulysses/source/ulysses.blend`

## About the Ulysses

The Ulysses is the default starter ship for new players. It's a reliable, multipurpose corvette-class vessel suitable for:
- Exploration and scanning
- Light combat engagements
- Basic mining operations
- Small cargo hauling
- Learning the game mechanics

## Model Files

Expected files (actual location: `Assets/Models/ships/Ulysses/source/`):
- `ulysses.blend` - Blender source file (primary)
- `ulysses.obj` - Exported OBJ format (for compatibility)
- `ulysses.fbx` - Exported FBX format (alternative)
- `ulysses.gltf` - Exported GLTF format (alternative)

The game's AssetManager supports loading from Blender files directly using Assimp.NET, which can read .blend files.

## Model Specifications

The Ulysses should have:
- **Length**: ~15-20 meters
- **Design**: Balanced, industrial aesthetic
- **Hardpoints**: 
  - 2x forward-facing weapon mounts
  - 3x utility equipment slots (belly/sides)
  - Engine mount at rear
- **Cockpit**: Forward section with good visibility
- **Hull**: Medium armor plating
- **Engines**: Single main engine or dual side-mounted engines

## Integration

The model is loaded by:
1. `ShipTemplateManager.cs` - Defines the Ulysses template
2. `AssetManager.cs` - Loads the 3D model file
3. `MeshRenderer.cs` - Renders the ship in-game
4. `StarterShipFactory.cs` - Creates instances for new players

## Interior Layout

The Ulysses has a basic interior with:
- Cockpit (pilot seat with flight controls)
- Small cargo bay
- Equipment bay
- Engineering access
- Basic crew quarters (optional)

Players can customize the interior using the ship building system.

## Customization

Players can customize:
- Paint color scheme
- Equipment loadout
- Interior layout
- Module upgrades (when reaching higher tech levels)

## Default Loadout

Starting equipment:
- 2x Mk1 Pulse Lasers (primary weapons)
- 1x Mk1 Mining Laser (utility)
- Basic shield generator
- Basic power core
- Small cargo hold (50 units)

## File Format Support

The game supports loading from:
- `.blend` - Blender (native format, preferred)
- `.obj` - Wavefront OBJ (simple, widely supported)
- `.fbx` - Autodesk FBX (for complex animations)
- `.gltf` / `.glb` - GL Transmission Format (modern, efficient)
- `.dae` - Collada (XML-based)
- And 40+ other formats via Assimp.NET

## To Export from Blender

If you need to export the model:

1. **OBJ Export**:
   - File → Export → Wavefront (.obj)
   - Enable: Triangulate Faces, Write Normals, Include UVs
   - Scale: 1.0

2. **FBX Export**:
   - File → Export → FBX (.fbx)
   - Apply Scalings: FBX Units Scale
   - Include: Mesh, Normals, UVs

3. **GLTF Export**:
   - File → Export → glTF 2.0 (.gltf/.glb)
   - Format: Binary (.glb) for smaller size
   - Include: Normals, Textures, Materials

The .blend file can be used directly - no export needed!
