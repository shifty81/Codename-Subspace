using Silk.NET.OpenGL;
using Silk.NET.Windowing;
using Silk.NET.Input;
using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Physics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Main graphics window for rendering the game world
/// Handles window creation, input, and rendering loop
/// </summary>
public class GraphicsWindow : IDisposable
{
    private IWindow? _window;
    private GL? _gl;
    private VoxelRenderer? _voxelRenderer;
    private Camera? _camera;
    private IInputContext? _inputContext;
    
    private readonly GameEngine _gameEngine;
    private bool _disposed = false;
    
    // Mouse state
    private Vector2 _lastMousePos;
    private bool _firstMouse = true;
    
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
        options.Title = "AvorionLike - 3D Voxel Viewer";
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

        // Initialize renderer
        _voxelRenderer = new VoxelRenderer(_gl);

        // Enable depth testing
        _gl.Enable(EnableCap.DepthTest);
        _gl.Enable(EnableCap.CullFace);

        // Set up input
        foreach (var keyboard in _inputContext.Keyboards)
        {
            keyboard.KeyDown += OnKeyDown;
            keyboard.KeyUp += OnKeyUp;
        }

        foreach (var mouse in _inputContext.Mice)
        {
            mouse.MouseMove += OnMouseMove;
            mouse.Cursor.CursorMode = CursorMode.Disabled;
        }

        Console.WriteLine("\n=== 3D Graphics Window Active ===");
        Console.WriteLine("Controls:");
        Console.WriteLine("  WASD - Move camera");
        Console.WriteLine("  Space/Shift - Move up/down");
        Console.WriteLine("  Mouse - Look around");
        Console.WriteLine("  ESC - Exit");
        Console.WriteLine("=====================================\n");
    }

    private void OnUpdate(double deltaTime)
    {
        _deltaTime = (float)deltaTime;

        if (_camera == null) return;

        // Process keyboard input
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

        // Update game engine
        _gameEngine.Update();
    }

    private void OnRender(double deltaTime)
    {
        if (_gl == null || _voxelRenderer == null || _camera == null || _window == null) return;

        // Clear the screen
        _gl.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);
        _gl.ClearColor(0.1f, 0.1f, 0.15f, 1.0f);

        // Calculate aspect ratio from window size
        float aspectRatio = (float)_window.Size.X / _window.Size.Y;

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
            _voxelRenderer?.Dispose();
            _inputContext?.Dispose();
            _gl?.Dispose();
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}
