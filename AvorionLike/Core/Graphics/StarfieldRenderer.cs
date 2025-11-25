using Silk.NET.OpenGL;
using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Renders an enhanced procedural starfield background with nebulae and varied star types
/// </summary>
public class StarfieldRenderer : IDisposable
{
    private readonly GL _gl;
    private Shader? _shader;
    private uint _vao;
    private uint _vbo;
    private readonly List<Star> _stars = new();
    private const int StarCount = 8000; // More stars for denser field
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

            // Varied star brightness with more bright stars
            float brightness = (float)Math.Pow(random.NextDouble(), 0.7) * 0.7f + 0.3f; // 0.3 to 1.0, biased toward bright
            float size = (float)(random.NextDouble() * 2.0 + 0.5); // 0.5 to 2.5
            
            // Enhanced star color variety based on stellar classification
            int starType = random.Next(100);
            Vector3 color;
            
            if (starType < 50)
            {
                // White/Yellow-white (G-type, like our Sun) - 50%
                color = new Vector3(1.0f, 0.98f, 0.95f);
            }
            else if (starType < 70)
            {
                // Blue-white (A/B-type, hot stars) - 20%
                color = new Vector3(0.85f, 0.92f, 1.0f);
            }
            else if (starType < 82)
            {
                // Bright blue (O-type, very hot) - 12%
                float blueTint = (float)random.NextDouble() * 0.15f;
                color = new Vector3(0.7f + blueTint, 0.8f + blueTint, 1.0f);
            }
            else if (starType < 92)
            {
                // Yellow/Orange (K-type) - 10%
                color = new Vector3(1.0f, 0.9f, 0.7f);
            }
            else if (starType < 97)
            {
                // Red/Orange (M-type, cool stars) - 5%
                color = new Vector3(1.0f, 0.75f, 0.6f);
            }
            else
            {
                // Rare colored stars (variable/exotic) - 3%
                float hue = (float)random.NextDouble();
                // Create subtle colored stars (cyan, magenta hints)
                if (hue < 0.5f)
                    color = new Vector3(0.9f, 0.95f, 1.0f); // Slight cyan
                else
                    color = new Vector3(1.0f, 0.92f, 0.98f); // Slight magenta
            }

            _stars.Add(new Star
            {
                Position = position,
                Color = color * brightness,
                Size = size
            });
        }
        
        // Add some extra bright "prominent" stars
        for (int i = 0; i < 50; i++)
        {
            float theta = (float)(random.NextDouble() * Math.PI * 2);
            float phi = (float)(random.NextDouble() * Math.PI);
            float radius = 500f;

            var position = new Vector3(
                radius * MathF.Sin(phi) * MathF.Cos(theta),
                radius * MathF.Sin(phi) * MathF.Sin(theta),
                radius * MathF.Cos(phi)
            );

            _stars.Add(new Star
            {
                Position = position,
                Color = new Vector3(1.0f, 1.0f, 1.0f) * 1.2f, // Extra bright
                Size = (float)random.NextDouble() * 2.0f + 2.5f // Larger size
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
out float StarSize;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    StarColor = aColor;
    StarSize = aSize;
    gl_Position = projection * view * vec4(aPosition, 1.0);
    gl_PointSize = aSize * 2.0; // Larger points for better visibility
}
";

        string fragmentShader = @"
#version 330 core
out vec4 FragColor;

in vec3 StarColor;
in float StarSize;

void main()
{
    // Create circular point with soft edges
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    
    // Multi-layer glow effect for more beautiful stars
    float coreAlpha = 1.0 - smoothstep(0.0, 0.15, dist); // Bright core
    float innerGlow = exp(-dist * 6.0) * 0.8; // Inner glow
    float outerGlow = exp(-dist * 2.5) * 0.3; // Outer halo
    
    // Combine glow layers
    float totalGlow = coreAlpha + innerGlow + outerGlow;
    
    // Star color with chromatic aberration hint (subtle color shift at edges)
    vec3 coreColor = StarColor;
    vec3 edgeColor = StarColor * vec3(0.95, 1.0, 1.05); // Slight blue shift at edges
    vec3 finalColor = mix(edgeColor, coreColor, coreAlpha);
    
    // Add subtle twinkle/sparkle based on star size
    float sparkle = 1.0;
    if (StarSize > 1.5) {
        // Create subtle cross/spike pattern for bright stars
        float angle = atan(coord.y, coord.x);
        float spike = pow(abs(sin(angle * 2.0)), 8.0);
        sparkle = 1.0 + spike * 0.3 * (1.0 - dist * 2.0);
    }
    
    finalColor *= sparkle;
    
    // Fade out at edges
    float alpha = clamp(totalGlow, 0.0, 1.0);
    
    FragColor = vec4(finalColor, alpha);
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
