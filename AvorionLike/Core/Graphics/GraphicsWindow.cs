using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Input;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.UI;
using AvorionLike.Core.Input;
using AvorionLike.Core.DevTools;
using AvorionLike.Core.Config;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Modular;
using Silk.NET.OpenGL.Extensions.ImGui;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Main graphics window for rendering the game world
/// Handles window creation, input, and rendering loop
/// </summary>
public class GraphicsWindow : IDisposable
{
    private IWindow? _window;
    private GL? _gl;
    private EnhancedVoxelRenderer? _voxelRenderer;
    private MeshRenderer? _meshRenderer;  // NEW: For rendering 3D models
    private StarfieldRenderer? _starfieldRenderer;
    private DebugRenderer? _debugRenderer;
    private Camera? _camera;
    private IInputContext? _inputContext;
    private ImGuiController? _imguiController;
    private PlayerControlSystem? _playerControlSystem;
    
    // Custom UI system (for game HUD and menus)
    private CustomUIRenderer? _customUIRenderer;
    private GameHUD? _gameHUD;
    private GameMenuSystem? _gameMenuSystem;
    private GalaxyMapUI? _galaxyMapUI;
    private TitleScreen? _titleScreen;
    
    // ImGui-based UI systems (for debug/console ONLY)
    private HUDSystem? _debugHUD;  // Renamed to clarify it's for debug
    private bool _showDebugUI = true;  // Toggle for debug UI - enabled by default for better UX
    
    // In-game testing console
    private InGameTestingConsole? _testingConsole;
    private string _consoleInput = "";
    private bool _consoleShiftPressed = false;
    
    private readonly GameEngine _gameEngine;
    private bool _disposed = false;
    private bool _playerControlMode = true; // Start in ship control mode (third-person) by default
    private bool _shouldClose = false; // Signal to close window and return to main menu
    
    // Title screen and background ship
    private Guid? _backgroundShipId = null;
    
    // Callback for new game request from title screen
    public Action? OnNewGameRequested { get; set; }
    
    // Mouse state
    private Vector2 _lastMousePos;
    private Vector2 _currentMousePos;
    private bool _firstMouse = true;
    private bool _uiWantsMouse = false;
    private bool _altKeyHeld = false; // Track if ALT is held for showing mouse cursor
    private bool _mouseLookEnabled = true; // Track if mouse look is active
    private readonly HashSet<MouseButton> _mouseButtonsPressed = new();
    
    // Timing
    private float _deltaTime = 0.0f;

    // Input state
    private readonly HashSet<Key> _keysPressed = new();

    public GraphicsWindow(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
    }
    
    /// <summary>
    /// Request window to close and return to main menu
    /// </summary>
    public void RequestClose()
    {
        _shouldClose = true;
    }
    
    /// <summary>
    /// Sets the player ship for control and UI tracking
    /// </summary>
    public void SetPlayerShip(Guid shipId)
    {
        if (_playerControlSystem != null)
        {
            _playerControlSystem.ControlledShipId = shipId;
        }
        if (_gameHUD != null)
        {
            _gameHUD.PlayerShipId = shipId;
        }
        if (_testingConsole != null)
        {
            _testingConsole.SetPlayerShip(shipId);
        }
        if (_galaxyMapUI != null)
        {
            _galaxyMapUI.PlayerShipId = shipId;
        }
    }
    
    /// <summary>
    /// Dismiss the title screen and start gameplay
    /// </summary>
    public void DismissTitleScreen()
    {
        if (_titleScreen != null)
        {
            _titleScreen.Dismiss();
            
            // Remove background ship if it exists
            if (_backgroundShipId.HasValue)
            {
                _gameEngine.EntityManager.DestroyEntity(_backgroundShipId.Value);
                _backgroundShipId = null;
            }
        }
    }

    public void Run()
    {
        var options = WindowOptions.Default;
        options.Size = new Silk.NET.Maths.Vector2D<int>(1280, 720);
        options.Title = "Codename:Subspace - 3D Visualization";
        options.VSync = true;

        _window = Window.Create(options);

        _window.Load += OnLoad;
        _window.Update += OnUpdate;
        _window.Render += OnRender;
        _window.Closing += OnClosing;

        _window.Run();
    }

    private void OnLoad()
    {
        _gl = _window!.CreateOpenGL();
        _inputContext = _window!.CreateInput();

        // Initialize camera with better third-person view position
        _camera = new Camera(new Vector3(0, 50, 150));
        // Set chase camera parameters for good ship visibility (distance, height, smoothness)
        _camera.SetChaseParameters(80.0f, 40.0f, 5.0f); // Further back and higher for better view

        // Initialize renderers
        _voxelRenderer = new EnhancedVoxelRenderer(_gl);
        _meshRenderer = new MeshRenderer(_gl);  // NEW: Initialize mesh renderer
        _starfieldRenderer = new StarfieldRenderer(_gl);
        _debugRenderer = new DebugRenderer(_gl);

        // Initialize custom UI renderer for game HUD and menus
        // Note: _window is guaranteed to be non-null here as it's assigned in Run() before OnLoad() is called
        _customUIRenderer = new CustomUIRenderer(_gl, _window!.Size.X, _window.Size.Y);
        _gameHUD = new GameHUD(_gameEngine, _customUIRenderer, _window.Size.X, _window.Size.Y);
        _gameMenuSystem = new GameMenuSystem(_gameEngine, _customUIRenderer, _window.Size.X, _window.Size.Y);
        
        // Set callback for returning to main menu
        _gameMenuSystem.SetReturnToMainMenuCallback(() => RequestClose());
        
        // Initialize ImGui for DEBUG/CONSOLE ONLY using Silk.NET extension
        _imguiController = new ImGuiController(_gl, _window!, _inputContext);
        _debugHUD = new HUDSystem(_gameEngine);
        
        // Initialize Galaxy Map UI (using default seed 0 for consistent generation)
        _galaxyMapUI = new GalaxyMapUI(_gameEngine.EntityManager, _gameEngine.NavigationSystem, 0);
        
        // Initialize In-Game Testing Console
        _testingConsole = new InGameTestingConsole(_gameEngine);
        
        // Initialize Player Control System
        _playerControlSystem = new PlayerControlSystem(_gameEngine.EntityManager);
        
        // Initialize Title Screen
        _titleScreen = new TitleScreen(_gameEngine);
        
        // Connect title screen callback to our own callback (set by Program.cs)
        _titleScreen.OnNewGameRequested = () =>
        {
            OnNewGameRequested?.Invoke();
        };
        
        // Create background ship for title screen
        CreateBackgroundShip();

        // Set clear color to pure black for space (fixes light blue screen on startup)
        _gl.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        // Enable depth testing with proper depth function
        _gl.Enable(EnableCap.DepthTest);
        _gl.DepthFunc(DepthFunction.Less);
        _gl.DepthMask(true); // Ensure depth writes are enabled
        
        // Enable face culling for performance (~50% fewer fragments to render)
        // Voxel vertices use correct CCW winding order for front faces
        _gl.Enable(EnableCap.CullFace);
        _gl.CullFace(TriangleFace.Back);
        _gl.FrontFace(FrontFaceDirection.Ccw);

        // Set up input
        foreach (var keyboard in _inputContext.Keyboards)
        {
            keyboard.KeyDown += OnKeyDown;
            keyboard.KeyUp += OnKeyUp;
        }

        foreach (var mouse in _inputContext.Mice)
        {
            mouse.MouseMove += OnMouseMove;
            mouse.MouseDown += OnMouseDown;
            mouse.MouseUp += OnMouseUp;
            // Start with Raw mode for free look
            mouse.Cursor.CursorMode = CursorMode.Raw;
        }

        Console.WriteLine("\n=== 3D Graphics Window Active ===");
        Console.WriteLine("Controls:");
        Console.WriteLine("  Ship Control Mode (Third-Person - DEFAULT):");
        Console.WriteLine("    WASD - Thrust (Forward/Back/Left/Right)");
        Console.WriteLine("    Space/Shift - Thrust Up/Down");
        Console.WriteLine("    Arrow Keys - Pitch/Yaw");
        Console.WriteLine("    Q/E - Roll");
        Console.WriteLine("    X - Emergency Brake");
        Console.WriteLine("    Mouse - Look around (camera follows ship)");
        Console.WriteLine("  Free Camera Mode (Press C to toggle):");
        Console.WriteLine("    WASD - Move camera");
        Console.WriteLine("    Space/Shift - Move up/down");
        Console.WriteLine("    Mouse - Look around (free-look)");
        Console.WriteLine("  UI Controls:");
        Console.WriteLine("    M - Toggle Galaxy Map");
        Console.WriteLine("    ~ (Tilde) - Toggle In-Game Testing Console");
        Console.WriteLine("    Console Button - Click bottom-left button to open/close console ✨ NEW!");
        Console.WriteLine("    ALT - Show mouse cursor (hold, doesn't affect free-look)");
        Console.WriteLine("    ESC - Pause Menu (press again to close)");
        Console.WriteLine("    F1 - Toggle Debug HUD (enabled by default)");
        Console.WriteLine("    F2 - Toggle Entity List");
        Console.WriteLine("    F3 - Toggle Resource Panel");
        Console.WriteLine("  Debug Helpers (for voxel rendering issues):");
        Console.WriteLine("    F7 - Toggle Two-Sided Rendering (fixes disappearing faces)");
        Console.WriteLine("    F8 - Bypass Frustum/Occlusion Culling (render all chunks)");
        Console.WriteLine("    F11 - Show Wireframe AABBs (bounding boxes)");
        Console.WriteLine("    F12 - Show Generation Stats (pending tasks/results)");
        Console.WriteLine("=====================================\n");
    }

    private void OnUpdate(double deltaTime)
    {
        _deltaTime = (float)deltaTime;

        if (_camera == null || _imguiController == null || _playerControlSystem == null || 
            _inputContext == null || _gameHUD == null || _gameMenuSystem == null) return;

        // Check if window should close (return to main menu)
        if (_shouldClose && _window != null)
        {
            _window.Close();
            return;
        }

        // Update ImGui (needed for GameHUD text rendering and debug UI)
        _imguiController.Update(_deltaTime);
        
        // Handle title screen if active
        if (_titleScreen != null && _titleScreen.IsActive)
        {
            _titleScreen.Update(_deltaTime);
            _titleScreen.HandleInput();
            
            // Update physics for background ship
            _gameEngine.PhysicsSystem.Update(_deltaTime);
            
            // Position camera to view the background ship
            if (_backgroundShipId.HasValue)
            {
                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_backgroundShipId.Value);
                if (physics != null)
                {
                    // Use FollowTarget to smoothly track the ship
                    // Set chase parameters for title screen viewing
                    _camera.SetChaseParameters(150.0f, 60.0f, 2.0f);
                    _camera.FollowTarget(physics.Position, physics.Velocity, _deltaTime);
                }
            }
            
            // Show mouse cursor on title screen
            foreach (var mouse in _inputContext.Mice)
            {
                if (mouse.Cursor.CursorMode != CursorMode.Normal)
                {
                    mouse.Cursor.CursorMode = CursorMode.Normal;
                }
            }
            
            return; // Skip gameplay update while title screen is active
        }
        
        // Handle HUD input (F1/F2/F3 toggles)
        if (_debugHUD != null)
        {
            _debugHUD.HandleInput();
        }
        
        // Update custom UI
        _gameHUD.Update(_deltaTime);
        
        // Check if ImGui wants mouse input (only when debug UI is shown)
        var io = ImGuiNET.ImGui.GetIO();
        _uiWantsMouse = _showDebugUI && io.WantCaptureMouse;
        
        // Check if ALT key is held
        _altKeyHeld = _keysPressed.Contains(Key.AltLeft) || _keysPressed.Contains(Key.AltRight);
        
        // Determine if menu is open
        bool menuOpen = _gameMenuSystem.IsMenuOpen;
        bool galaxyMapOpen = _galaxyMapUI?.IsOpen ?? false;
        
        // Handle galaxy map input
        if (_galaxyMapUI != null)
        {
            _galaxyMapUI.HandleInput();
        }
        
        // Update mouse cursor mode based on state
        foreach (var mouse in _inputContext.Mice)
        {
            if (menuOpen || galaxyMapOpen || _altKeyHeld)
            {
                // Show cursor when menu is open, galaxy map is open, or ALT is held
                if (mouse.Cursor.CursorMode != CursorMode.Normal)
                {
                    mouse.Cursor.CursorMode = CursorMode.Normal;
                    _mouseLookEnabled = false;
                }
            }
            else
            {
                // Hide cursor and enable free-look during normal gameplay
                if (mouse.Cursor.CursorMode != CursorMode.Raw)
                {
                    mouse.Cursor.CursorMode = CursorMode.Raw;
                    _firstMouse = true; // Reset mouse to avoid jumps
                    _mouseLookEnabled = true;
                }
            }
        }

        // Process keyboard input
        bool anyUIOpen = menuOpen || galaxyMapOpen;
        
        if (!io.WantCaptureKeyboard && !anyUIOpen)
        {
            if (_playerControlMode && _playerControlSystem.ControlledShipId.HasValue)
            {
                // Ship control mode
                _playerControlSystem.Update(_deltaTime);
                
                // Follow player ship with smooth chase camera using interpolated position
                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_playerControlSystem.ControlledShipId.Value);
                if (physics != null)
                {
                    // Use interpolated position for smoother camera follow
                    _camera.FollowTarget(physics.InterpolatedPosition, physics.Velocity, _deltaTime);
                }
            }
            else
            {
                // Camera control mode
                if (_keysPressed.Contains(Key.W))
                    _camera.ProcessKeyboard(CameraMovement.Forward, _deltaTime);
                if (_keysPressed.Contains(Key.S))
                    _camera.ProcessKeyboard(CameraMovement.Backward, _deltaTime);
                if (_keysPressed.Contains(Key.A))
                    _camera.ProcessKeyboard(CameraMovement.Left, _deltaTime);
                if (_keysPressed.Contains(Key.D))
                    _camera.ProcessKeyboard(CameraMovement.Right, _deltaTime);
                if (_keysPressed.Contains(Key.Space))
                    _camera.ProcessKeyboard(CameraMovement.Up, _deltaTime);
                if (_keysPressed.Contains(Key.ShiftLeft))
                    _camera.ProcessKeyboard(CameraMovement.Down, _deltaTime);
            }
        }
        
        // Handle menu input (ESC key handling and mouse position)
        if (_gameMenuSystem != null)
        {
            foreach (var keyboard in _inputContext.Keyboards)
            {
                _gameMenuSystem.HandleInput(keyboard);
            }
            _gameMenuSystem.HandleMouseMove(_currentMousePos);
        }

        // Update game engine (pause if menu is open)
        if (!anyUIOpen)
        {
            _gameEngine.Update();
        }
    }

    private void OnRender(double deltaTime)
    {
        if (_gl == null || _voxelRenderer == null || _starfieldRenderer == null || _camera == null || 
            _window == null || _imguiController == null || _gameHUD == null || _gameMenuSystem == null) return;

        // Clear the screen (clear color is set once in OnLoad)
        _gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);
        
        // Toggle backface culling based on debug flag
        if (DebugConfig.TwoSidedRendering)
        {
            _gl.Disable(EnableCap.CullFace);
        }
        else
        {
            _gl.Enable(EnableCap.CullFace);
        }

        // Calculate aspect ratio from window size
        float aspectRatio = (float)_window.Size.X / _window.Size.Y;

        // Interpolate physics for smooth rendering (use deltaTime as alpha)
        // This provides smooth motion between physics updates
        float alpha = Math.Clamp(_deltaTime * 60f, 0f, 1f); // Assume 60 FPS target
        _gameEngine.PhysicsSystem.InterpolatePhysics(alpha);

        // Render starfield background first (without depth write)
        _starfieldRenderer.Render(_camera, aspectRatio);

        // Render all entities with voxel structures (including background ship on title screen)
        var entities = _gameEngine.EntityManager.GetAllEntities();
        foreach (var entity in entities)
        {
            var voxelComponent = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(entity.Id);
            if (voxelComponent != null)
            {
                // Get interpolated position from physics component for smooth rendering
                Vector3 position = Vector3.Zero;
                var physicsComponent = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
                if (physicsComponent != null)
                {
                    position = physicsComponent.InterpolatedPosition;
                }

                _voxelRenderer.RenderVoxelStructure(voxelComponent, _camera, position, aspectRatio);
            }
            
            // NEW: Render modular ships with 3D models
            var modularShipComponent = _gameEngine.EntityManager.GetComponent<ModularShipComponent>(entity.Id);
            if (modularShipComponent != null && _meshRenderer != null)
            {
                RenderModularShip(modularShipComponent, entity.Id, aspectRatio);
            }
        }
        
        // Render debug visualizations if enabled
        if (_debugRenderer != null && DebugConfig.ShowAABBs)
        {
            // Clear previous frame's debug lines
            _debugRenderer.Clear();
            
            // Note: ChunkManager is not directly accessible from GameEngine
            // AABB visualization would be implemented when ChunkManager is exposed
            // For now, this is a placeholder for the debug visualization framework
            
            // Example of how AABBs would be drawn if ChunkManager was accessible:
            // foreach (var chunk in chunkManager.GetLoadedChunks())
            // {
            //     if (chunk.BoundingBox != null)
            //     {
            //         _debugRenderer.DrawAABB(chunk.BoundingBox.Min, chunk.BoundingBox.Max, "Yellow");
            //     }
            // }
            
            // Render the debug visualizations
            _debugRenderer.Render(_camera.GetViewMatrix(), _camera.GetProjectionMatrix(aspectRatio));
        }
        
        // Update debug renderer (remove expired items)
        if (_debugRenderer != null)
        {
            _debugRenderer.Update(_deltaTime);
        }
        
        // Render title screen if active (overlays everything)
        if (_titleScreen != null && _titleScreen.IsActive)
        {
            _titleScreen.Render();
            
            // Render ImGui for title screen
            _imguiController.Render();
            return; // Skip game UI rendering while title screen is active
        }
        
        // Render custom game HUD (crosshair, ship status, radar, corner frames)
        _gameHUD.Render();
        
        // Render custom game menu (pause menu, settings) if open
        _gameMenuSystem.Render();
        
        // Render debug/console UI if enabled with F1
        if (_showDebugUI && _debugHUD != null)
        {
            _debugHUD.Render();
        }
        
        // Render In-Game Testing Console if visible
        if (_testingConsole != null && _testingConsole.IsVisible)
        {
            RenderTestingConsole();
        }
        
        // Render Console Toggle Button (always visible for easy access)
        RenderConsoleToggleButton();
        
        // Render Galaxy Map if open
        if (_galaxyMapUI != null && _galaxyMapUI.IsOpen)
        {
            _galaxyMapUI.Render();
        }
        
        // Always render ImGui (needed for GameHUD text and debug UI when enabled)
        _imguiController.Render();
    }
    
    private void RenderConsoleToggleButton()
    {
        if (_testingConsole == null || _window == null) return;
        
        // Position button in bottom-left corner, above where console would appear
        float buttonWidth = 150f;
        float buttonHeight = 30f;
        Vector2 buttonPos = new Vector2(10, _window.Size.Y - 320);
        
        ImGuiNET.ImGui.SetNextWindowPos(buttonPos);
        ImGuiNET.ImGui.SetNextWindowSize(new Vector2(buttonWidth, buttonHeight));
        ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.WindowBg, new Vector4(0.0f, 0.15f, 0.2f, 0.85f));
        ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.Border, new Vector4(0.0f, 0.9f, 1.0f, 0.9f));
        ImGuiNET.ImGui.PushStyleVar(ImGuiNET.ImGuiStyleVar.WindowPadding, new Vector2(8, 6));
        ImGuiNET.ImGui.PushStyleVar(ImGuiNET.ImGuiStyleVar.WindowBorderSize, 2f);
        
        if (ImGuiNET.ImGui.Begin("##ConsoleToggle", ImGuiNET.ImGuiWindowFlags.NoTitleBar | 
                                ImGuiNET.ImGuiWindowFlags.NoResize | ImGuiNET.ImGuiWindowFlags.NoMove | 
                                ImGuiNET.ImGuiWindowFlags.NoScrollbar))
        {
            string buttonText = _testingConsole.IsVisible ? "▼ CONSOLE" : "▲ CONSOLE";
            Vector4 buttonColor = _testingConsole.IsVisible ? 
                new Vector4(0.0f, 0.9f, 1.0f, 1.0f) : new Vector4(0.0f, 0.7f, 0.9f, 0.8f);
            
            ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.Button, new Vector4(0.0f, 0.3f, 0.4f, 0.7f));
            ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.ButtonHovered, new Vector4(0.0f, 0.5f, 0.6f, 0.9f));
            ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.ButtonActive, new Vector4(0.0f, 0.7f, 0.8f, 1.0f));
            ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.Text, buttonColor);
            
            if (ImGuiNET.ImGui.Button(buttonText, new Vector2(buttonWidth - 16, buttonHeight - 12)))
            {
                _testingConsole.Toggle();
                if (_testingConsole.IsVisible)
                {
                    _consoleInput = "";
                }
            }
            
            ImGuiNET.ImGui.PopStyleColor(4);
        }
        ImGuiNET.ImGui.End();
        ImGuiNET.ImGui.PopStyleVar(2);
        ImGuiNET.ImGui.PopStyleColor(2);
    }
    
    private void RenderTestingConsole()
    {
        if (_testingConsole == null || _window == null) return;
        
        // Create console window using ImGui with better styling
        ImGuiNET.ImGui.SetNextWindowPos(new Vector2(10, _window.Size.Y - 310));
        ImGuiNET.ImGui.SetNextWindowSize(new Vector2(_window.Size.X - 20, 300));
        ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.WindowBg, new Vector4(0.0f, 0.05f, 0.08f, 0.95f));
        ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.Border, new Vector4(0.0f, 0.9f, 1.0f, 0.9f));
        ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.TitleBg, new Vector4(0.0f, 0.2f, 0.25f, 0.9f));
        ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.TitleBgActive, new Vector4(0.0f, 0.3f, 0.35f, 1.0f));
        ImGuiNET.ImGui.PushStyleVar(ImGuiNET.ImGuiStyleVar.WindowBorderSize, 3f);
        
        if (ImGuiNET.ImGui.Begin("⬡ IN-GAME TESTING CONSOLE", ImGuiNET.ImGuiWindowFlags.NoCollapse | 
                                ImGuiNET.ImGuiWindowFlags.NoMove | ImGuiNET.ImGuiWindowFlags.NoResize))
        {
            // Help text
            ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.Text, new Vector4(0.4f, 0.9f, 1.0f, 0.8f));
            ImGuiNET.ImGui.Text("Type 'help' for available commands. Press ~ or click button to toggle.");
            ImGuiNET.ImGui.PopStyleColor();
            ImGuiNET.ImGui.Separator();
            ImGuiNET.ImGui.Spacing();
            
            // Output history with color coding
            ImGuiNET.ImGui.BeginChild("ConsoleOutput", new Vector2(0, -35), true);
            foreach (var line in _testingConsole.OutputHistory.TakeLast(20))
            {
                Vector4 textColor = line.StartsWith("✓") ? new Vector4(0.0f, 1.0f, 0.6f, 1.0f) :
                                   line.StartsWith("✗") || line.StartsWith("Error") ? new Vector4(1.0f, 0.3f, 0.3f, 1.0f) :
                                   line.StartsWith(">") ? new Vector4(0.0f, 0.9f, 1.0f, 1.0f) :
                                   new Vector4(0.9f, 0.9f, 0.9f, 1.0f);
                ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.Text, textColor);
                ImGuiNET.ImGui.TextUnformatted(line);
                ImGuiNET.ImGui.PopStyleColor();
            }
            // Auto-scroll to bottom
            if (ImGuiNET.ImGui.GetScrollY() >= ImGuiNET.ImGui.GetScrollMaxY())
                ImGuiNET.ImGui.SetScrollHereY(1.0f);
            ImGuiNET.ImGui.EndChild();
            
            ImGuiNET.ImGui.Spacing();
            
            // Input field with styling
            ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.Text, new Vector4(0.0f, 1.0f, 1.0f, 1.0f));
            ImGuiNET.ImGui.Text($"> {_consoleInput}");
            ImGuiNET.ImGui.SameLine();
            ImGuiNET.ImGui.PushStyleColor(ImGuiNET.ImGuiCol.Text, new Vector4(0.0f, 0.9f, 1.0f, 0.5f));
            ImGuiNET.ImGui.Text("_");
            ImGuiNET.ImGui.PopStyleColor(2);
        }
        ImGuiNET.ImGui.End();
        ImGuiNET.ImGui.PopStyleVar();
        ImGuiNET.ImGui.PopStyleColor(4);
    }

    private void OnKeyDown(IKeyboard keyboard, Key key, int keyCode)
    {
        _keysPressed.Add(key);
        
        // Debug toggle keys F7-F10 (note: F9 is for quick load, so moved to F11)
        if (key == Key.F7)
        {
            Config.DebugConfig.TwoSidedRendering = !Config.DebugConfig.TwoSidedRendering;
            Console.WriteLine($"[DEBUG] Two-Sided Rendering: {(Config.DebugConfig.TwoSidedRendering ? "ON" : "OFF")}");
            Console.WriteLine("  This disables backface culling to render both sides of triangles");
            Console.WriteLine("  Useful for debugging missing faces while preserving hollow structures");
        }
        else if (key == Key.F8)
        {
            Config.DebugConfig.BypassCulling = !Config.DebugConfig.BypassCulling;
            Console.WriteLine($"[DEBUG] Bypass Culling: {(Config.DebugConfig.BypassCulling ? "ON" : "OFF")}");
            Console.WriteLine("  This forces all chunks to render regardless of camera position");
        }
        else if (key == Key.F11)
        {
            Config.DebugConfig.ShowAABBs = !Config.DebugConfig.ShowAABBs;
            Console.WriteLine($"[DEBUG] Show AABBs: {(Config.DebugConfig.ShowAABBs ? "ON" : "OFF")}");
            Console.WriteLine("  This shows wireframe bounding boxes for all chunks");
        }
        else if (key == Key.F12)
        {
            Config.DebugConfig.ShowGenStats = !Config.DebugConfig.ShowGenStats;
            Console.WriteLine($"[DEBUG] Show Generation Stats: {(Config.DebugConfig.ShowGenStats ? "ON" : "OFF")}");
            Console.WriteLine("  This displays world generation task/result counts on screen");
        }
        
        // Track Shift for console input
        if (key == Key.ShiftLeft || key == Key.ShiftRight)
        {
            _consoleShiftPressed = true;
        }
        
        // Toggle testing console with tilde (~)
        if (key == Key.GraveAccent)
        {
            _testingConsole?.Toggle();
            if (_testingConsole?.IsVisible == true)
            {
                _consoleInput = "";
                Console.WriteLine("Testing Console opened. Type 'help' for available commands.");
            }
            return; // Don't process other inputs when toggling console
        }
        
        // Handle console input when console is visible
        if (_testingConsole != null && _testingConsole.IsVisible)
        {
            HandleConsoleInput(key, keyCode);
            return; // Don't process other inputs when console is open
        }
        
        // Pass to player control system
        _playerControlSystem?.OnKeyDown(key);

        // Toggle control mode
        if (key == Key.C)
        {
            _playerControlMode = !_playerControlMode;
            Console.WriteLine($"Control Mode: {(_playerControlMode ? "Ship Control (Third-Person)" : "Free Camera")}");
        }
        
        // Toggle debug UI with F1
        if (key == Key.F1)
        {
            _showDebugUI = !_showDebugUI;
            Console.WriteLine($"Debug HUD: {(_showDebugUI ? "Shown" : "Hidden")}");
        }
        
        // Quick Save with F5
        if (key == Key.F5)
        {
            Console.WriteLine("Quick saving...");
            bool success = _gameEngine?.QuickSave() ?? false;
            if (success)
            {
                Console.WriteLine("✓ Quick save completed successfully");
            }
            else
            {
                Console.WriteLine("✗ Quick save failed");
            }
        }
        
        // Quick Load with F9
        if (key == Key.F9)
        {
            Console.WriteLine("Quick loading...");
            bool success = _gameEngine?.QuickLoad() ?? false;
            if (success)
            {
                Console.WriteLine("✓ Quick load completed successfully");
            }
            else
            {
                Console.WriteLine("✗ Quick load failed");
            }
        }
        
        // Handle ESC for pause menu (or close console if open)
        if (key == Key.Escape)
        {
            if (_testingConsole != null && _testingConsole.IsVisible)
            {
                _testingConsole.IsVisible = false;
            }
            else
            {
                _gameMenuSystem?.TogglePauseMenu();
            }
        }
    }
    
    private void HandleConsoleInput(Key key, int keyCode)
    {
        if (_testingConsole == null) return;
        
        if (key == Key.Enter)
        {
            // Execute command
            _testingConsole.ExecuteCommand(_consoleInput);
            _consoleInput = "";
        }
        else if (key == Key.Backspace)
        {
            // Remove last character
            if (_consoleInput.Length > 0)
            {
                _consoleInput = _consoleInput.Substring(0, _consoleInput.Length - 1);
            }
        }
        else if (key == Key.Space)
        {
            _consoleInput += " ";
        }
        else if (key == Key.Escape)
        {
            // Clear input on escape
            _consoleInput = "";
        }
        else
        {
            // Convert Key enum to character
            char? c = KeyToChar(key, _consoleShiftPressed);
            if (c.HasValue)
            {
                _consoleInput += c.Value;
            }
        }
    }
    
    /// <summary>
    /// Convert Silk.NET Key enum to character, respecting shift state
    /// </summary>
    private char? KeyToChar(Key key, bool shiftPressed)
    {
        // Letters A-Z
        if (key >= Key.A && key <= Key.Z)
        {
            char baseChar = (char)('a' + (key - Key.A));
            return shiftPressed ? char.ToUpper(baseChar) : baseChar;
        }
        
        // Numbers 0-9 (with shift for special characters)
        if (key >= Key.Number0 && key <= Key.Number9)
        {
            if (shiftPressed)
            {
                // Shift + number keys produce special characters
                return key switch
                {
                    Key.Number1 => '!',
                    Key.Number2 => '@',
                    Key.Number3 => '#',
                    Key.Number4 => '$',
                    Key.Number5 => '%',
                    Key.Number6 => '^',
                    Key.Number7 => '&',
                    Key.Number8 => '*',
                    Key.Number9 => '(',
                    Key.Number0 => ')',
                    _ => null
                };
            }
            else
            {
                return (char)('0' + (key - Key.Number0));
            }
        }
        
        // Keypad numbers
        if (key >= Key.Keypad0 && key <= Key.Keypad9)
        {
            return (char)('0' + (key - Key.Keypad0));
        }
        
        // Special characters and symbols
        return key switch
        {
            Key.Minus => shiftPressed ? '_' : '-',
            Key.Equal => shiftPressed ? '+' : '=',
            Key.LeftBracket => shiftPressed ? '{' : '[',
            Key.RightBracket => shiftPressed ? '}' : ']',
            Key.Semicolon => shiftPressed ? ':' : ';',
            Key.Apostrophe => shiftPressed ? '"' : '\'',
            Key.Comma => shiftPressed ? '<' : ',',
            Key.Period => shiftPressed ? '>' : '.',
            Key.Slash => shiftPressed ? '?' : '/',
            Key.BackSlash => shiftPressed ? '|' : '\\',
            Key.GraveAccent => shiftPressed ? '~' : '`',
            Key.KeypadDecimal => '.',
            Key.KeypadDivide => '/',
            Key.KeypadMultiply => '*',
            Key.KeypadSubtract => '-',
            Key.KeypadAdd => '+',
            _ => null
        };
    }

    private void OnKeyUp(IKeyboard keyboard, Key key, int keyCode)
    {
        _keysPressed.Remove(key);
        
        // Track Shift for console input
        if (key == Key.ShiftLeft || key == Key.ShiftRight)
        {
            _consoleShiftPressed = false;
        }
        
        // Pass to player control system
        _playerControlSystem?.OnKeyUp(key);
    }

    private void OnMouseMove(IMouse mouse, Vector2 position)
    {
        if (_camera == null) return;
        
        // Always track mouse position for UI interaction
        _currentMousePos = position;
        
        // Don't process mouse movement if UI wants the mouse or ALT is held or menu is open
        if (_uiWantsMouse || _altKeyHeld || !_mouseLookEnabled) return;

        if (_firstMouse)
        {
            _lastMousePos = position;
            _firstMouse = false;
            return;
        }

        var xOffset = position.X - _lastMousePos.X;
        var yOffset = _lastMousePos.Y - position.Y; // Reversed since y-coordinates range from bottom to top

        _lastMousePos = position;

        _camera.ProcessMouseMovement(xOffset, yOffset);
    }
    
    private void OnMouseDown(IMouse mouse, MouseButton button)
    {
        _mouseButtonsPressed.Add(button);
        
        // Pass mouse clicks to menu system when menu is open
        if (_gameMenuSystem != null && _gameMenuSystem.IsMenuOpen)
        {
            _gameMenuSystem.HandleMouseClick(_currentMousePos, button);
        }
    }
    
    private void OnMouseUp(IMouse mouse, MouseButton button)
    {
        _mouseButtonsPressed.Remove(button);
    }

    private void OnClosing()
    {
        Dispose();
    }
    
    /// <summary>
    /// Create a background ship that slowly flies by on the title screen
    /// </summary>
    private void CreateBackgroundShip()
    {
        try
        {
            Console.WriteLine("Creating background ship for title screen...");
            
            var shipGenerator = new ProceduralShipGenerator(Environment.TickCount);
            
            // Generate a medium-sized combat ship with interesting details
            var shipConfig = new ShipGenerationConfig
            {
                Size = Procedural.ShipSize.Frigate,
                Role = Procedural.ShipRole.Combat,
                Material = "Titanium",
                Style = FactionShipStyle.GetDefaultStyle("Military"),
                Seed = 12345 // Fixed seed for consistent title screen ship
            };
            
            var generatedShip = shipGenerator.GenerateShip(shipConfig);
            var ship = _gameEngine.EntityManager.CreateEntity("TitleScreen_BackgroundShip");
            
            _gameEngine.EntityManager.AddComponent(ship.Id, generatedShip.Structure);
            
            // Position ship to fly across the screen
            // Start off to the right side, will move left
            var shipPhysics = new PhysicsComponent
            {
                Position = new Vector3(200, 0, -300), // Start to the right and back
                Velocity = new Vector3(-5, 0, 0), // Slow movement to the left
                Mass = generatedShip.Structure.TotalMass,
                MomentOfInertia = generatedShip.Structure.MomentOfInertia,
                MaxThrust = generatedShip.Structure.TotalThrust,
                MaxTorque = generatedShip.Structure.TotalTorque,
                // Slight rotation for visual interest
                AngularVelocity = new Vector3(0, 0.1f, 0)
            };
            _gameEngine.EntityManager.AddComponent(ship.Id, shipPhysics);
            
            _backgroundShipId = ship.Id;
            
            Console.WriteLine($"✓ Background ship created with {generatedShip.Structure.Blocks.Count} blocks");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Warning: Failed to create background ship: {ex.Message}");
            // Not critical, continue without background ship
        }
    }
    
    /// <summary>
    /// Renders a modular ship using 3D models from its modules
    /// </summary>
    private void RenderModularShip(ModularShipComponent ship, Guid entityId, float aspectRatio)
    {
        if (_meshRenderer == null || _camera == null || _gl == null)
            return;
        
        // Get ship position from physics component
        Vector3 shipPosition = Vector3.Zero;
        var physicsComponent = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entityId);
        if (physicsComponent != null)
        {
            shipPosition = physicsComponent.InterpolatedPosition;
        }
        
        var viewMatrix = _camera.GetViewMatrix();
        var projectionMatrix = _camera.GetProjectionMatrix(aspectRatio);
        
        // Get module library to resolve definitions
        var moduleLibrary = new ModuleLibrary();
        moduleLibrary.InitializeBuiltInModules();
        
        // Render each module
        foreach (var module in ship.Modules)
        {
            try
            {
                // Get module definition
                var definition = moduleLibrary.GetDefinition(module.ModuleDefinitionId);
                
                // Try to load the 3D model from AssetManager
                List<MeshData> meshes;
                
                // If model path is empty or null, use placeholder cube
                if (definition == null || string.IsNullOrEmpty(definition.ModelPath))
                {
                    // Use placeholder cube
                    var cube = AssetManager.Instance.CreatePlaceholderCube(2.0f);
                    meshes = new List<MeshData> { cube };
                }
                else
                {
                    // Try to load actual model
                    try
                    {
                        meshes = AssetManager.Instance.LoadModel(definition.ModelPath);
                    }
                    catch
                    {
                        // Fall back to placeholder if model not found
                        var cube = AssetManager.Instance.CreatePlaceholderCube(2.0f);
                        meshes = new List<MeshData> { cube };
                    }
                }
                
                // Create transform matrix for this module
                var transform = CreateModuleTransformMatrix(module, shipPosition);
                
                // Determine module color (could be from material type or ship color)
                var color = GetModuleColor(module);
                
                // Render all meshes in this module
                foreach (var mesh in meshes)
                {
                    _meshRenderer.RenderMesh(
                        mesh,
                        transform,
                        color,
                        viewMatrix,
                        projectionMatrix,
                        _camera.Position
                    );
                }
            }
            catch (Exception ex)
            {
                // Log error but continue rendering other modules
                Console.WriteLine($"Error rendering module {module.Id}: {ex.Message}");
            }
        }
    }
    
    /// <summary>
    /// Creates a transformation matrix for a module
    /// </summary>
    private Matrix4x4 CreateModuleTransformMatrix(ShipModulePart module, Vector3 shipPosition)
    {
        // Create scale matrix (modules can have different sizes)
        var scale = Matrix4x4.CreateScale(1.0f); // Default scale
        
        // Create rotation matrix from module rotation (Euler angles in degrees)
        var rotationX = Matrix4x4.CreateRotationX(module.Rotation.X * (float)Math.PI / 180.0f);
        var rotationY = Matrix4x4.CreateRotationY(module.Rotation.Y * (float)Math.PI / 180.0f);
        var rotationZ = Matrix4x4.CreateRotationZ(module.Rotation.Z * (float)Math.PI / 180.0f);
        var rotation = rotationZ * rotationY * rotationX; // ZYX order
        
        // Create translation matrix (module position + ship position)
        var translation = Matrix4x4.CreateTranslation(module.Position + shipPosition);
        
        // Combine: Scale * Rotation * Translation
        return scale * rotation * translation;
    }
    
    /// <summary>
    /// Gets the color for a module based on its material or properties
    /// </summary>
    private Vector3 GetModuleColor(ShipModulePart module)
    {
        // Default colors based on material type
        return module.MaterialType?.ToLower() switch
        {
            "iron" => new Vector3(0.7f, 0.7f, 0.7f),      // Gray
            "titanium" => new Vector3(0.8f, 0.8f, 0.9f),  // Light blue-gray
            "naonite" => new Vector3(0.3f, 0.6f, 0.8f),   // Blue
            "trinium" => new Vector3(0.5f, 0.8f, 0.5f),   // Green
            "xanion" => new Vector3(0.9f, 0.7f, 0.3f),    // Gold
            "ogonite" => new Vector3(0.8f, 0.4f, 0.2f),   // Orange
            "avorion" => new Vector3(0.9f, 0.3f, 0.9f),   // Purple
            _ => new Vector3(0.6f, 0.6f, 0.6f)            // Default gray
        };
    }

    public void Dispose()
    {
        if (!_disposed)
        {
            _customUIRenderer?.Dispose();
            _imguiController?.Dispose();
            _voxelRenderer?.Dispose();
            _meshRenderer?.Dispose();  // NEW: Dispose mesh renderer
            _starfieldRenderer?.Dispose();
            _inputContext?.Dispose();
            _gl?.Dispose();
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}
