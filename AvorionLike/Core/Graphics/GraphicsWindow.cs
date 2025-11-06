using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Input;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;
using AvorionLike.Core.UI;

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
    private HUDSystem? _hudSystem;
    private MenuSystem? _menuSystem;
    private InventoryUI? _inventoryUI;
    private ShipBuilderUI? _shipBuilderUI;
    private FuturisticHUD? _futuristicHUD;
    
    private readonly GameEngine _gameEngine;
    private bool _disposed = false;
    
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
        Console.WriteLine("  WASD - Move camera");
        Console.WriteLine("  Space/Shift - Move up/down");
        Console.WriteLine("  Mouse - Look around");
        Console.WriteLine("  F1/F2/F3 - Toggle UI panels");
        Console.WriteLine("  F4 - Toggle Futuristic HUD");
        Console.WriteLine("  I - Toggle Inventory");
        Console.WriteLine("  B - Toggle Ship Builder");
        Console.WriteLine("  ESC - Exit");
        Console.WriteLine("=====================================\n");
    }

    private void OnUpdate(double deltaTime)
    {
        _deltaTime = (float)deltaTime;

        if (_camera == null || _imguiController == null || _hudSystem == null || _menuSystem == null || _inventoryUI == null || _shipBuilderUI == null || _futuristicHUD == null) return;

        // Update ImGui
        _imguiController.Update(_deltaTime);
        
        // Check if ImGui wants mouse input
        var io = ImGuiNET.ImGui.GetIO();
        _uiWantsMouse = io.WantCaptureMouse;

        // Process keyboard input for camera (only if UI doesn't want it and menu is not open)
        bool anyUIOpen = _menuSystem.IsMenuOpen || _inventoryUI.IsOpen || _shipBuilderUI.IsOpen;
        if (!io.WantCaptureKeyboard && !anyUIOpen)
        {
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
        
        // Handle UI and menu input
        _hudSystem.HandleInput();
        _menuSystem.HandleInput();
        _inventoryUI.HandleInput();
        _shipBuilderUI.HandleInput();
        _futuristicHUD.HandleInput();

        // Update game engine (pause if menu is open)
        if (!anyUIOpen)
        {
            _gameEngine.Update();
        }
    }

    private void OnRender(double deltaTime)
    {
        if (_gl == null || _voxelRenderer == null || _starfieldRenderer == null || _camera == null || _window == null || _imguiController == null || _hudSystem == null || _menuSystem == null || _inventoryUI == null || _shipBuilderUI == null || _futuristicHUD == null) return;

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
        
        // Render ImGui UI on top
        if (_menuSystem.IsMenuOpen)
        {
            // Only render menu when menu is open
            _menuSystem.Render();
        }
        else
        {
            // Render HUD when menu is closed
            _hudSystem.Render();
            
            // Render futuristic HUD if enabled
            _futuristicHUD.Render();
            
            // Render inventory if open
            _inventoryUI.Render();
            
            // Render ship builder if open
            _shipBuilderUI.Render();
        }
        
        _imguiController.Render();
    }

    private void OnKeyDown(IKeyboard keyboard, Key key, int keyCode)
    {
        _keysPressed.Add(key);

        if (key == Key.Escape)
        {
            _window?.Close();
        }
    }

    private void OnKeyUp(IKeyboard keyboard, Key key, int keyCode)
    {
        _keysPressed.Remove(key);
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
