# Native Windows UI Considerations

## Current State

The game currently uses **ImGui.NET** via Silk.NET for its UI layer, which provides a cross-platform immediate-mode GUI rendered with OpenGL. While functional and cross-platform, the user mentioned interest in migrating to native Windows UI.

## Benefits of Native Windows UI

### Advantages
- **Native Look & Feel**: Matches Windows 11 design language (Fluent Design)
- **Better OS Integration**: Native file dialogs, notifications, taskbar integration
- **Accessibility**: Built-in Windows accessibility features (screen readers, high contrast, etc.)
- **Performance**: Can potentially reduce overhead of rendering UI through OpenGL
- **Theming**: Automatic dark/light mode support from Windows
- **Input**: Better Windows input method integration (IME, touch, pen)

### Potential Disadvantages
- **Platform Lock-in**: Lose cross-platform support (Linux/macOS)
- **Learning Curve**: Different paradigm from immediate-mode GUI
- **Migration Effort**: Significant rewrite of UI code
- **Dependency on WinUI 3/WPF**: Requires Windows 10+ or .NET WPF

## Implementation Options

### Option 1: WinUI 3 (Recommended)
**Technology**: Microsoft's latest native Windows UI framework

**Pros**:
- Modern, follows Windows 11 Fluent Design
- Good performance with DirectX
- Native Windows 11 features (rounded corners, shadows, etc.)
- Active development by Microsoft
- Good for new applications

**Cons**:
- Windows 10 version 1809+ only
- Some learning curve
- Requires Windows App SDK

**Architecture**:
```
┌─────────────────────────────────────┐
│     WinUI 3 Application             │
│  ┌───────────────────────────────┐  │
│  │   Main Window (XAML)          │  │
│  │  ┌─────────────────────────┐  │  │
│  │  │ 3D Viewport (Win2D/DX)  │  │  │
│  │  └─────────────────────────┘  │  │
│  │  ┌─────────────────────────┐  │  │
│  │  │ UI Controls (Native)    │  │  │
│  │  └─────────────────────────┘  │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
         ↕ (P/Invoke or C++/WinRT)
┌─────────────────────────────────────┐
│     Game Engine (C# .NET 9)         │
│  • EntityManager                    │
│  • Systems (Physics, Combat, etc.)  │
│  • OpenGL/DirectX Renderer          │
└─────────────────────────────────────┘
```

### Option 2: WPF (Windows Presentation Foundation)
**Technology**: Mature .NET UI framework

**Pros**:
- Mature, stable, well-documented
- Large ecosystem of controls
- Works on Windows 7+
- Good MVVM support
- Can host OpenGL/DirectX via HwndHost

**Cons**:
- Older technology (but still supported)
- Less modern than WinUI 3
- Performance can be slower for complex UIs

**Architecture**:
```
┌─────────────────────────────────────┐
│     WPF Application                 │
│  ┌───────────────────────────────┐  │
│  │   Main Window (XAML)          │  │
│  │  ┌─────────────────────────┐  │  │
│  │  │ HwndHost (OpenGL)       │  │  │
│  │  └─────────────────────────┘  │  │
│  │  ┌─────────────────────────┐  │  │
│  │  │ WPF Controls            │  │  │
│  │  └─────────────────────────┘  │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
         ↕
┌─────────────────────────────────────┐
│     Game Engine (C# .NET 9)         │
└─────────────────────────────────────┘
```

### Option 3: Windows Forms (Not Recommended)
**Technology**: Legacy Windows UI framework

**Why Not Recommended**:
- Very old technology
- Poor for modern game UIs
- Limited styling options
- Doesn't support modern Windows features

## Migration Strategy

### Phase 1: Hybrid Approach (Least Disruption)
Keep the current OpenGL + ImGui for the 3D game view, but wrap it in a native Windows shell:

```csharp
// WinUI 3 Main Window
public sealed partial class MainWindow : Window
{
    private GraphicsWindow _gameWindow; // Current Silk.NET window
    
    public MainWindow()
    {
        // Native Windows chrome and menus
        this.InitializeComponent();
        
        // Embed game window in SwapChainPanel
        var panel = this.Content as SwapChainPanel;
        _gameWindow = new GraphicsWindow(gameEngine);
        _gameWindow.AttachToPanel(panel);
    }
}
```

**Benefits**:
- Minimal changes to game code
- Get native Windows shell immediately
- Can migrate UI piecemeal
- Test native integration without full rewrite

### Phase 2: Gradual UI Migration
Replace ImGui dialogs one-by-one with native Windows controls:

1. **Start with simple dialogs**: Settings, About
2. **Move to forms**: Ship builder, inventory
3. **Convert HUD last**: Keep ImGui HUD until native replacement is ready

### Phase 3: Full Native UI
Complete migration with all UI in native Windows framework.

## Code Architecture Changes

### Current Architecture
```
Program.cs (Console)
    ↓
GraphicsWindow (Silk.NET)
    ↓
ImGui.NET UI Rendering
    ↓
OpenGL 3D Rendering
```

### Proposed Native Architecture
```
WinUI 3 App
    ↓
MainWindow (XAML) ← Native UI Controls
    ├→ SwapChainPanel ← Game Viewport
    │     ↓
    │  GraphicsWindow (Silk.NET or SharpDX)
    │     ↓
    │  DirectX/OpenGL 3D Rendering
    │
    └→ Native Controls (XAML) ← HUD, Menus, Dialogs
```

## Implementation Example

### WinUI 3 Integration

**1. Create WinUI 3 Project**
```bash
dotnet new winui3 -n CodenameSubspace.WinUI
```

**2. Reference Game Engine**
```xml
<ProjectReference Include="..\AvorionLike\AvorionLike.csproj" />
```

**3. Main Window XAML**
```xml
<Window
    x:Class="CodenameSubspace.WinUI.MainWindow"
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">
    
    <Grid>
        <Grid.RowDefinitions>
            <RowDefinition Height="Auto"/>
            <RowDefinition Height="*"/>
            <RowDefinition Height="Auto"/>
        </Grid.RowDefinitions>
        
        <!-- Menu Bar -->
        <MenuBar Grid.Row="0">
            <MenuBarItem Title="File">
                <MenuFlyoutItem Text="New Game" Click="NewGame_Click"/>
                <MenuFlyoutItem Text="Load Game" Click="LoadGame_Click"/>
                <MenuFlyoutSeparator/>
                <MenuFlyoutItem Text="Exit" Click="Exit_Click"/>
            </MenuBarItem>
        </MenuBar>
        
        <!-- Game Viewport -->
        <SwapChainPanel x:Name="GameViewport" Grid.Row="1"/>
        
        <!-- Status Bar -->
        <Grid Grid.Row="2" Background="{ThemeResource SystemControlBackgroundAltHighBrush}">
            <TextBlock x:Name="StatusText" Margin="10,5"/>
        </Grid>
    </Grid>
</Window>
```

**4. Code-Behind Integration**
```csharp
using Microsoft.UI.Xaml;
using AvorionLike.Core;

namespace CodenameSubspace.WinUI
{
    public sealed partial class MainWindow : Window
    {
        private GameEngine _gameEngine;
        private GameRenderer _renderer;
        
        public MainWindow()
        {
            this.InitializeComponent();
            InitializeGame();
        }
        
        private void InitializeGame()
        {
            _gameEngine = new GameEngine(12345);
            _gameEngine.Start();
            
            // Attach renderer to SwapChainPanel
            _renderer = new GameRenderer(_gameEngine);
            _renderer.AttachToSwapChain(GameViewport);
        }
        
        private void NewGame_Click(object sender, RoutedEventArgs e)
        {
            // Native dialog instead of ImGui
            var dialog = new ContentDialog
            {
                Title = "New Game",
                Content = new NewGameDialog(),
                PrimaryButtonText = "Start",
                CloseButtonText = "Cancel",
                XamlRoot = this.Content.XamlRoot
            };
            
            var result = await dialog.ShowAsync();
            if (result == ContentDialogResult.Primary)
            {
                StartNewGame();
            }
        }
        
        private void StartNewGame()
        {
            // Create player ship and start game
            // (Existing StartNewGame() logic)
        }
    }
}
```

## Rendering Considerations

### DirectX Integration
WinUI 3 uses DirectX natively, so we should consider:

1. **Keep OpenGL**: Continue using Silk.NET with OpenGL, render to texture, display in SwapChainPanel
2. **Switch to DirectX**: Use SharpDX or Vortice.Windows to render directly with DirectX 11/12
3. **Hybrid**: Use DirectX for UI elements, OpenGL for 3D game world

**Recommended**: Start with Option 1 (keep OpenGL), migrate to DirectX only if needed for performance.

## Data Binding with MVVM

For native Windows UI, use Model-View-ViewModel pattern:

```csharp
// ViewModel for player status
public class PlayerStatusViewModel : INotifyPropertyChanged
{
    private readonly GameEngine _gameEngine;
    private string _zoneName;
    private int _credits;
    
    public string ZoneName
    {
        get => _zoneName;
        set { _zoneName = value; OnPropertyChanged(); }
    }
    
    public int Credits
    {
        get => _credits;
        set { _credits = value; OnPropertyChanged(); }
    }
    
    public void Update()
    {
        // Update from game engine
        var player = _gameEngine.EntityManager.GetComponent<PlayerProgressionComponent>(playerId);
        ZoneName = player.CurrentZone;
        
        var inventory = _gameEngine.EntityManager.GetComponent<InventoryComponent>(playerId);
        Credits = inventory.Inventory.GetResourceAmount(ResourceType.Credits);
    }
}
```

## Timeline Estimate

### Quick Win (1-2 weeks)
- Create WinUI 3 shell application
- Embed current OpenGL window
- Add native menu bar
- Test integration

### Partial Migration (1-2 months)
- Convert major dialogs to native
- Settings UI
- Inventory UI
- Ship builder UI
- Keep ImGui for HUD/overlays

### Full Migration (3-6 months)
- Convert all UI to native
- Reimplement HUD natively
- Polish and optimize
- Add Windows-specific features

## Recommendation

**For the short term**: Continue with ImGui for rapid development and cross-platform support.

**For future Windows-only release**: 
1. Start with **WinUI 3** for modern Windows experience
2. Use **hybrid approach** - embed current game window
3. Migrate UI **gradually** - start with dialogs and menus
4. Consider keeping ImGui for in-game HUD (it works well for that)

**Best of both worlds**:
- Maintain current codebase for cross-platform
- Create separate `CodenameSubspace.WinUI` project for Windows-native version
- Share game engine code between both

## Files to Create/Modify for Migration

### New Files
```
CodenameSubspace.WinUI/
├── App.xaml
├── App.xaml.cs
├── MainWindow.xaml
├── MainWindow.xaml.cs
├── Views/
│   ├── NewGameDialog.xaml
│   ├── SettingsPage.xaml
│   └── InventoryPage.xaml
├── ViewModels/
│   ├── MainViewModel.cs
│   ├── PlayerStatusViewModel.cs
│   └── InventoryViewModel.cs
└── GameRenderer.cs (DirectX integration)
```

### Modified Files
```
AvorionLike/Core/
├── GameEngine.cs (add event system for UI updates)
├── Graphics/
│   └── GraphicsWindow.cs (add SwapChainPanel support)
```

## Conclusion

Moving to native Windows UI is feasible and would provide a more polished Windows experience. The hybrid approach allows for gradual migration while maintaining functionality. WinUI 3 is the recommended technology for modern Windows applications.

However, **consider keeping cross-platform support** by maintaining both UIs - the current ImGui for Linux/macOS and native WinUI 3 for Windows.
