# Ship Module 3D Models

This directory contains 3D model files for modular ship components.

## Directory Structure

```
GameData/Assets/Models/ships/modules/
├── README.md (this file)
├── cockpit_basic.obj          - Detailed cockpit with canopy
├── hull_section.obj           - Hull connector with panels
├── engine_main.obj            - Main engine with nozzles and fins
├── wing_left.obj              - Left wing with aerodynamic shape
├── wing_right.obj             - Right wing (mirrored)
├── weapon_mount.obj           - Turret base with rotation details
├── cargo_bay.obj              - Cargo container with doors
├── thruster.obj               - Maneuvering thruster with vanes
├── power_core.obj             - Reactor with cooling coils
└── sensor_array.obj           - Parabolic sensor dish
```

## Current Models - Enhanced Detail (Updated January 2026)

All models have been significantly enhanced with detailed geometry:

### Core Hull Modules
- **cockpit_basic.obj** - Streamlined cockpit with:
  - Tapered nose cone
  - Canopy windows
  - Multi-section design (20 vertices, 31 faces)
  
- **hull_section.obj** - Modular hull connector with:
  - Recessed panel details
  - Connection ports front/back
  - Panel line details (36 vertices, 26 faces)

### Engine Modules
- **engine_main.obj** - Detailed main engine with:
  - Octagonal housing (cylindrical appearance)
  - Flared nozzle with inner exhaust port
  - 4 cooling fins for heat dissipation (40 vertices, 66 faces)
  
- **thruster.obj** - Maneuvering thruster with:
  - Hexagonal housing
  - Tapered nozzle
  - 4 thrust vectoring vanes (28 vertices, 44 faces)

### Wing Modules
- **wing_left.obj** - Aerodynamic left wing with:
  - Multi-segment tapered design
  - Panel detail sections
  - Leading edge definition (27 vertices, 31 faces)
  
- **wing_right.obj** - Aerodynamic right wing (mirrored)
  - Same features as left wing
  - Properly mirrored geometry (27 vertices, 31 faces)

### Utility Modules
- **weapon_mount.obj** - Turret mounting system with:
  - Base mounting plate
  - Layered turret structure
  - Rotation mechanism details
  - Barrel mounting point (28 vertices, 48 faces)
  
- **cargo_bay.obj** - Storage container with:
  - Large cargo hold structure
  - Access door panels
  - Top hatch
  - Structural reinforcement bars (36 vertices, 30 faces)
  
- **power_core.obj** - Power generation module with:
  - Core reactor chamber
  - Octagonal inner core
  - Wrapping cooling coils
  - Energy port connectors (36 vertices, 48 faces)
  
- **sensor_array.obj** - Sensor/radar system with:
  - Parabolic dish (octagonal approximation)
  - Central antenna feed
  - Support struts
  - Mounting arm (37 vertices, 56 faces)

## Model Format

All models are in Wavefront OBJ format (.obj), which is:
- Simple and widely supported
- Human-readable text format
- Easy to create and modify
- Compatible with Assimp.NET for loading

## Performance Characteristics

The enhanced models maintain excellent performance:
- **Vertex Range:** 20-40 vertices per module (2.5-5x more than placeholders)
- **Face Range:** 26-66 faces per module (2-5.5x more than placeholders)
- **Polygon Budget:** All under 100 faces, well within real-time rendering limits
- **Memory Impact:** Minimal - total ~315 vertices across all 10 modules
- **Rendering:** Can easily render hundreds of ships without performance impact

## Visual Improvements

Compared to the original 8-vertex placeholder models:

| Module | Old Vertices | New Vertices | Detail Increase |
|--------|--------------|--------------|-----------------|
| Cockpit | 8 | 20 | 2.5x |
| Hull Section | 8 | 36 | 4.5x |
| Engine Main | 8 | 40 | 5.0x |
| Thruster | 8 | 28 | 3.5x |
| Wings (each) | 8 | 27 | 3.4x |
| Weapon Mount | 8 | 28 | 3.5x |
| Cargo Bay | 8 | 36 | 4.5x |
| Power Core | 8 | 36 | 4.5x |
| Sensor Array | 8 | 37 | 4.6x |

**Total geometric detail increase:** ~4x average across all modules

## Integration with Game Systems

These models integrate seamlessly with:
- **ModuleLibrary.cs** - Module definitions reference these files via `ModelPath` property
- **AssetManager.cs** - Loads and caches models for efficient rendering
- **MeshRenderer.cs** - Renders models in 3D with lighting and materials
- **ModularShipComponent.cs** - Assembles ships from these modular parts

## Adding New Models

To add new ship module models:

1. Create or obtain a 3D model in OBJ, FBX, GLTF, or other supported format
2. Place the file in this directory (or appropriate subdirectory)
3. Update the corresponding module definition in `ModuleLibrary.cs`
4. Set the `ModelPath` property to the relative path from `Assets/Models/`
   ```csharp
   ModelPath = "ships/modules/your_model.obj"
   ```

## Future Improvements

These enhanced models provide a strong foundation, and can be further improved with:
- Texture maps for surface details (diffuse, normal, specular)
- Multiple visual variants for each module type (military, industrial, sleek)
- Different size variants (small, medium, large)
- LOD (Level of Detail) variants for distant rendering
- Animations (rotating turrets, opening cargo doors)
- Emissive maps for glowing engine effects

## Model Guidelines

When creating or replacing models:
- Keep polygon count reasonable (< 500 triangles per module for complex versions)
- Use consistent scale (1 unit = 1 meter)
- Center models at origin (0, 0, 0)
- Include proper normals for lighting
- Consider attachment points for module connections
- Create modular pieces that can be mixed and matched
- Follow the same visual style for consistency

## Supported Formats

The AssetManager and ModelLoader support:
- OBJ (Wavefront Object) - Current format
- FBX (Autodesk Filmbox)
- GLTF/GLB (GL Transmission Format)
- DAE (Collada)
- BLEND (Blender)
- 3DS (3D Studio)
- And 40+ other formats via Assimp.NET

## Testing

To test these models in-game:
1. Build the project: `dotnet build AvorionLike/AvorionLike.csproj`
2. Run the game: `dotnet run --project AvorionLike/AvorionLike.csproj`
3. Ships generated using the modular system will use these enhanced models

## References

- [ModuleLibrary.cs](../../../../../../AvorionLike/Core/Modular/ModuleLibrary.cs)
- [ShipModuleDefinition.cs](../../../../../../AvorionLike/Core/Modular/ShipModuleDefinition.cs)
- [AssetManager.cs](../../../../../../AvorionLike/Core/Graphics/AssetManager.cs)
- [MeshRenderer.cs](../../../../../../AvorionLike/Core/Graphics/MeshRenderer.cs)
- [MODULAR_SHIP_3D_MODELS_IMPLEMENTATION.md](../../../../../../MODULAR_SHIP_3D_MODELS_IMPLEMENTATION.md)

---

**Last Updated:** January 4, 2026  
**Version:** Enhanced Detail Models v1.0  
**Status:** Production Ready
