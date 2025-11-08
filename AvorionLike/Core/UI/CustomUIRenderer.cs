using Silk.NET.OpenGL;
using System.Numerics;
using AvorionLike.Core.Graphics;

namespace AvorionLike.Core.UI;

/// <summary>
/// Custom OpenGL-based UI renderer for game interface
/// Separate from ImGui which is used only for debug/console
/// </summary>
public class CustomUIRenderer : IDisposable
{
    private readonly GL _gl;
    private AvorionLike.Core.Graphics.Shader? _uiShader;
    private uint _vao;
    private uint _vbo;
    private uint _ebo;
    private bool _disposed = false;
    
    // UI colors
    private readonly Vector4 _primaryColor = new(0.0f, 0.8f, 1.0f, 1.0f);      // Cyan
    private readonly Vector4 _accentColor = new(0.2f, 1.0f, 0.8f, 1.0f);       // Bright teal
    private readonly Vector4 _warningColor = new(1.0f, 0.5f, 0.0f, 1.0f);      // Orange
    private readonly Vector4 _dangerColor = new(1.0f, 0.2f, 0.2f, 1.0f);       // Red
    private readonly Vector4 _hudColor = new(0.0f, 0.8f, 1.0f, 0.8f);          // HUD elements

    private float _screenWidth;
    private float _screenHeight;
    private float _animationTime = 0f;
    
    public CustomUIRenderer(GL gl, float screenWidth, float screenHeight)
    {
        _gl = gl;
        _screenWidth = screenWidth;
        _screenHeight = screenHeight;
        InitializeBuffers();
        InitializeShader();
    }
    
    public void UpdateScreenSize(float width, float height)
    {
        _screenWidth = width;
        _screenHeight = height;
    }
    
    public void Update(float deltaTime)
    {
        _animationTime += deltaTime;
    }
    
    private unsafe void InitializeBuffers()
    {
        _vao = _gl.GenVertexArray();
        _gl.BindVertexArray(_vao);
        
        _vbo = _gl.GenBuffer();
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);
        
        // Allocate buffer for dynamic vertex data (will update each frame)
        _gl.BufferData(BufferTargetARB.ArrayBuffer, 10000 * sizeof(float), null, BufferUsageARB.DynamicDraw);
        
        _ebo = _gl.GenBuffer();
        _gl.BindBuffer(BufferTargetARB.ElementArrayBuffer, _ebo);
        _gl.BufferData(BufferTargetARB.ElementArrayBuffer, 10000 * sizeof(uint), null, BufferUsageARB.DynamicDraw);
        
        // Position attribute (x, y)
        _gl.VertexAttribPointer(0, 2, VertexAttribPointerType.Float, false, 6 * sizeof(float), (void*)0);
        _gl.EnableVertexAttribArray(0);
        
        // Color attribute (r, g, b, a)
        _gl.VertexAttribPointer(1, 4, VertexAttribPointerType.Float, false, 6 * sizeof(float), (void*)(2 * sizeof(float)));
        _gl.EnableVertexAttribArray(1);
        
        _gl.BindVertexArray(0);
    }
    
    private void InitializeShader()
    {
        string vertexShader = @"
#version 330 core
layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec4 aColor;

out vec4 vertexColor;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(aPosition, 0.0, 1.0);
    vertexColor = aColor;
}
";

        string fragmentShader = @"
#version 330 core
in vec4 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vertexColor;
}
";

        _uiShader = new AvorionLike.Core.Graphics.Shader(_gl, vertexShader, fragmentShader);
    }
    
    public void BeginRender()
    {
        if (_uiShader == null) return;
        
        // Enable blending for transparent UI elements
        _gl.Enable(EnableCap.Blend);
        _gl.BlendFunc(BlendingFactor.SrcAlpha, BlendingFactor.OneMinusSrcAlpha);
        
        // Disable depth test for UI rendering
        _gl.Disable(EnableCap.DepthTest);
        
        _uiShader.Use();
        
        // Set up orthographic projection for 2D UI
        var projection = Matrix4x4.CreateOrthographicOffCenter(
            0f, _screenWidth,
            _screenHeight, 0f,
            -1f, 1f
        );
        
        _uiShader.SetMatrix4("projection", projection);
        
        _gl.BindVertexArray(_vao);
    }
    
    public void EndRender()
    {
        _gl.BindVertexArray(0);
        _gl.Enable(EnableCap.DepthTest);
        _gl.Disable(EnableCap.Blend);
    }
    
    /// <summary>
    /// Draw a line from point A to point B
    /// </summary>
    public void DrawLine(Vector2 start, Vector2 end, Vector4 color, float thickness = 2.0f)
    {
        // Calculate perpendicular vector for line thickness
        Vector2 direction = Vector2.Normalize(end - start);
        Vector2 perpendicular = new Vector2(-direction.Y, direction.X) * (thickness * 0.5f);
        
        // Create quad for the line
        float[] vertices = new float[]
        {
            // Position (x, y), Color (r, g, b, a)
            start.X - perpendicular.X, start.Y - perpendicular.Y, color.X, color.Y, color.Z, color.W,
            start.X + perpendicular.X, start.Y + perpendicular.Y, color.X, color.Y, color.Z, color.W,
            end.X + perpendicular.X, end.Y + perpendicular.Y, color.X, color.Y, color.Z, color.W,
            end.X - perpendicular.X, end.Y - perpendicular.Y, color.X, color.Y, color.Z, color.W,
        };
        
        uint[] indices = new uint[] { 0, 1, 2, 2, 3, 0 };
        
        DrawPrimitive(vertices, indices);
    }
    
    /// <summary>
    /// Draw a rectangle outline
    /// </summary>
    public void DrawRect(Vector2 topLeft, Vector2 size, Vector4 color, float thickness = 2.0f)
    {
        Vector2 topRight = topLeft + new Vector2(size.X, 0);
        Vector2 bottomRight = topLeft + size;
        Vector2 bottomLeft = topLeft + new Vector2(0, size.Y);
        
        DrawLine(topLeft, topRight, color, thickness);
        DrawLine(topRight, bottomRight, color, thickness);
        DrawLine(bottomRight, bottomLeft, color, thickness);
        DrawLine(bottomLeft, topLeft, color, thickness);
    }
    
    /// <summary>
    /// Draw a filled rectangle
    /// </summary>
    public void DrawRectFilled(Vector2 topLeft, Vector2 size, Vector4 color)
    {
        float[] vertices = new float[]
        {
            topLeft.X, topLeft.Y, color.X, color.Y, color.Z, color.W,
            topLeft.X + size.X, topLeft.Y, color.X, color.Y, color.Z, color.W,
            topLeft.X + size.X, topLeft.Y + size.Y, color.X, color.Y, color.Z, color.W,
            topLeft.X, topLeft.Y + size.Y, color.X, color.Y, color.Z, color.W,
        };
        
        uint[] indices = new uint[] { 0, 1, 2, 2, 3, 0 };
        
        DrawPrimitive(vertices, indices);
    }
    
    /// <summary>
    /// Draw a filled rectangle with vertical gradient
    /// </summary>
    public void DrawRectGradient(Vector2 topLeft, Vector2 size, Vector4 topColor, Vector4 bottomColor)
    {
        float[] vertices = new float[]
        {
            topLeft.X, topLeft.Y, topColor.X, topColor.Y, topColor.Z, topColor.W,
            topLeft.X + size.X, topLeft.Y, topColor.X, topColor.Y, topColor.Z, topColor.W,
            topLeft.X + size.X, topLeft.Y + size.Y, bottomColor.X, bottomColor.Y, bottomColor.Z, bottomColor.W,
            topLeft.X, topLeft.Y + size.Y, bottomColor.X, bottomColor.Y, bottomColor.Z, bottomColor.W,
        };
        
        uint[] indices = new uint[] { 0, 1, 2, 2, 3, 0 };
        
        DrawPrimitive(vertices, indices);
    }
    
    /// <summary>
    /// Draw a filled rectangle with glow effect around edges
    /// </summary>
    public void DrawRectWithGlow(Vector2 topLeft, Vector2 size, Vector4 fillColor, Vector4 glowColor, float glowSize = 8f)
    {
        // Draw glow layers from outside to inside
        for (int i = 0; i < 4; i++)
        {
            float offset = glowSize * (1f - i / 4f);
            float alpha = glowColor.W * (i / 4f) * 0.3f;
            Vector4 layerColor = new Vector4(glowColor.X, glowColor.Y, glowColor.Z, alpha);
            
            DrawRectFilled(
                new Vector2(topLeft.X - offset, topLeft.Y - offset),
                new Vector2(size.X + offset * 2, size.Y + offset * 2),
                layerColor
            );
        }
        
        // Draw main fill
        DrawRectFilled(topLeft, size, fillColor);
    }
    
    /// <summary>
    /// Draw a progress bar with enhanced visuals
    /// </summary>
    public void DrawProgressBar(Vector2 position, Vector2 size, float progress, Vector4 fillColor, Vector4 bgColor, Vector4 borderColor)
    {
        progress = Math.Clamp(progress, 0f, 1f);
        
        // Background with slight gradient
        Vector4 bgDark = new Vector4(bgColor.X * 0.7f, bgColor.Y * 0.7f, bgColor.Z * 0.7f, bgColor.W);
        DrawRectGradient(position, size, bgColor, bgDark);
        
        // Fill bar with glow effect
        if (progress > 0.01f)
        {
            Vector2 fillSize = new Vector2(size.X * progress, size.Y);
            Vector4 glowColor = new Vector4(fillColor.X, fillColor.Y, fillColor.Z, 0.5f);
            
            // Add glow effect on the fill
            float glowWidth = 4f;
            for (int i = 0; i < 3; i++)
            {
                float offset = glowWidth * (1f - i / 3f);
                float alpha = fillColor.W * (i / 3f) * 0.2f;
                Vector4 layerColor = new Vector4(fillColor.X * 1.2f, fillColor.Y * 1.2f, fillColor.Z * 1.2f, alpha);
                
                DrawRectFilled(
                    new Vector2(position.X, position.Y - offset),
                    new Vector2(fillSize.X, fillSize.Y + offset * 2),
                    layerColor
                );
            }
            
            // Brighten the fill with gradient
            Vector4 brightFill = new Vector4(
                Math.Min(1f, fillColor.X * 1.1f),
                Math.Min(1f, fillColor.Y * 1.1f),
                Math.Min(1f, fillColor.Z * 1.1f),
                fillColor.W
            );
            DrawRectGradient(position, fillSize, brightFill, fillColor);
        }
        
        // Border with enhanced thickness
        DrawRect(position, size, borderColor, 2f);
        
        // Inner highlight on top edge
        Vector4 highlightColor = new Vector4(borderColor.X * 1.5f, borderColor.Y * 1.5f, borderColor.Z * 1.5f, borderColor.W * 0.5f);
        DrawLine(position + new Vector2(1, 1), position + new Vector2(size.X - 1, 1), highlightColor, 1f);
    }
    
    /// <summary>
    /// Draw a circle outline
    /// </summary>
    public void DrawCircle(Vector2 center, float radius, Vector4 color, int segments = 32, float thickness = 2.0f)
    {
        float angleStep = (float)(2 * Math.PI / segments);
        
        for (int i = 0; i < segments; i++)
        {
            float angle1 = i * angleStep;
            float angle2 = (i + 1) * angleStep;
            
            Vector2 p1 = center + new Vector2(MathF.Cos(angle1), MathF.Sin(angle1)) * radius;
            Vector2 p2 = center + new Vector2(MathF.Cos(angle2), MathF.Sin(angle2)) * radius;
            
            DrawLine(p1, p2, color, thickness);
        }
    }
    
    /// <summary>
    /// Draw a filled circle
    /// </summary>
    public void DrawCircleFilled(Vector2 center, float radius, Vector4 color, int segments = 32)
    {
        List<float> vertices = new List<float>();
        List<uint> indices = new List<uint>();
        
        // Center vertex
        vertices.AddRange(new[] { center.X, center.Y, color.X, color.Y, color.Z, color.W });
        
        // Outer vertices
        float angleStep = (float)(2 * Math.PI / segments);
        for (int i = 0; i <= segments; i++)
        {
            float angle = i * angleStep;
            float x = center.X + MathF.Cos(angle) * radius;
            float y = center.Y + MathF.Sin(angle) * radius;
            vertices.AddRange(new[] { x, y, color.X, color.Y, color.Z, color.W });
        }
        
        // Create triangle fan indices
        for (uint i = 1; i <= segments; i++)
        {
            indices.Add(0);
            indices.Add(i);
            indices.Add(i + 1);
        }
        
        DrawPrimitive(vertices.ToArray(), indices.ToArray());
    }
    
    /// <summary>
    /// Draw a crosshair at the center of the screen
    /// </summary>
    public void DrawCrosshair()
    {
        Vector2 center = new Vector2(_screenWidth * 0.5f, _screenHeight * 0.5f);
        float dotRadius = 2.0f;
        
        // Draw center dot
        DrawCircleFilled(center, dotRadius, _hudColor);
        
        // Optional: Draw subtle outline
        Vector4 outlineColor = new Vector4(0.0f, 0.0f, 0.0f, 0.5f);
        DrawCircle(center, dotRadius + 1, outlineColor, 12, 1.5f);
    }
    
    /// <summary>
    /// Draw corner frames for HUD aesthetic
    /// </summary>
    public void DrawCornerFrames()
    {
        float cornerSize = 80f;
        float thickness = 3f;
        float offset = 15f;
        
        // Animate brightness
        float pulse = 0.7f + 0.3f * MathF.Sin(_animationTime * 2f);
        Vector4 frameColor = new Vector4(_hudColor.X, _hudColor.Y, _hudColor.Z, _hudColor.W * pulse);
        
        // Top-left
        DrawLine(new Vector2(offset, offset), new Vector2(cornerSize, offset), frameColor, thickness);
        DrawLine(new Vector2(offset, offset), new Vector2(offset, cornerSize), frameColor, thickness);
        
        // Top-right
        DrawLine(new Vector2(_screenWidth - offset, offset), new Vector2(_screenWidth - cornerSize, offset), frameColor, thickness);
        DrawLine(new Vector2(_screenWidth - offset, offset), new Vector2(_screenWidth - offset, cornerSize), frameColor, thickness);
        
        // Bottom-left
        DrawLine(new Vector2(offset, _screenHeight - offset), new Vector2(cornerSize, _screenHeight - offset), frameColor, thickness);
        DrawLine(new Vector2(offset, _screenHeight - offset), new Vector2(offset, _screenHeight - cornerSize), frameColor, thickness);
        
        // Bottom-right
        DrawLine(new Vector2(_screenWidth - offset, _screenHeight - offset), new Vector2(_screenWidth - cornerSize, _screenHeight - offset), frameColor, thickness);
        DrawLine(new Vector2(_screenWidth - offset, _screenHeight - offset), new Vector2(_screenWidth - offset, _screenHeight - cornerSize), frameColor, thickness);
        
        // Diagonal accents
        float accentSize = 15f;
        DrawLine(new Vector2(cornerSize, offset), new Vector2(cornerSize - accentSize, offset + accentSize), frameColor, thickness);
        DrawLine(new Vector2(_screenWidth - cornerSize, offset), new Vector2(_screenWidth - cornerSize + accentSize, offset + accentSize), frameColor, thickness);
    }
    
    private unsafe void DrawPrimitive(float[] vertices, uint[] indices)
    {
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);
        fixed (float* v = &vertices[0])
        {
            _gl.BufferSubData(BufferTargetARB.ArrayBuffer, 0, (nuint)(vertices.Length * sizeof(float)), v);
        }
        
        _gl.BindBuffer(BufferTargetARB.ElementArrayBuffer, _ebo);
        fixed (uint* i = &indices[0])
        {
            _gl.BufferSubData(BufferTargetARB.ElementArrayBuffer, 0, (nuint)(indices.Length * sizeof(uint)), i);
        }
        
        _gl.DrawElements(PrimitiveType.Triangles, (uint)indices.Length, DrawElementsType.UnsignedInt, null);
    }
    
    public void Dispose()
    {
        if (!_disposed)
        {
            _gl.DeleteBuffer(_vbo);
            _gl.DeleteBuffer(_ebo);
            _gl.DeleteVertexArray(_vao);
            _uiShader?.Dispose();
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}
