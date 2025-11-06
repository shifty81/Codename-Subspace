using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Input;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.UI;
using AvorionLike.Core.Input;

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
    private PlayerUIManager? _playerUIManager;
    private PlayerControlSystem? _playerControlSystem;
    
    // Individual UI systems (managed by PlayerUIManager)
    private HUDSystem? _hudSystem;
    private MenuSystem? _menuSystem;
    private InventoryUI? _inventoryUI;
    private ShipBuilderUI? _shipBuilderUI;
    private FuturisticHUD? _futuristicHUD;
    private CrewManagementUI? _crewManagementUI;
    private SubsystemManagementUI? _subsystemManagementUI;
    private FleetMissionUI? _fleetMissionUI;
    
    private readonly GameEngine _gameEngine;
    private bool _disposed = false;
    private bool _playerControlMode = false; // Toggle between camera and ship control
    
    // Mouse state
    private Vector2 _lastMousePos;
    private bool _firstMouse = true;
    private bool _uiWantsMouse = false;
    
    // Timing
    private float _deltaTime = 0.0f;

    // Input state
    private readonly HashSet<Key> _keysPressed = new();

    public GraphicsWindow(GameEngine gameEngine)
    {
        _gameEngine = gameEngine;
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
        if (_playerUIManager != null)
        {
            _playerUIManager.PlayerShipId = shipId;
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

        // Initialize ImGui
        _imguiController = new ImGuiController(_gl, _window!, _inputContext);
        _hudSystem = new HUDSystem(_gameEngine);
        _menuSystem = new MenuSystem(_gameEngine);
        _inventoryUI = new InventoryUI(_gameEngine);
        _shipBuilderUI = new ShipBuilderUI(_gameEngine);
        _futuristicHUD = new FuturisticHUD(_gameEngine);
        _crewManagementUI = new CrewManagementUI(_gameEngine);
        _subsystemManagementUI = new SubsystemManagementUI(_gameEngine);
        _fleetMissionUI = new FleetMissionUI(_gameEngine);
        
        // Initialize Player UI Manager
        _playerUIManager = new PlayerUIManager(
            _gameEngine,
            _hudSystem,
            _menuSystem,
            _inventoryUI,
            _shipBuilderUI,
            _futuristicHUD,
            _crewManagementUI,
            _subsystemManagementUI,
            _fleetMissionUI
        );
        
        // Initialize Player Control System
        _playerControlSystem = new PlayerControlSystem(_gameEngine.EntityManager);

        // Enable depth testing
        _gl.Enable(EnableCap.DepthTest);
        
        // Disable face culling for voxel blocks (they should be visible from all angles)
        // TODO: Re-enable with correct CCW winding order for performance optimization
        _gl.Disable(EnableCap.CullFace);

        // Set up input
        foreach (var keyboard in _inputContext.Keyboards)
        {
            keyboard.KeyDown += OnKeyDown;
            keyboard.KeyUp += OnKeyUp;
        }

        foreach (var mouse in _inputContext.Mice)
        {
            mouse.MouseMove += OnMouseMove;
        }

        Console.WriteLine("\n=== 3D Graphics Window Active ===");
        Console.WriteLine("Controls:");
        Console.WriteLine("  Camera Mode (Default):");
        Console.WriteLine("    WASD - Move camera");
        Console.WriteLine("    Space/Shift - Move up/down");
        Console.WriteLine("    Mouse - Look around");
        Console.WriteLine("  Ship Control Mode (Press C to toggle):");
        Console.WriteLine("    WASD - Thrust (Forward/Back/Left/Right)");
        Console.WriteLine("    Space/Shift - Thrust Up/Down");
        Console.WriteLine("    Arrow Keys - Pitch/Yaw");
        Console.WriteLine("    Q/E - Roll");
        Console.WriteLine("    X - Emergency Brake");
        Console.WriteLine("  UI Controls:");
        Console.WriteLine("    F1/F2/F3 - Toggle debug panels");
        Console.WriteLine("    F4 - Toggle Futuristic HUD");
        Console.WriteLine("    I - Toggle Inventory");
        Console.WriteLine("    B - Toggle Ship Builder");
        Console.WriteLine("    TAB - Toggle Player Status");
        Console.WriteLine("    J - Toggle Mission Info");
        Console.WriteLine("    ESC - Exit");
        Console.WriteLine("=====================================\n");
    }

    private void OnUpdate(double deltaTime)
    {
        _deltaTime = (float)deltaTime;

        if (_camera == null || _imguiController == null || _playerUIManager == null || _playerControlSystem == null) return;

        // Update ImGui
        _imguiController.Update(_deltaTime);
        
        // Check if ImGui wants mouse input
        var io = ImGuiNET.ImGui.GetIO();
        _uiWantsMouse = io.WantCaptureMouse;

        // Process keyboard input
        bool anyUIOpen = _playerUIManager.IsAnyPanelOpen;
        
        if (!io.WantCaptureKeyboard && !anyUIOpen)
        {
            if (_playerControlMode && _playerControlSystem.ControlledShipId.HasValue)
            {
                // Ship control mode
                _playerControlSystem.Update(_deltaTime);
                
                // Follow player ship with camera
                var physics = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(_playerControlSystem.ControlledShipId.Value);
                if (physics != null)
                {
                    // Position camera behind and above the ship
                    _camera.Position = physics.Position + new Vector3(-50, 30, 50);
                    // TODO: Implement proper chase camera
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
        
        // Handle UI input
        _playerUIManager.HandleInput();

        // Update game engine (pause if menu is open)
        if (!anyUIOpen)
        {
            _gameEngine.Update();
        }
        
        _playerUIManager.Update(_deltaTime);
    }

    private void OnRender(double deltaTime)
    {
        if (_gl == null || _voxelRenderer == null || _starfieldRenderer == null || _camera == null || 
            _window == null || _imguiController == null || _playerUIManager == null) return;

        // Clear the screen
        _gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);
        _gl.ClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Pure black for space

        // Calculate aspect ratio from window size
        float aspectRatio = (float)_window.Size.X / _window.Size.Y;

        // Render starfield background first (without depth write)
        _starfieldRenderer.Render(_camera, aspectRatio);

        // Render all entities with voxel structures
        var entities = _gameEngine.EntityManager.GetAllEntities();
        foreach (var entity in entities)
        {
            var voxelComponent = _gameEngine.EntityManager.GetComponent<VoxelStructureComponent>(entity.Id);
            if (voxelComponent != null)
            {
                // Get entity position from physics component if available
                Vector3 position = Vector3.Zero;
                var physicsComponent = _gameEngine.EntityManager.GetComponent<PhysicsComponent>(entity.Id);
                if (physicsComponent != null)
                {
                    position = physicsComponent.Position;
                }

                _voxelRenderer.RenderVoxelStructure(voxelComponent, _camera, position, aspectRatio);
            }
        }
        
        // Render Player UI Manager (handles all UI panels)
        _playerUIManager.Render();
        
        _imguiController.Render();
    }

    private void OnKeyDown(IKeyboard keyboard, Key key, int keyCode)
    {
        _keysPressed.Add(key);
        
        // Pass to player control system
        _playerControlSystem?.OnKeyDown(key);

        // Toggle control mode
        if (key == Key.C)
        {
            _playerControlMode = !_playerControlMode;
            Console.WriteLine($"Control Mode: {(_playerControlMode ? "Ship Control" : "Camera")}");
        }

        if (key == Key.Escape)
        {
            _window?.Close();
        }
    }

    private void OnKeyUp(IKeyboard keyboard, Key key, int keyCode)
    {
        _keysPressed.Remove(key);
        
        // Pass to player control system
        _playerControlSystem?.OnKeyUp(key);
    }

    private void OnMouseMove(IMouse mouse, Vector2 position)
    {
        if (_camera == null) return;
        
        // Don't process mouse movement if UI wants the mouse
        if (_uiWantsMouse) return;

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

    private void OnClosing()
    {
        Dispose();
    }

    public void Dispose()
    {
        if (!_disposed)
        {
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
