using System.Numerics;
using Silk.NET.OpenGL;
using AvorionLike.Core.Graphics;

namespace AvorionLike.Core.DevTools;

/// <summary>
/// Debug Renderer - Provides debug visualization capabilities for game objects and physics
/// </summary>
public class DebugRenderer : IDisposable
{
    private readonly GL? _gl;
    private List<DebugLine> lines = new();
    private List<DebugBox> boxes = new();
    private bool isEnabled = true;
    private Graphics.Shader? _shader;
    private uint _vao;
    private uint _vbo;
    private bool _disposed = false;

    public bool IsEnabled
    {
        get => isEnabled;
        set => isEnabled = value;
    }
    
    public DebugRenderer()
    {
        // For console-only mode
        _gl = null;
    }
    
    public DebugRenderer(GL gl)
    {
        _gl = gl;
        InitializeBuffers();
        InitializeShader();
    }
    
    private unsafe void InitializeBuffers()
    {
        if (_gl == null) return;
        
        _vao = _gl.GenVertexArray();
        _gl.BindVertexArray(_vao);

        _vbo = _gl.GenBuffer();
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);

        // Position attribute
        _gl.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 6 * sizeof(float), (void*)0);
        _gl.EnableVertexAttribArray(0);

        // Color attribute
        _gl.VertexAttribPointer(1, 3, VertexAttribPointerType.Float, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        _gl.EnableVertexAttribArray(1);

        _gl.BindVertexArray(0);
    }
    
    private void InitializeShader()
    {
        if (_gl == null) return;
        
        string vertexShader = @"
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;

out vec3 Color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    Color = aColor;
    gl_Position = projection * view * vec4(aPosition, 1.0);
}
";

        string fragmentShader = @"
#version 330 core
out vec4 FragColor;

in vec3 Color;

void main()
{
    FragColor = vec4(Color, 1.0);
}
";

        _shader = new Graphics.Shader(_gl, vertexShader, fragmentShader);
    }

    /// <summary>
    /// Draw a debug line between two points
    /// </summary>
    public void DrawLine(Vector3 start, Vector3 end, string color = "White", float duration = 0f)
    {
        if (!isEnabled) return;
        lines.Add(new DebugLine { Start = start, End = end, Color = color, Duration = duration });
    }

    /// <summary>
    /// Draw a debug box at a position with given size
    /// </summary>
    public void DrawBox(Vector3 position, Vector3 size, string color = "Green", float duration = 0f)
    {
        if (!isEnabled) return;
        boxes.Add(new DebugBox { Position = position, Size = size, Color = color, Duration = duration });
    }
    
    /// <summary>
    /// Draw wireframe AABB (axis-aligned bounding box)
    /// </summary>
    public void DrawAABB(Vector3 min, Vector3 max, string color = "Yellow", float duration = 0f)
    {
        if (!isEnabled) return;
        
        // Bottom face
        DrawLine(new Vector3(min.X, min.Y, min.Z), new Vector3(max.X, min.Y, min.Z), color, duration);
        DrawLine(new Vector3(max.X, min.Y, min.Z), new Vector3(max.X, min.Y, max.Z), color, duration);
        DrawLine(new Vector3(max.X, min.Y, max.Z), new Vector3(min.X, min.Y, max.Z), color, duration);
        DrawLine(new Vector3(min.X, min.Y, max.Z), new Vector3(min.X, min.Y, min.Z), color, duration);
        
        // Top face
        DrawLine(new Vector3(min.X, max.Y, min.Z), new Vector3(max.X, max.Y, min.Z), color, duration);
        DrawLine(new Vector3(max.X, max.Y, min.Z), new Vector3(max.X, max.Y, max.Z), color, duration);
        DrawLine(new Vector3(max.X, max.Y, max.Z), new Vector3(min.X, max.Y, max.Z), color, duration);
        DrawLine(new Vector3(min.X, max.Y, max.Z), new Vector3(min.X, max.Y, min.Z), color, duration);
        
        // Vertical edges
        DrawLine(new Vector3(min.X, min.Y, min.Z), new Vector3(min.X, max.Y, min.Z), color, duration);
        DrawLine(new Vector3(max.X, min.Y, min.Z), new Vector3(max.X, max.Y, min.Z), color, duration);
        DrawLine(new Vector3(max.X, min.Y, max.Z), new Vector3(max.X, max.Y, max.Z), color, duration);
        DrawLine(new Vector3(min.X, min.Y, max.Z), new Vector3(min.X, max.Y, max.Z), color, duration);
    }

    /// <summary>
    /// Draw coordinate axes at a position
    /// </summary>
    public void DrawAxes(Vector3 position, float size = 1f)
    {
        DrawLine(position, position + new Vector3(size, 0, 0), "Red");
        DrawLine(position, position + new Vector3(0, size, 0), "Green");
        DrawLine(position, position + new Vector3(0, 0, size), "Blue");
    }

    /// <summary>
    /// Clear all debug visualizations
    /// </summary>
    public void Clear()
    {
        lines.Clear();
        boxes.Clear();
    }

    /// <summary>
    /// Update debug visualizations (removes expired items)
    /// </summary>
    public void Update(float deltaTime)
    {
        lines.RemoveAll(l => l.Duration > 0 && (l.Duration -= deltaTime) <= 0);
        boxes.RemoveAll(b => b.Duration > 0 && (b.Duration -= deltaTime) <= 0);
    }

    /// <summary>
    /// Render all debug visualizations
    /// </summary>
    public unsafe void Render(Matrix4x4 viewMatrix, Matrix4x4 projectionMatrix)
    {
        if (!isEnabled || _gl == null || _shader == null || lines.Count == 0) return;
        
        // Temporarily disable depth testing for debug lines so they're always visible
        bool depthTestEnabled = _gl.IsEnabled(EnableCap.DepthTest);
        _gl.Disable(EnableCap.DepthTest);
        
        _shader.Use();
        _shader.SetMatrix4("view", viewMatrix);
        _shader.SetMatrix4("projection", projectionMatrix);
        
        // Build vertex data for all lines
        List<float> vertices = new();
        
        foreach (var line in lines)
        {
            var color = GetColorVector(line.Color);
            
            // Start vertex
            vertices.Add(line.Start.X);
            vertices.Add(line.Start.Y);
            vertices.Add(line.Start.Z);
            vertices.Add(color.X);
            vertices.Add(color.Y);
            vertices.Add(color.Z);
            
            // End vertex
            vertices.Add(line.End.X);
            vertices.Add(line.End.Y);
            vertices.Add(line.End.Z);
            vertices.Add(color.X);
            vertices.Add(color.Y);
            vertices.Add(color.Z);
        }
        
        if (vertices.Count == 0)
        {
            if (depthTestEnabled) _gl.Enable(EnableCap.DepthTest);
            return;
        }
        
        // Upload vertex data
        _gl.BindVertexArray(_vao);
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);
        
        var vertexArray = vertices.ToArray();
        fixed (float* v = &vertexArray[0])
        {
            _gl.BufferData(BufferTargetARB.ArrayBuffer, (nuint)(vertexArray.Length * sizeof(float)), v, BufferUsageARB.DynamicDraw);
        }
        
        // Draw lines
        _gl.DrawArrays(PrimitiveType.Lines, 0, (uint)(vertices.Count / 6));
        
        _gl.BindVertexArray(0);
        
        // Restore depth testing
        if (depthTestEnabled) _gl.Enable(EnableCap.DepthTest);
    }
    
    private Vector3 GetColorVector(string colorName)
    {
        return colorName.ToLower() switch
        {
            "red" => new Vector3(1.0f, 0.0f, 0.0f),
            "green" => new Vector3(0.0f, 1.0f, 0.0f),
            "blue" => new Vector3(0.0f, 0.0f, 1.0f),
            "yellow" => new Vector3(1.0f, 1.0f, 0.0f),
            "cyan" => new Vector3(0.0f, 1.0f, 1.0f),
            "magenta" => new Vector3(1.0f, 0.0f, 1.0f),
            "white" => new Vector3(1.0f, 1.0f, 1.0f),
            "orange" => new Vector3(1.0f, 0.5f, 0.0f),
            _ => new Vector3(1.0f, 1.0f, 1.0f)
        };
    }

    public int GetLineCount() => lines.Count;
    public int GetBoxCount() => boxes.Count;
    
    public void Dispose()
    {
        if (!_disposed && _gl != null)
        {
            _gl.DeleteBuffer(_vbo);
            _gl.DeleteVertexArray(_vao);
            _shader?.Dispose();
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }

    private struct DebugLine
    {
        public Vector3 Start { get; set; }
        public Vector3 End { get; set; }
        public string Color { get; set; }
        public float Duration { get; set; }
    }

    private struct DebugBox
    {
        public Vector3 Position { get; set; }
        public Vector3 Size { get; set; }
        public string Color { get; set; }
        public float Duration { get; set; }
    }
}
