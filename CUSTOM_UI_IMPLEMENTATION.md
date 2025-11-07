# Custom UI System Implementation Summary

## Overview

The game now has a **custom OpenGL-based UI rendering system** separate from ImGui. ImGui is now **only used for debug/console purposes** and can be toggled with **F1**.

## Architecture

### Two Separate UI Systems

1. **Game UI** - Custom OpenGL renderer
   - Used for all in-game HUD elements
   - Pause menus and settings
   - Crosshair and ship status
   - Rendered directly with OpenGL primitives

2. **Debug UI** - ImGui-based (F1 to toggle)
   - Console and debug information
   - Performance metrics
   - Entity inspection
   - Development tools only

## Key Components

### CustomUIRenderer (`Core/UI/CustomUIRenderer.cs`)

Low-level OpenGL renderer that provides drawing primitives:

**Features:**
- `DrawLine()` - Draw lines with thickness
- `DrawRect()` - Draw rectangle outlines
- `DrawRectFilled()` - Draw filled rectangles
- `DrawCircle()` - Draw circle outlines
- `DrawCircleFilled()` - Draw filled circles
- `DrawCrosshair()` - Centered crosshair
- `DrawCornerFrames()` - HUD corner frames with animation

**Technical Details:**
- Uses custom vertex/fragment shaders
- Orthographic projection for 2D UI
- Alpha blending support
- Dynamic vertex buffer updates
- Animated elements (pulsing effects)

### GameHUD (`Core/UI/GameHUD.cs`)

Main game heads-up display rendered with CustomUIRenderer:

**Elements:**
- Centered crosshair (small dot)
- Animated corner frames (sci-fi aesthetic)
- Ship status panel (top-left)
  - Hull integrity bar
  - Energy bar
  - Status indicators
- Radar/scanner (bottom-left)
  - Circular design
  - Grid lines
  - Player position marker
  - Entity tracking (TODO)

### GameMenuSystem (`Core/UI/GameMenuSystem.cs`)

Custom-rendered pause menu and settings:

**Features:**
- Semi-transparent overlay
- Pause menu with buttons:
  - Resume
  - Settings
  - Save Game
  - Load Game
  - Main Menu
- Settings menu (work in progress):
  - Difficulty slider
  - Volume controls
  - Mouse sensitivity
  - Tab-based organization

## Controls

### Gameplay
- **WASD** - Camera movement
- **Space/Shift** - Up/Down
- **Mouse** - Free-look camera
- **C** - Toggle ship control mode

### UI
- **ESC** - Toggle pause menu (press again to close)
- **ALT** - Show mouse cursor (hold, doesn't affect mouselook)
- **F1** - Toggle debug UI (ImGui)

### Mouse Behavior
- **Normal gameplay**: Mouse captured for free-look
- **Menu open**: Mouse visible and clickable
- **ALT held**: Mouse visible for UI interaction
- **Debug UI (F1)**: Mouse available for ImGui interaction

## Implementation Details

### Rendering Pipeline

```
1. 3D Scene Rendering
   - Starfield background
   - Voxel entities
   
2. Custom UI Rendering (Game HUD)
   - Enable blending
   - Disable depth test
   - Orthographic projection
   - Draw HUD elements
   - Draw menu if open
   
3. Debug UI Rendering (if enabled with F1)
   - ImGui rendering
   - Debug panels
```

### Color Scheme

The game UI uses a cyan/teal sci-fi aesthetic:
- **Primary**: Cyan (0.0, 0.8, 1.0)
- **Accent**: Bright Teal (0.2, 1.0, 0.8)
- **Warning**: Orange (1.0, 0.5, 0.0)
- **Danger**: Red (1.0, 0.2, 0.2)
- **HUD**: Cyan with alpha (0.0, 0.8, 1.0, 0.8)

### Animation

Corner frames and UI elements use sine wave animation:
```csharp
float pulse = 0.7f + 0.3f * MathF.Sin(animationTime * 2f);
```

## TODO / Future Enhancements

### High Priority
- [ ] Text rendering system for UI
  - Font atlas generation
  - Text shader
  - String rendering API
- [ ] Complete menu interaction
  - Mouse click detection
  - Button highlighting
  - Keyboard navigation
- [ ] Settings persistence
  - Save/load settings to file
  - Apply settings to game systems

### Medium Priority
- [ ] Enhanced ship status display
  - More detailed information
  - System status icons
  - Alert indicators
- [ ] Radar entity tracking
  - Show nearby ships
  - Color-code by faction
  - Distance indicators
- [ ] Inventory UI
  - Custom-rendered panels
  - Drag-and-drop support
  - Item tooltips

### Low Priority
- [ ] Animations and transitions
  - Menu fade in/out
  - Panel slide animations
  - Button press effects
- [ ] Advanced visual effects
  - Glow effects
  - Scan lines
  - Holographic distortion
- [ ] Customizable HUD
  - User-adjustable positions
  - Toggle individual elements
  - Color themes

## Benefits of Custom UI

1. **Performance**: Direct OpenGL rendering is faster than ImGui for game HUD
2. **Visual Consistency**: Custom styling matches game aesthetic
3. **Flexibility**: Full control over rendering and layout
4. **Separation**: Debug tools don't interfere with game UI
5. **Scalability**: Easy to add new UI elements and effects

## Debug UI (ImGui) Usage

ImGui is now reserved for:
- Performance metrics (FPS, frame time, memory)
- Entity inspection and debugging
- Component visualization
- Development console
- Test panels and tools

Toggle with **F1** key during gameplay.

## Integration with Existing Systems

### GraphicsWindow
- Manages both UI systems
- Handles mouse cursor mode switching
- Coordinates ESC and ALT key behavior

### Menu System (Old ImGui-based)
- Still exists for reference
- MenuSystem.cs uses ImGui
- Now replaced by GameMenuSystem for game UI

### HUDSystem (Old ImGui-based)
- Renamed to debugHUD in GraphicsWindow
- Only rendered when F1 debug mode is active
- Provides development information

## Migration Notes

**Old System:**
- Everything used ImGui (game + debug)
- Mixed UI styles
- Debug panels visible by default

**New System:**
- Game UI uses CustomUIRenderer
- Debug UI uses ImGui (F1 toggle)
- Clean separation of concerns
- Better performance and visual consistency

## File Structure

```
Core/
├── UI/
│   ├── CustomUIRenderer.cs      # OpenGL primitive renderer
│   ├── GameHUD.cs                # Main game HUD
│   ├── GameMenuSystem.cs         # Pause/settings menus
│   ├── HUDSystem.cs              # Debug HUD (ImGui)
│   ├── MenuSystem.cs             # Old menu system (ImGui)
│   └── ImGuiController.cs        # ImGui integration
└── Graphics/
    ├── GraphicsWindow.cs         # Main window & rendering
    ├── Shader.cs                 # Shader wrapper
    └── ...
```

## Example Usage

### Drawing a Simple HUD Element

```csharp
// In GameHUD.Render()
_renderer.BeginRender();

// Draw a status bar
Vector2 barPos = new Vector2(100, 100);
Vector2 barSize = new Vector2(200, 20);
Vector4 barColor = new Vector4(0.0f, 1.0f, 0.5f, 0.9f);

_renderer.DrawRectFilled(barPos, barSize, barColor);
_renderer.DrawRect(barPos, barSize, borderColor, 2f);

_renderer.EndRender();
```

### Adding New Menu Items

```csharp
// In GameMenuSystem.RenderPauseMenu()
string[] menuItems = { "Resume", "Settings", "New Item" };

for (int i = 0; i < menuItems.Length; i++)
{
    Vector2 buttonPos = new Vector2(x, y + i * spacing);
    _renderer.DrawRectFilled(buttonPos, buttonSize, buttonColor);
    _renderer.DrawRect(buttonPos, buttonSize, borderColor, 2f);
    // TODO: Draw button text
}
```

## Conclusion

The custom UI system provides a solid foundation for building a professional-looking game interface while keeping ImGui available for development and debugging. The separation of concerns makes both systems easier to maintain and extend.
