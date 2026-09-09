# 3D Graphics Rendering System

## Overview

The AvorionLike engine now includes a complete 3D graphics rendering system that allows you to visualize voxel ships in real-time! This major addition transforms the text-only console application into an interactive 3D visualization tool.

## Features

### Real-Time 3D Rendering
- OpenGL-based rendering using Silk.NET
- Cross-platform support (Windows, Linux, macOS)
- Real-time visualization of all entities with voxel structures
- Smooth 60 FPS rendering with VSync

### Camera System
- **Free-look camera** with full 6 degrees of freedom
- **WASD controls** for movement (forward, back, left, right)
- **Space/Shift** for vertical movement (up/down)
- **Mouse look** for rotating the view
- Adjustable movement speed and mouse sensitivity

### Lighting & Materials
- **Phong lighting model** with ambient, diffuse, and specular components
- **Material-based coloring** for different block types:
  - Iron: Gray (#B3B3B3)
  - Titanium: Light Blue (#CCE5FF)
  - Naonite: Green (#33CC4D)
  - Trinium: Blue (#4D99E5)
  - Xanion: Gold (#E5B333)
  - Ogonite: Red (#E54D4D)
  - Avorion: Purple (#CC33E5)

### Integration with ECS
- Seamless integration with the Entity-Component System
- Automatically renders all entities with `VoxelStructureComponent`
- Uses `PhysicsComponent` for entity positioning
- Updates in real-time as entities move

## How to Use

### Accessing the 3D Graphics Window

1. Run the AvorionLike application
2. Select **option 10** from the main menu: "3D Graphics Demo - Visualize Voxel Ships [NEW]"
3. If no entities exist, the system automatically creates three demo ships with different designs
4. A new window will open showing your voxel structures in 3D

### Controls

| Key | Action |
|-----|--------|
| **W** | Move camera forward |
| **S** | Move camera backward |
| **A** | Move camera left |
| **D** | Move camera right |
| **Space** | Move camera up |
| **Shift** | Move camera down |
| **Mouse** | Look around (free look) |
| **ESC** | Close window and return to console |

### Demo Ships

When you first launch the graphics demo without existing entities, three sample ships are created:

1. **Demo Ship 1** - A simple linear ship made of Iron blocks
2. **Demo Ship 2** - A cross-shaped ship with mixed materials (Titanium, Iron, Naonite)
3. **Demo Ship 3** - A compact cubic ship with Trinium and Xanion blocks

Each ship is positioned at a different location for easy viewing and comparison.

## Technical Details

### Architecture

The graphics system consists of four main components:

#### 1. Camera (`Camera.cs`)
- Manages 3D camera position and orientation
- Handles keyboard and mouse input for movement
- Calculates view and projection matrices for rendering
- Configurable FOV, movement speed, and mouse sensitivity

#### 2. Shader (`Shader.cs`)
- Wraps OpenGL shader programs
- Compiles vertex and fragment shaders
- Manages uniform variables for lighting and transformations
- Provides methods for setting matrices and vectors

#### 3. VoxelRenderer (`VoxelRenderer.cs`)
- Generates cube meshes for voxel blocks
- Applies materials and colors based on block type
- Implements Phong lighting calculations
- Renders all blocks in a voxel structure efficiently

#### 4. GraphicsWindow (`GraphicsWindow.cs`)
- Creates and manages the OpenGL window
- Handles input context and event callbacks
- Manages the rendering loop
- Coordinates between engine updates and rendering

### Shader Implementation

The system uses GLSL shaders (OpenGL Shading Language) version 3.3 core:

**Vertex Shader:**
- Transforms vertices from model space to screen space
- Calculates fragment positions and normals in world space
- Applies model, view, and projection matrices

**Fragment Shader:**
- Implements Phong lighting model
- Calculates ambient, diffuse, and specular components
- Combines lighting with material color
- Outputs final pixel color

### Performance

- **Rendering**: Approximately 60 FPS on modern hardware
- **Voxel Capacity**: Can render hundreds of voxel blocks smoothly
- **Memory Usage**: Minimal overhead, reuses cube mesh for all blocks
- **Updates**: Integrates with engine update loop for synchronized gameplay

## Example Code

### Creating and Rendering a Custom Ship

```csharp
// Create an entity with voxel structure
var ship = engine.EntityManager.CreateEntity("Custom Ship");

// Add voxel blocks
var voxelComponent = new VoxelStructureComponent();
voxelComponent.AddBlock(new VoxelBlock(
    new Vector3(0, 0, 0),
    new Vector3(3, 3, 3),
    "Titanium"
));
voxelComponent.AddBlock(new VoxelBlock(
    new Vector3(4, 0, 0),
    new Vector3(2, 2, 2),
    "Naonite"
));

engine.EntityManager.AddComponent(ship.Id, voxelComponent);

// Add physics for positioning
var physicsComponent = new PhysicsComponent
{
    Position = new Vector3(0, 0, 0),
    Mass = voxelComponent.TotalMass
};
engine.EntityManager.AddComponent(ship.Id, physicsComponent);

// Now launch the graphics demo (option 10) to see your ship in 3D!
```

### Opening the Graphics Window Programmatically

```csharp
using var graphicsWindow = new GraphicsWindow(gameEngine);
graphicsWindow.Run(); // Blocks until window is closed
```

## Requirements

### Runtime Requirements
- .NET 9.0 or later
- OpenGL 3.3 compatible graphics card
- Operating System: Windows, Linux, or macOS

### Build Requirements
- Silk.NET v2.21.0 (automatically installed via NuGet)
  - Silk.NET.Windowing
  - Silk.NET.OpenGL
  - Silk.NET.Input
  - Silk.NET.Maths

## Future Enhancements

Potential improvements for the graphics system:

- [ ] **Textures** - Add texture mapping to voxel blocks
- [ ] **Shadows** - Implement shadow mapping
- [ ] **Post-processing** - Add bloom, SSAO, etc.
- [ ] **Skybox** - Add space background with stars
- [ ] **Particles** - Add particle effects for engines, explosions
- [ ] **UI Overlay** - Add HUD with ship stats and controls
- [ ] **Multiple views** - Split-screen or picture-in-picture
- [ ] **Recording** - Capture screenshots and videos
- [ ] **VR Support** - Add virtual reality rendering

## Troubleshooting

### Window doesn't open
- Ensure your graphics drivers are up to date
- Verify OpenGL 3.3 support with `glxinfo` (Linux) or `OpenGL Extensions Viewer` (Windows)
- Check console for error messages

### Poor performance
- Reduce number of voxel blocks in your ships
- Close other GPU-intensive applications
- Update graphics drivers

### Black screen
- May indicate shader compilation failure
- Check console output for shader errors
- Verify OpenGL version compatibility

### Controls not responding
- Ensure the graphics window has focus
- Try clicking inside the window
- Check for conflicting input from other applications

## Credits

Built with:
- **Silk.NET** - Modern, cross-platform .NET graphics library
- **OpenGL** - Industry-standard graphics API
- Inspired by **Avorion** by Boxelware

---

**Enjoy visualizing your voxel creations in 3D!** 🎨🚀
