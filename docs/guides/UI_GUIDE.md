# UI Framework Guide

## Overview

The AvorionLike game engine now includes a comprehensive UI framework built with **ImGui.NET**, providing immediate-mode GUI capabilities integrated with the 3D graphics rendering system.

## Architecture

### Core Components

1. **ImGuiController** (`Core/UI/ImGuiController.cs`)
   - Bridges ImGui.NET with Silk.NET OpenGL rendering
   - Handles input processing (keyboard, mouse)
   - Manages ImGui rendering pipeline
   - Provides font texture and shader management

2. **HUDSystem** (`Core/UI/HUDSystem.cs`)
   - Real-time game information display
   - Performance monitoring (FPS, frame time)
   - Entity and component inspection
   - Resource tracking across all inventories

3. **MenuSystem** (`Core/UI/MenuSystem.cs`)
   - Main menu for game startup
   - Pause menu for in-game pausing
   - Settings menu with tabs for Graphics, Audio, and Controls

4. **InventoryUI** (`Core/UI/InventoryUI.cs`)
   - Entity inventory management
   - Resource viewing and manipulation
   - Capacity tracking
   - Color-coded resource types

## Key Features

### HUD System

#### Main HUD Panel
- **Location:** Top-left corner
- **Always visible** during gameplay
- **Displays:**
  - FPS and frame time
  - Entity count
  - Control hints

#### Debug Overlay (F1)
- System information
- Memory usage (RAM)
- GC collection counts
- Component statistics
- Physics body count
- Voxel structure count

#### Entity List (F2)
- List all entities in the world
- Expand to view components
- Real-time component data:
  - Position, velocity, mass (Physics)
  - Block count, total mass (Voxel)
  - Resource count, capacity (Inventory)
  - Level, XP (Progression)

#### Resource Panel (F3)
- Global resource totals
- Aggregates across all entity inventories
- Color-coded resource types

### Menu System

#### Main Menu
- **New Game** - Start fresh game
- **Continue** - Resume last game
- **Load Game** - Load saved game
- **Settings** - Open settings menu
- **Exit** - Close application

#### Pause Menu (ESC)
- **Resume** - Continue gameplay
- **Settings** - Adjust game settings
- **Save Game** - Save current state
- **Main Menu** - Return to main menu

#### Settings Menu
- **Graphics Tab:**
  - Resolution selection
  - VSync toggle
  - Target FPS slider

- **Audio Tab:**
  - Master volume slider
  - Music volume slider
  - SFX volume slider

- **Controls Tab:**
  - Camera controls reference
  - UI controls reference

### Inventory System

#### Features
- **Entity Selector** - Choose entity to view
- **Resource Table:**
  - Color-coded resource names
  - Current amounts
  - Add/Remove buttons (+10/-10)
- **Capacity Bar** - Visual capacity usage
- **Debug Buttons:**
  - Add All Resources (+100)
  - Clear Inventory

#### Color Coding
- **Iron** - Gray
- **Titanium** - Light Blue
- **Naonite** - Green
- **Trinium** - Cyan
- **Xanion** - Purple
- **Ogonite** - Orange
- **Avorion** - Red
- **Credits** - Gold/Yellow

### Futuristic HUD System

The game now includes a comprehensive sci-fi HUD overlay with holographic-style elements, providing an immersive space game experience.

#### Features

**Corner Frames:**
- Animated holographic corner decorations
- Pulsing cyan frames with diagonal accents
- Creates a futuristic sci-fi aesthetic

**Radar/Scanner:**
- Top-right circular radar display
- Shows nearby entities as colored blips
- Animated scanning line effect
- Range indicator (default: 1000m)
- Selected targets pulse on radar

**Ship Status Panel:**
- Bottom-left ship status display
- Visual gauges for:
  - Hull integrity (cyan/green)
  - Shield strength (cyan)
  - Energy levels (yellow)
- Real-time ship mass and block count
- Color-coded warnings (low levels show orange/red)

**Target Information:**
- Right-side panel showing selected target details
- Displays target ID, position, distance
- Shows target structure information
- Pulse effect on selected target
- Press Tab to cycle through targets

**Navigation Compass:**
- Top-center circular compass
- Cardinal direction indicators (N/E/S/W)
- Animated heading indicator
- Real-time orientation display

**Holographic Effects:**
- Animated scan lines
- Subtle grid overlay
- Pulsing elements
- Semi-transparent panels

#### Controls
- `F4` - Toggle Futuristic HUD on/off
- `Tab` - Cycle through available targets

#### Color Scheme
- Primary: Cyan (#00CCFF) - Main UI elements
- Secondary: Bright Teal (#33FFCC) - Highlights
- Warning: Orange (#FF8000) - Alerts
- Danger: Red (#FF3333) - Critical status
- Frame: Semi-transparent Cyan - Decorative elements

## Controls

### Camera Controls
- `W` - Move forward
- `S` - Move backward
- `A` - Move left
- `D` - Move right
- `Space` - Move up
- `Shift` - Move down
- `Mouse` - Look around (free-look)

### UI Controls
- `F1` - Toggle Debug HUD (shown by default, displays FPS and entity count)
- `F2` - Toggle Entity List (detailed entity information)
- `F3` - Toggle Resource Panel (global resource tracking)
- `F4` - Toggle Futuristic HUD
- `I` - Toggle Inventory UI
- `ESC` - Pause Menu / Exit
- `Tab` - Cycle Targets (when Futuristic HUD is active)

## Integration

### Adding UI to Your Scene

The UI system is automatically integrated with the `GraphicsWindow` class:

```csharp
// In GraphicsWindow.OnLoad()
_imguiController = new ImGuiController(_gl, _window, _inputContext);
_hudSystem = new HUDSystem(_gameEngine);
_menuSystem = new MenuSystem(_gameEngine);
_inventoryUI = new InventoryUI(_gameEngine);
_shipBuilderUI = new ShipBuilderUI(_gameEngine);
_futuristicHUD = new FuturisticHUD(_gameEngine);
```
```

### Rendering Order

1. **3D Scene** - Voxel structures and entities
2. **UI Overlay** - ImGui rendering
   - Menu (if open) OR
   - HUD + Inventory (if not in menu)

### Input Handling

The system intelligently manages input:
- **Camera control** disabled when UI has focus
- **Game updates** paused when menu/inventory open
- **Mouse capture** respects ImGui's `WantCaptureMouse`

## Customization

### Adding New UI Panels

1. Create a new class in `Core/UI/`
2. Implement render logic with ImGui calls
3. Add to `GraphicsWindow.cs`:
   - Declare field
   - Initialize in `OnLoad()`
   - Call `Render()` in `OnRender()`
   - Call `HandleInput()` in `OnUpdate()`

Example:
```csharp
public class CustomPanel
{
    private readonly GameEngine _gameEngine;
    private bool _isOpen = false;
    
    public bool IsOpen => _isOpen;
    
    public CustomPanel(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
    }
    
    public void Render()
    {
        if (!_isOpen) return;
        
        if (ImGui.Begin("My Custom Panel", ref _isOpen))
        {
            ImGui.Text("Hello from custom panel!");
            // Add your UI code here
        }
        ImGui.End();
    }
    
    public void HandleInput()
    {
        if (ImGui.IsKeyPressed(ImGuiKey.F4))
            _isOpen = !_isOpen;
    }
}
```

### Styling

The UI uses ImGui's Dark theme by default. To customize:

```csharp
// In ImGuiController constructor, after ImGui.SetCurrentContext()
var style = ImGui.GetStyle();
style.WindowRounding = 5.0f;
style.FrameRounding = 3.0f;
// Customize colors
style.Colors[(int)ImGuiCol.WindowBg] = new Vector4(0.1f, 0.1f, 0.15f, 0.9f);
```

## Performance Considerations

### Optimization Tips

1. **Avoid frequent string allocations** in hot paths
2. **Use `ImGui.IsItemHovered()`** for tooltips, not constant rendering
3. **Collapse expensive panels** when not in use
4. **Cache entity queries** instead of calling every frame
5. **Use `ImGuiWindowFlags.NoResize`** when appropriate

### Profiling

The Debug Overlay shows:
- FPS and frame time
- Memory usage
- Entity counts

Monitor these to ensure UI doesn't impact performance.

## Troubleshooting

### UI Not Rendering
- Ensure `ImGuiController.Update()` called before rendering
- Verify `ImGuiController.Render()` called after ImGui draw calls
- Check that ImGui context is set correctly

### Input Not Working
- Verify input events are hooked in `SetupInput()`
- Check `io.WantCaptureKeyboard` / `io.WantCaptureMouse`
- Ensure event handlers are added to all input devices

### Font Issues
- Font texture created in `CreateFontTexture()`
- Default font used if custom font not loaded
- Check OpenGL texture binding

## Future Enhancements

Planned UI additions (see NEXT_STEPS.md):
- **Ship Builder UI** - Voxel placement interface
- **Trading Interface** - Buy/sell with stations
- **Map/Navigation** - Galaxy map and waypoints
- **Mission/Quest UI** - Quest tracking
- **Faction UI** - Reputation and relations

## Resources

- [ImGui.NET Documentation](https://github.com/ImGuiNET/ImGui.NET)
- [Dear ImGui GitHub](https://github.com/ocornut/imgui)
- [Silk.NET Documentation](https://github.com/dotnet/Silk.NET)

## Example Usage

```csharp
// Open inventory for first entity with inventory
var entities = gameEngine.EntityManager.GetAllEntities()
    .Where(e => gameEngine.EntityManager.GetComponent<InventoryComponent>(e.Id) != null);

if (entities.Any())
{
    inventoryUI.Show(entities.First().Id);
}

// Show pause menu
menuSystem.ShowPauseMenu();

// Hide all UI
menuSystem.HideMenu();
inventoryUI.Hide();
```
