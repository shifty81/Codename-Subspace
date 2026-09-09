# Blender Addons for Codename: Subspace

This directory contains Blender addons designed to streamline asset creation and export for the Codename: Subspace game engine.

## Addons

### 1. NovaForge Asset Generator

A procedural asset generation tool with project profiles, materials, and animations.

**Location:** `NovaForge/`

**Features:**
- Project profile management with JSON-based configurations
- Procedural panel and door generators with customizable parameters
- Material presets (Metal, Wood, etc.) with diffuse and specular properties
- Animation presets (DoorOpen, PanelSlide, etc.) with keyframe generation
- Automatic FBX and JSON export for engine integration

**Installation:**
1. Open Blender
2. Go to Edit > Preferences > Add-ons
3. Click "Install" and select the `NovaForge/__init__.py` file
4. Enable the "NovaForge Asset Generator" addon
5. Find the panel in the 3D Viewport sidebar (press N) under "NovaForge" tab

**Usage:**
1. Create a new profile with custom defaults
2. Use procedural generators to create panels and doors
3. Customize materials and animations per profile
4. Export assets as FBX with accompanying JSON metadata

### 2. PCG Exporter

A comprehensive export system for procedural content generation with LOD, collision meshes, snap points, and metadata.

**Location:** `PCGExporter/`

**Features:**
- Automatic LOD (Level of Detail) generation with configurable levels
- Collision mesh export with triangle optimization
- Snap point JSON generation from object children
- Metadata export including bounding boxes, volume, and custom properties
- Warp mining zone support for space game mechanics
- Adjacency matrix export for structural intelligence learning
- Thumbnail generation for asset preview
- Project profile support with customizable export settings

**Installation:**
1. Open Blender
2. Go to Edit > Preferences > Add-ons
3. Click "Install" and select the `PCGExporter/__init__.py` file
4. Enable the "PCG Exporter" addon
5. Find the panel in the 3D Viewport sidebar (press N) under "PCG Exporter" tab

**Usage:**

#### Basic Export:
1. Prepare your scene with mesh objects
2. Set custom properties on objects (e.g., `type`, `category`, `collision`)
3. Open the PCG Exporter panel in the 3D Viewport sidebar
4. Click "Export Project for PCG"
5. Configure export settings and select output folder
6. Click OK to export

#### Custom Properties:
Add these custom properties to objects for enhanced functionality:

- `type` (string): Object type ("module", "planet", "asteroid", "ship", etc.)
- `category` (string): Asset category ("general", "weapon", "hull", etc.)
- `collision` (bool): Whether to export collision mesh
- `collision_mesh` (string): Name of collision mesh reference
- `material_tags` (list): Material tags for the object
- `unit_scale` (float): Scale multiplier for the object

#### Snap Points:
1. Create empty objects as children of your mesh
2. Set custom property `snap` to `True` on the empty
3. Add optional properties:
   - `snap_type` (string): Type of snap point
   - `compatible_tags` (list): Compatible connection types
   - `max_connections` (int): Maximum connections allowed
   - `strength` (float): Connection strength
   - `alignment_constraint` (string): Constraint type

#### Warp Mining Zones:
For space game asteroid belts and mining zones:
1. Set `warp_mining_zone` to `True` on object
2. Add custom properties:
   - `inner_radius` (float): Inner zone radius
   - `outer_radius` (float): Outer zone radius
   - `thickness` (float): Zone thickness
   - `density_seed` (int): Random seed for resource distribution
   - `resource_types` (list): Available resource types
   - `npc_spawn_weight` (float): NPC spawn probability

## Export Folder Structure

When you export a project, the following folder structure is created:

```
Export_Project/
├── Meshes/
│   ├── General/
│   ├── Planet/
│   ├── Ship/
│   └── Module/
├── LOD/
│   ├── General/
│   ├── Planet/
│   └── ...
├── Collision/
│   └── [type]/
├── SnapPoints/
│   └── [object_name]_snap.json
├── Metadata/
│   ├── [object_name]_metadata.json
│   ├── adjacency_matrix.json
│   └── warp_mining_zones.json
└── Thumbnails/
    └── [object_name]_thumb.png
```

## Integration with Codename: Subspace Engine

The exported assets are designed to integrate seamlessly with the C++/OpenGL engine:

1. **FBX Meshes**: Import using Assimp or FBX SDK
2. **JSON Metadata**: Parse for procedural parameters, snap points, and custom properties
3. **LOD System**: Load appropriate LOD level based on camera distance
4. **Collision Meshes**: Use for physics simulation
5. **Snap Points**: Use for modular construction and attachment systems
6. **Warp Mining Zones**: Generate dynamic resource fields in space

## Project Profiles

Project profiles are JSON files stored in `PCGExporter/templates/` that define export settings:

- Unit scale and LOD levels
- Material tags and snap rules
- Export settings (FBX scale, modifiers, triangulation)
- Collision and LOD generation parameters
- Thumbnail settings

You can create custom profiles for different projects or asset types.

## Requirements

- Blender 3.5 or higher
- Python 3.x (included with Blender)

## Development

Both addons are modular and extensible:

### NovaForge:
- Add new procedural generators by creating operator classes
- Extend material and animation presets in profiles
- Customize export formats by modifying export operators

### PCG Exporter:
- Add new utility functions in `utils/` modules
- Extend metadata generation in `metadata_utils.py`
- Create custom export operators for specific asset types
- Modify project profile templates for different workflows

## Troubleshooting

### Addon not appearing in Blender:
- Ensure you selected the `__init__.py` file during installation
- Check the Blender console for Python errors
- Verify Blender version compatibility (3.5+)

### Export fails:
- Check file permissions for export directory
- Ensure all objects have valid mesh data
- Verify custom properties are correctly typed (strings, floats, bools)

### LOD generation issues:
- Ensure objects have sufficient geometry for decimation
- Try reducing LOD levels if meshes are low-poly
- Check for non-manifold geometry

## Support

For issues, feature requests, or contributions, please visit the project repository.

## License

These addons are part of the Codename: Subspace project. See the main project LICENSE file for details.
