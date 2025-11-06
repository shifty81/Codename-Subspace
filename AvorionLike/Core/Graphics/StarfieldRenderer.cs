using Silk.NET.OpenGL;
using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Renders a procedural starfield background for space atmosphere
/// </summary>
public class StarfieldRenderer : IDisposable
{
    private readonly GL _gl;
    private Shader? _shader;
    private uint _vao;
    private uint _vbo;
    private readonly List<Star> _stars = new();
    private const int StarCount = 5000;
    private bool _disposed = false;

    public StarfieldRenderer(GL gl, int seed = 42)
    {
        _gl = gl;
        GenerateStars(seed);
        InitializeBuffers();
        InitializeShader();
    }

    private void GenerateStars(int seed)
    {
        var random = new Random(seed);
        
        for (int i = 0; i < StarCount; i++)
        {
            // Random position on a large sphere
            float theta = (float)(random.NextDouble() * Math.PI * 2);
            float phi = (float)(random.NextDouble() * Math.PI);
            float radius = 500f; // Far from camera

            var position = new Vector3(
                radius * MathF.Sin(phi) * MathF.Cos(theta),
                radius * MathF.Sin(phi) * MathF.Sin(theta),
                radius * MathF.Cos(phi)
            );

            // Random star properties
            float brightness = (float)(random.NextDouble() * 0.5 + 0.5); // 0.5 to 1.0
            float size = (float)(random.NextDouble() * 1.5 + 0.5); // 0.5 to 2.0
            
            // Star colors: white, blue-white, or yellow-white
            Vector3 color = random.Next(10) switch
            {
                < 7 => new Vector3(1.0f, 1.0f, 1.0f),           // White (70%)
                < 9 => new Vector3(0.9f, 0.95f, 1.0f),          // Blue-white (20%)
                _ => new Vector3(1.0f, 0.95f, 0.8f)             // Yellow-white (10%)
            };

            _stars.Add(new Star
            {
                Position = position,
                Color = color * brightness,
                Size = size
            });
        }
    }

    private unsafe void InitializeBuffers()
    {
        // Create vertex data (position + color + size)
        var vertices = new List<float>();
        
        foreach (var star in _stars)
        {
            vertices.Add(star.Position.X);
            vertices.Add(star.Position.Y);
            vertices.Add(star.Position.Z);
            vertices.Add(star.Color.X);
            vertices.Add(star.Color.Y);
            vertices.Add(star.Color.Z);
            vertices.Add(star.Size);
        }

        var vertexArray = vertices.ToArray();

        _vao = _gl.GenVertexArray();
        _gl.BindVertexArray(_vao);

        _vbo = _gl.GenBuffer();
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);
        
        fixed (float* v = &vertexArray[0])
        {
            _gl.BufferData(BufferTargetARB.ArrayBuffer, (nuint)(vertexArray.Length * sizeof(float)), v, BufferUsageARB.StaticDraw);
        }

        // Position attribute (location = 0)
        _gl.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 7 * sizeof(float), (void*)0);
        _gl.EnableVertexAttribArray(0);

        // Color attribute (location = 1)
        _gl.VertexAttribPointer(1, 3, VertexAttribPointerType.Float, false, 7 * sizeof(float), (void*)(3 * sizeof(float)));
        _gl.EnableVertexAttribArray(1);

        // Size attribute (location = 2)
        _gl.VertexAttribPointer(2, 1, VertexAttribPointerType.Float, false, 7 * sizeof(float), (void*)(6 * sizeof(float)));
        _gl.EnableVertexAttribArray(2);

        _gl.BindVertexArray(0);
    }

    private void InitializeShader()
    {
        string vertexShader = @"
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aSize;

out vec3 StarColor;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    StarColor = aColor;
    gl_Position = projection * view * vec4(aPosition, 1.0);
    gl_PointSize = aSize;
}
";

        string fragmentShader = @"
#version 330 core
out vec4 FragColor;

in vec3 StarColor;

void main()
{
    // Create circular point (distance from center)
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    
    // Fade out at edges
    float alpha = 1.0 - smoothstep(0.3, 0.5, dist);
    
    // Add slight glow
    float glow = exp(-dist * 4.0);
    
    vec3 color = StarColor * (0.7 + glow * 0.3);
    FragColor = vec4(color, alpha);
}
";

        _shader = new Shader(_gl, vertexShader, fragmentShader);
    }

    public void Render(Camera camera, float aspectRatio)
    {
        if (_shader == null) return;

        // Disable depth writing for stars (they're always in background)
        _gl.DepthMask(false);
        _gl.Enable(EnableCap.Blend);
        _gl.BlendFunc(BlendingFactor.SrcAlpha, BlendingFactor.OneMinusSrcAlpha);
        _gl.Enable(EnableCap.ProgramPointSize); // Enable point size from shader

        _shader.Use();
        _gl.BindVertexArray(_vao);

        // Remove translation from view matrix (stars stay at infinite distance)
        var viewMatrix = camera.GetViewMatrix();
        viewMatrix.M41 = 0;
        viewMatrix.M42 = 0;
        viewMatrix.M43 = 0;

        _shader.SetMatrix4("view", viewMatrix);
        _shader.SetMatrix4("projection", camera.GetProjectionMatrix(aspectRatio));

        _gl.DrawArrays(PrimitiveType.Points, 0, (uint)_stars.Count);

        _gl.BindVertexArray(0);
        _gl.DepthMask(true);
        _gl.Disable(EnableCap.Blend);
    }

    public void Dispose()
    {
        if (!_disposed)
        {
            _gl.DeleteBuffer(_vbo);
            _gl.DeleteVertexArray(_vao);
            _shader?.Dispose();
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }

    private class Star
    {
        public Vector3 Position { get; set; }
        public Vector3 Color { get; set; }
        public float Size { get; set; }
    }
}
