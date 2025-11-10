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
    private StarfieldRenderer? _starfieldRenderer;
    private Camera? _camera;
    private IInputContext? _inputContext;
    private ImGuiController? _imguiController;
    private PlayerControlSystem? _playerControlSystem;
    
    // Custom UI system (for game HUD and menus)
    private CustomUIRenderer? _customUIRenderer;
    private GameHUD? _gameHUD;
    private GameMenuSystem? _gameMenuSystem;
    
    // ImGui-based UI systems (for debug/console ONLY)
    private HUDSystem? _debugHUD;  // Renamed to clarify it's for debug
    private bool _showDebugUI = true;  // Toggle for debug UI - enabled by default for better UX
    
    // In-game testing console
    private InGameTestingConsole? _testingConsole;
    private string _consoleInput = "";
    private bool _consoleShiftPressed = false;
    
    private readonly GameEngine _gameEngine;
    private bool _disposed = false;
    private bool _playerControlMode = false; // Toggle between camera and ship control
    private bool _shouldClose = false; // Signal to close window and return to main menu
    
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

        // Initialize camera
        _camera = new Camera(new Vector3(0, 50, 150));

        // Initialize renderers
        _voxelRenderer = new EnhancedVoxelRenderer(_gl);
        _starfieldRenderer = new StarfieldRenderer(_gl);

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
        
        // Initialize In-Game Testing Console
        _testingConsole = new InGameTestingConsole(_gameEngine);
        
        // Initialize Player Control System
        _playerControlSystem = new PlayerControlSystem(_gameEngine.EntityManager);

        // Enable depth testing
        _gl.Enable(EnableCap.DepthTest);
        
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
        Console.WriteLine("  Camera Mode (Default):");
        Console.WriteLine("    WASD - Move camera");
        Console.WriteLine("    Space/Shift - Move up/down");
        Console.WriteLine("    Mouse - Look around (free-look)");
        Console.WriteLine("  Ship Control Mode (Press C to toggle):");
        Console.WriteLine("    WASD - Thrust (Forward/Back/Left/Right)");
        Console.WriteLine("    Space/Shift - Thrust Up/Down");
        Console.WriteLine("    Arrow Keys - Pitch/Yaw");
        Console.WriteLine("    Q/E - Roll");
        Console.WriteLine("    X - Emergency Brake");
        Console.WriteLine("  UI Controls:");
        Console.WriteLine("    ~ (Tilde) - Toggle In-Game Testing Console");
        Console.WriteLine("    ALT - Show mouse cursor (hold, doesn't affect free-look)");
        Console.WriteLine("    ESC - Pause Menu (press again to close)");
        Console.WriteLine("    F1 - Toggle Debug HUD (enabled by default)");
        Console.WriteLine("    F2 - Toggle Entity List");
        Console.WriteLine("    F3 - Toggle Resource Panel");
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
        
        // Update mouse cursor mode based on state
        foreach (var mouse in _inputContext.Mice)
        {
            if (menuOpen || _altKeyHeld)
            {
                // Show cursor when menu is open or ALT is held
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
        bool anyUIOpen = menuOpen;
        
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

        // Clear the screen
        _gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);
        _gl.ClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Pure black for space

        // Calculate aspect ratio from window size
        float aspectRatio = (float)_window.Size.X / _window.Size.Y;

        // Interpolate physics for smooth rendering (use deltaTime as alpha)
        // This provides smooth motion between physics updates
        float alpha = Math.Clamp(_deltaTime * 60f, 0f, 1f); // Assume 60 FPS target
        _gameEngine.PhysicsSystem.InterpolatePhysics(alpha);

        // Render starfield background first (without depth write)
        _starfieldRenderer.Render(_camera, aspectRatio);

        // Render all entities with voxel structures
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
        
        // Always render ImGui (needed for GameHUD text and debug UI when enabled)
        _imguiController.Render();
    }
    
    private void RenderTestingConsole()
    {
        if (_testingConsole == null || _window == null) return;
        
        // Create console window using ImGui
        ImGuiNET.ImGui.SetNextWindowPos(new Vector2(10, _window.Size.Y - 310));
        ImGuiNET.ImGui.SetNextWindowSize(new Vector2(_window.Size.X - 20, 300));
        
        if (ImGuiNET.ImGui.Begin("In-Game Testing Console", ImGuiNET.ImGuiWindowFlags.NoCollapse | ImGuiNET.ImGuiWindowFlags.NoMove | ImGuiNET.ImGuiWindowFlags.NoResize))
        {
            // Output history
            ImGuiNET.ImGui.BeginChild("ConsoleOutput", new Vector2(0, -30), true);
            foreach (var line in _testingConsole.OutputHistory.TakeLast(20))
            {
                ImGuiNET.ImGui.TextUnformatted(line);
            }
            // Auto-scroll to bottom
            if (ImGuiNET.ImGui.GetScrollY() >= ImGuiNET.ImGui.GetScrollMaxY())
                ImGuiNET.ImGui.SetScrollHereY(1.0f);
            ImGuiNET.ImGui.EndChild();
            
            // Input field
            ImGuiNET.ImGui.Text($"> {_consoleInput}");
            ImGuiNET.ImGui.SameLine();
            ImGuiNET.ImGui.Text("_");
        }
        ImGuiNET.ImGui.End();
    }

    private void OnKeyDown(IKeyboard keyboard, Key key, int keyCode)
    {
        _keysPressed.Add(key);
        
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
            Console.WriteLine($"Control Mode: {(_playerControlMode ? "Ship Control" : "Camera")}");
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
        else if (keyCode >= 32 && keyCode < 127)
        {
            // Add printable character
            char c = (char)keyCode;
            // Handle Shift for uppercase
            if (_consoleShiftPressed)
            {
                c = char.ToUpper(c);
            }
            else
            {
                c = char.ToLower(c);
            }
            _consoleInput += c;
        }
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

    public void Dispose()
    {
        if (!_disposed)
        {
            _customUIRenderer?.Dispose();
            _imguiController?.Dispose();
            _voxelRenderer?.Dispose();
            _starfieldRenderer?.Dispose();
            _inputContext?.Dispose();
            _gl?.Dispose();
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}
