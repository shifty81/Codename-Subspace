# Ship Module 3D Models

This directory contains 3D model files for modular ship components.

## Directory Structure

```
GameData/Assets/Models/ships/modules/
├── README.md (this file)
├── cockpit_basic.obj
├── hull_section.obj
├── engine_main.obj
├── wing_left.obj
├── wing_right.obj
├── weapon_mount.obj
├── cargo_bay.obj
├── thruster.obj
├── power_core.obj
└── sensor_array.obj
```

## Current Models

### Core Hull Modules
- **cockpit_basic.obj** - Basic tapered cockpit shape
- **hull_section.obj** - Elongated hull section for connections

### Engine Modules
- **engine_main.obj** - Main engine with cylindrical shape
- **thruster.obj** - Small maneuvering thruster

### Wing Modules
- **wing_left.obj** - Left wing (tapered flat shape)
- **wing_right.obj** - Right wing (mirrored)

### Utility Modules
- **weapon_mount.obj** - Weapon/turret mounting point
- **cargo_bay.obj** - Large cargo storage bay
- **power_core.obj** - Power generator module
- **sensor_array.obj** - Sensor/radar array

## Model Format

All models are in Wavefront OBJ format (.obj), which is:
- Simple and widely supported
- Human-readable text format
- Easy to create and modify
- Compatible with Assimp.NET for loading

## Placeholder Models

The current models are simple geometric placeholders for testing the modular ship system. They provide:
- Basic shapes that represent each module type
- Proper dimensions and proportions
- Test geometry for the rendering system

## Adding New Models

To add new ship module models:

1. Create or obtain a 3D model in OBJ, FBX, GLTF, or other supported format
2. Place the file in this directory (or appropriate subdirectory)
3. Update the corresponding module definition in `ModuleLibrary.cs`
4. Set the `ModelPath` property to the relative path from `GameData/Assets/Models/`
   ```csharp
   ModelPath = "ships/modules/your_model.obj"
   ```

## Future Improvements

These placeholder models should eventually be replaced with:
- Detailed 3D models with proper mesh topology
- Multiple visual variants for each module type
- Different styles (Military, Industrial, Sleek, etc.)
- Texture maps for materials
- Normal maps for surface detail
- LOD (Level of Detail) variants for performance

## Model Guidelines

When creating replacement models:
- Keep polygon count reasonable (< 5000 triangles per module)
- Use consistent scale (1 unit = 1 meter)
- Center models at origin (0, 0, 0)
- Include proper normals for lighting
- Consider attachment points for module connections
- Create modular pieces that can be mixed and matched

## Supported Formats

The AssetManager and ModelLoader support:
- OBJ (Wavefront Object)
- FBX (Autodesk Filmbox)
- GLTF/GLB (GL Transmission Format)
- DAE (Collada)
- BLEND (Blender)
- 3DS (3D Studio)
- And 40+ other formats via Assimp.NET

## References

- [ModuleLibrary.cs](../../../../../../AvorionLike/Core/Modular/ModuleLibrary.cs)
- [ShipModuleDefinition.cs](../../../../../../AvorionLike/Core/Modular/ShipModuleDefinition.cs)
- [AssetManager.cs](../../../../../../AvorionLike/Core/Graphics/AssetManager.cs)
- [SHIP_GENERATION_NEXT_STEPS_IMPLEMENTATION.md](../../../../../../SHIP_GENERATION_NEXT_STEPS_IMPLEMENTATION.md)
