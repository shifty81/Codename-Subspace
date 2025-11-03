using Silk.NET.OpenGL;
using System.Numerics;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Renders voxel blocks as colored cubes in 3D space
/// </summary>
public class VoxelRenderer : IDisposable
{
    private readonly GL _gl;
    private Shader? _shader;
    private uint _vao;
    private uint _vbo;
    private bool _disposed = false;

    // Lighting configuration
    private static readonly Vector3 DefaultLightPosition = new Vector3(100, 200, 100);
    private static readonly Vector3 DefaultLightColor = new Vector3(1.0f, 1.0f, 1.0f);

    // Cube vertices (position + normal)
    private readonly float[] _cubeVertices = new[]
    {
        // Positions          // Normals
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    public VoxelRenderer(GL gl)
    {
        _gl = gl;
        InitializeBuffers();
        InitializeShader();
    }

    private unsafe void InitializeBuffers()
    {
        _vao = _gl.GenVertexArray();
        _gl.BindVertexArray(_vao);

        _vbo = _gl.GenBuffer();
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, _vbo);
        
        fixed (float* v = &_cubeVertices[0])
        {
            _gl.BufferData(BufferTargetARB.ArrayBuffer, (nuint)(_cubeVertices.Length * sizeof(float)), v, BufferUsageARB.StaticDraw);
        }

        // Position attribute
        _gl.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 6 * sizeof(float), (void*)0);
        _gl.EnableVertexAttribArray(0);

        // Normal attribute
        _gl.VertexAttribPointer(1, 3, VertexAttribPointerType.Float, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        _gl.EnableVertexAttribArray(1);

        _gl.BindVertexArray(0);
    }

    private void InitializeShader()
    {
        string vertexShader = @"
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
";

        string fragmentShader = @"
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
";

        _shader = new Shader(_gl, vertexShader, fragmentShader);
    }

    public void RenderVoxelStructure(VoxelStructureComponent structure, Camera camera, Vector3 entityPosition, float aspectRatio)
    {
        if (_shader == null) return;

        _shader.Use();
        _gl.BindVertexArray(_vao);

        // Set lighting
        _shader.SetVector3("lightPos", DefaultLightPosition);
        _shader.SetVector3("lightColor", DefaultLightColor);
        _shader.SetVector3("viewPos", camera.Position);

        // Set view and projection matrices
        _shader.SetMatrix4("view", camera.GetViewMatrix());
        _shader.SetMatrix4("projection", camera.GetProjectionMatrix(aspectRatio));

        // Render each voxel block
        foreach (var block in structure.Blocks)
        {
            // Create model matrix for this block
            var model = Matrix4x4.CreateScale(block.Size) * 
                       Matrix4x4.CreateTranslation(entityPosition + block.Position);
            
            _shader.SetMatrix4("model", model);
            _shader.SetVector3("objectColor", GetMaterialColor(block.MaterialType));

            _gl.DrawArrays(PrimitiveType.Triangles, 0, 36);
        }

        _gl.BindVertexArray(0);
    }

    private Vector3 GetMaterialColor(string materialType)
    {
        return materialType.ToLower() switch
        {
            "iron" => new Vector3(0.7f, 0.7f, 0.7f),      // Gray
            "titanium" => new Vector3(0.8f, 0.9f, 1.0f),   // Light blue
            "naonite" => new Vector3(0.2f, 0.8f, 0.3f),    // Green
            "trinium" => new Vector3(0.3f, 0.6f, 0.9f),    // Blue
            "xanion" => new Vector3(0.9f, 0.7f, 0.2f),     // Gold
            "ogonite" => new Vector3(0.9f, 0.3f, 0.3f),    // Red
            "avorion" => new Vector3(0.8f, 0.2f, 0.9f),    // Purple
            _ => new Vector3(0.5f, 0.5f, 0.5f)             // Default gray
        };
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
}
