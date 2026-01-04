# Modular Ship 3D Models Implementation

## Overview

This document describes the implementation of 3D model support for the modular ship generation system, continuing the work from PR #122.

## What Was Implemented

### 1. Directory Structure
Created `GameData/Assets/Models/ships/modules/` directory to store 3D model files.

### 2. Placeholder 3D Models
Added 10 placeholder OBJ files for different module types:

| Model File | Module Type | Description |
|------------|-------------|-------------|
| `cockpit_basic.obj` | Hull/Cockpit | Tapered cockpit shape |
| `hull_section.obj` | Hull/Section | Elongated hull connector |
| `engine_main.obj` | Engine/Main | Cylindrical main engine |
| `wing_left.obj` | Wing | Left wing section |
| `wing_right.obj` | Wing | Right wing section (mirrored) |
| `weapon_mount.obj` | Weapon Mount | Weapon hardpoint |
| `cargo_bay.obj` | Cargo | Large cargo storage |
| `thruster.obj` | Thruster | Small maneuvering thruster |
| `power_core.obj` | Power Core | Power generator |
| `sensor_array.obj` | Sensor | Sensor/radar array |

### 3. Updated ModuleLibrary
Updated the following methods in `ModuleLibrary.cs` to include `ModelPath` properties:

- `CreateCockpitModule()` → `"ships/modules/cockpit_basic.obj"`
- `CreateHullSectionModule()` → `"ships/modules/hull_section.obj"`
- `CreateMainEngineModule()` → `"ships/modules/engine_main.obj"`
- `CreateThrusterModule()` → `"ships/modules/thruster.obj"`
- `CreateWingModule()` → `"ships/modules/wing_left.obj"`
- `CreateWeaponMountModule()` → `"ships/modules/weapon_mount.obj"`
- `CreatePowerCoreModule()` → `"ships/modules/power_core.obj"`
- `CreateCargoModule()` → `"ships/modules/cargo_bay.obj"`
- `CreateSensorModule()` → `"ships/modules/sensor_array.obj"`

## Model Format

All models use the Wavefront OBJ format (.obj), which:
- Is simple and human-readable
- Is widely supported across 3D tools
- Works with Assimp.NET for loading
- Can be easily created or modified

## How It Works

1. **Model Loading**: When a ship is generated, the `ModularProceduralShipGenerator` creates instances of modules
2. **Path Resolution**: Each module has a `ModelPath` that points to its 3D model file
3. **Rendering**: The `GraphicsWindow` uses `AssetManager` to load the model via `ModelLoader`
4. **Fallback**: If a model file is missing, the system uses a placeholder cube (as implemented in PR #122)

## Testing

### Building
```bash
dotnet build AvorionLike/AvorionLike.csproj
```
Build Status: ✅ Success (2 pre-existing warnings, 0 errors)

### Running
To see modular ships with 3D models:
```bash
dotnet run
# Select option for modular ship generation or gameplay
```

## Integration with Existing Systems

### AssetManager (from PR #122)
- Loads 3D models from the `GameData/Assets/Models/` directory
- Caches models for performance
- Handles multiple file formats via Assimp.NET

### GraphicsWindow (from PR #122)
- Renders modules using `MeshRenderer`
- Applies material-based coloring
- Handles module transformations (position, rotation)

### ModularShipComponent
- Stores module instances with their definitions
- Maintains module positions and rotations
- Integrates with voxel damage system

## Placeholder Model Details

The current models are simple geometric shapes for testing:
- **Vertices**: 8 per model (box-based geometry)
- **Faces**: 12 triangles per model
- **Format**: Indexed triangle lists
- **Centered**: At origin (0, 0, 0)
- **Scale**: Matches module `Size` property

## Next Steps

### Immediate
1. ✅ Create model directory structure
2. ✅ Add placeholder OBJ files
3. ✅ Update ModuleLibrary with model paths
4. ✅ Build and verify compilation
5. ⏳ Test rendering in-game
6. ⏳ Verify model loading and fallback

### Short-term
1. Create more detailed placeholder models
2. Add multiple variants per module type
3. Implement style-specific models (Military, Industrial, etc.)
4. Add texture coordinates to models

### Long-term
1. Replace placeholders with professional 3D models
2. Add multiple LOD variants
3. Implement texture mapping
4. Add normal maps for detail
5. Create animation support for moving parts

## Model Creation Guidelines

For future model additions:
- **Polygon Count**: Keep under 5,000 triangles per module
- **Scale**: 1 unit = 1 meter
- **Origin**: Center models at (0, 0, 0)
- **Normals**: Include proper face normals
- **Format**: OBJ, FBX, or GLTF recommended
- **Naming**: Use descriptive names matching module IDs

## File References

### New Files
- `GameData/Assets/Models/ships/modules/*.obj` (10 model files)
- `GameData/Assets/Models/ships/modules/README.md`
- `MODULAR_SHIP_3D_MODELS_IMPLEMENTATION.md` (this file)

### Modified Files
- `AvorionLike/Core/Modular/ModuleLibrary.cs`

### Related Files (from PR #122)
- `AvorionLike/Core/Graphics/AssetManager.cs`
- `AvorionLike/Core/Graphics/ModelLoader.cs`
- `AvorionLike/Core/Graphics/MeshData.cs`
- `AvorionLike/Core/Graphics/MeshRenderer.cs`
- `AvorionLike/Core/Graphics/GraphicsWindow.cs`

## Troubleshooting

### Models Not Loading
1. Check that `ModelPath` is correct in module definition
2. Verify OBJ file exists in `GameData/Assets/Models/ships/modules/`
3. Check console logs for Assimp.NET errors
4. Ensure file permissions allow reading

### Rendering Issues
1. Verify model has valid geometry (vertices and faces)
2. Check that normals are properly defined
3. Ensure model scale matches module size
4. Verify Assimp.NET dependency is installed

### Performance Issues
1. Check polygon count (should be < 5,000)
2. Enable model caching in AssetManager
3. Consider using LOD variants
4. Profile with GraphicsWindow debugging

## Summary

This implementation completes the "next steps" from PR #122 by:
1. ✅ Creating the model directory structure
2. ✅ Adding placeholder 3D model files
3. ✅ Updating ModuleLibrary with model references
4. ✅ Documenting the system for future development

The modular ship system now has all the infrastructure needed to render ships with actual 3D models instead of voxel cubes. The placeholder models provide a foundation for testing and can be easily replaced with professional assets.
