using Silk.NET.OpenGL;
using System.Numerics;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Enhanced voxel renderer with PBR-like lighting, glow effects, and better visuals
/// </summary>
public class EnhancedVoxelRenderer : IDisposable
{
    private readonly GL _gl;
    private Shader? _shader;
    private uint _vao;
    private uint _vbo;
    private MaterialManager? _materialManager;
    private bool _disposed = false;

    // Multiple light sources for better lighting
    private readonly List<LightSource> _lights = new();

    // Cube vertices with normals and texture coordinates
    private readonly float[] _cubeVertices = GenerateCubeVertices();
    
    // Cache for optimized meshes per structure
    private readonly Dictionary<Guid, CachedMesh> _meshCache = new();
    
    /// <summary>
    /// Represents a cached mesh for a voxel structure
    /// </summary>
    private class CachedMesh
    {
        public uint VAO { get; set; }
        public uint VertexVBO { get; set; }
        public uint IndexEBO { get; set; }
        public int IndexCount { get; set; }
        public int BlockCount { get; set; }
    }

    public EnhancedVoxelRenderer(GL gl)
    {
        _gl = gl;
        _materialManager = new MaterialManager(gl);
        InitializeBuffers();
        InitializeShader();
        InitializeLights();
    }

    private void InitializeLights()
    {
        // Main sun light - Brighter for better visibility
        _lights.Add(new LightSource
        {
            Position = new Vector3(200, 300, 200),
            Color = new Vector3(1.0f, 0.98f, 0.95f), // Slightly warm white
            Intensity = 1.5f // Increased from 1.0
        });

        // Ambient fill light - Cooler blue tone
        _lights.Add(new LightSource
        {
            Position = new Vector3(-100, -50, 100),
            Color = new Vector3(0.5f, 0.6f, 0.9f),  // More blue
            Intensity = 0.4f // Increased from 0.3
        });

        // Rim light - Accent lighting
        _lights.Add(new LightSource
        {
            Position = new Vector3(0, 50, -200),
            Color = new Vector3(0.7f, 0.8f, 1.0f),  // Bright blue
            Intensity = 0.5f  // Increased from 0.4
        });
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

        // Position attribute (location = 0)
        _gl.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 8 * sizeof(float), (void*)0);
        _gl.EnableVertexAttribArray(0);

        // Normal attribute (location = 1)
        _gl.VertexAttribPointer(1, 3, VertexAttribPointerType.Float, false, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        _gl.EnableVertexAttribArray(1);

        // Texture coordinate attribute (location = 2)
        _gl.VertexAttribPointer(2, 2, VertexAttribPointerType.Float, false, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        _gl.EnableVertexAttribArray(2);

        _gl.BindVertexArray(0);
    }

    private void InitializeShader()
    {
        string vertexShader = @"
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec4 VertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(aPosition, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    VertexColor = aColor;
    gl_Position = projection * view * worldPos;
}
";

        string fragmentShader = @"
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 VertexColor;

// Material properties
uniform vec3 baseColor;
uniform vec3 emissiveColor;
uniform float metallic;
uniform float roughness;
uniform float emissiveStrength;

// Lighting
uniform vec3 lightPos[3];
uniform vec3 lightColor[3];
uniform float lightIntensity[3];
uniform vec3 viewPos;

// Constants
const float PI = 3.14159265359;
const vec3 ambientLight = vec3(0.25, 0.25, 0.28); // Increased ambient light for better material visibility

// Simplified PBR functions
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // Use vertex color as the base color
    vec3 blockColor = VertexColor.rgb;

    // Fresnel for metals vs dielectrics
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, blockColor, metallic);

    vec3 Lo = vec3(0.0);

    // Calculate reflectance for each light
    for(int i = 0; i < 3; i++)
    {
        vec3 L = normalize(lightPos[i] - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPos[i] - FragPos);
        float attenuation = 1.0 / (distance * distance * 0.0001 + 1.0);
        vec3 radiance = lightColor[i] * lightIntensity[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = numerator / max(denominator, 0.001);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * blockColor / PI + specular) * radiance * NdotL;
    }

    // Add ambient lighting
    vec3 ambient = ambientLight * blockColor;
    
    // Add emissive
    vec3 emissive = emissiveColor * emissiveStrength;

    vec3 color = ambient + Lo + emissive;

    // HDR tone mapping (Reinhard)
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
";

        _shader = new Shader(_gl, vertexShader, fragmentShader);
    }

    public unsafe void RenderVoxelStructure(VoxelStructureComponent structure, Camera camera, Vector3 entityPosition, float aspectRatio)
    {
        if (_shader == null || _materialManager == null) return;

        _shader.Use();

        // Set view and projection matrices
        _shader.SetMatrix4("view", camera.GetViewMatrix());
        _shader.SetMatrix4("projection", camera.GetProjectionMatrix(aspectRatio));
        _shader.SetVector3("viewPos", camera.Position);

        // Set light properties
        for (int i = 0; i < _lights.Count && i < 3; i++)
        {
            _shader.SetVector3($"lightPos[{i}]", _lights[i].Position);
            _shader.SetVector3($"lightColor[{i}]", _lights[i].Color);
            _shader.SetFloat($"lightIntensity[{i}]", _lights[i].Intensity);
        }

        // Get or create cached mesh for this structure
        CachedMesh? cachedMesh = GetOrCreateMesh(structure);
        if (cachedMesh == null)
            return;

        // Bind the cached mesh VAO
        _gl.BindVertexArray(cachedMesh.VAO);

        // Create model matrix for the entire structure (positioned at entity position)
        var model = Matrix4x4.CreateTranslation(entityPosition);
        _shader.SetMatrix4("model", model);

        // Use average material properties for the whole structure
        // In a more advanced version, we could render by material type in batches
        var material = _materialManager.GetMaterial("Iron"); // Default material
        _shader.SetVector3("baseColor", material.BaseColor);
        _shader.SetVector3("emissiveColor", material.EmissiveColor);
        _shader.SetFloat("metallic", material.Metallic);
        _shader.SetFloat("roughness", material.Roughness);
        _shader.SetFloat("emissiveStrength", material.EmissiveStrength);

        // Draw the optimized mesh
        _gl.DrawElements(PrimitiveType.Triangles, (uint)cachedMesh.IndexCount, DrawElementsType.UnsignedInt, (void*)0);

        _gl.BindVertexArray(0);
    }
    
    /// <summary>
    /// Get or create an optimized mesh for a voxel structure
    /// </summary>
    private unsafe CachedMesh? GetOrCreateMesh(VoxelStructureComponent structure)
    {
        // Check if we have a cached mesh and if the block count matches
        if (_meshCache.TryGetValue(structure.EntityId, out var cached) && 
            cached.BlockCount == structure.Blocks.Count)
        {
            return cached;
        }

        // If cached mesh exists but is outdated, delete it
        if (cached != null)
        {
            DeleteMesh(cached);
            _meshCache.Remove(structure.EntityId);
        }

        // Build optimized mesh using face culling
        var optimizedMesh = GreedyMeshBuilder.BuildMesh(structure.Blocks);
        
        if (optimizedMesh.VertexCount == 0)
            return null;

        // Create VAO and VBOs
        uint vao = _gl.GenVertexArray();
        _gl.BindVertexArray(vao);

        // Create vertex buffer (interleaved: position, normal, color)
        uint vbo = _gl.GenBuffer();
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, vbo);
        
        // Prepare interleaved vertex data
        int vertexCount = optimizedMesh.VertexCount;
        float[] vertexData = new float[vertexCount * 10]; // 3 pos + 3 normal + 4 color (RGBA)
        
        for (int i = 0; i < vertexCount; i++)
        {
            int offset = i * 10;
            
            // Position
            vertexData[offset + 0] = optimizedMesh.Vertices[i].X;
            vertexData[offset + 1] = optimizedMesh.Vertices[i].Y;
            vertexData[offset + 2] = optimizedMesh.Vertices[i].Z;
            
            // Normal
            vertexData[offset + 3] = optimizedMesh.Normals[i].X;
            vertexData[offset + 4] = optimizedMesh.Normals[i].Y;
            vertexData[offset + 5] = optimizedMesh.Normals[i].Z;
            
            // Color (convert from uint RGB to RGBA floats)
            uint color = optimizedMesh.Colors[i];
            vertexData[offset + 6] = ((color >> 16) & 0xFF) / 255.0f; // R
            vertexData[offset + 7] = ((color >> 8) & 0xFF) / 255.0f;  // G
            vertexData[offset + 8] = (color & 0xFF) / 255.0f;         // B
            vertexData[offset + 9] = 1.0f;                             // A
        }
        
        fixed (float* v = &vertexData[0])
        {
            _gl.BufferData(BufferTargetARB.ArrayBuffer, (nuint)(vertexData.Length * sizeof(float)), v, BufferUsageARB.StaticDraw);
        }

        // Position attribute (location = 0)
        _gl.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 10 * sizeof(float), (void*)0);
        _gl.EnableVertexAttribArray(0);

        // Normal attribute (location = 1)
        _gl.VertexAttribPointer(1, 3, VertexAttribPointerType.Float, false, 10 * sizeof(float), (void*)(3 * sizeof(float)));
        _gl.EnableVertexAttribArray(1);

        // Color attribute (location = 2) - reusing texture coordinate attribute
        _gl.VertexAttribPointer(2, 4, VertexAttribPointerType.Float, false, 10 * sizeof(float), (void*)(6 * sizeof(float)));
        _gl.EnableVertexAttribArray(2);

        // Create index buffer
        uint ebo = _gl.GenBuffer();
        _gl.BindBuffer(BufferTargetARB.ElementArrayBuffer, ebo);
        
        int[] indices = optimizedMesh.Indices.ToArray();
        fixed (int* idx = &indices[0])
        {
            _gl.BufferData(BufferTargetARB.ElementArrayBuffer, (nuint)(indices.Length * sizeof(int)), idx, BufferUsageARB.StaticDraw);
        }

        _gl.BindVertexArray(0);

        // Cache the mesh
        var newCached = new CachedMesh
        {
            VAO = vao,
            VertexVBO = vbo,
            IndexEBO = ebo,
            IndexCount = optimizedMesh.IndexCount,
            BlockCount = structure.Blocks.Count
        };
        
        _meshCache[structure.EntityId] = newCached;
        
        return newCached;
    }
    
    /// <summary>
    /// Delete a cached mesh and free GPU resources
    /// </summary>
    private void DeleteMesh(CachedMesh mesh)
    {
        _gl.DeleteBuffer(mesh.VertexVBO);
        _gl.DeleteBuffer(mesh.IndexEBO);
        _gl.DeleteVertexArray(mesh.VAO);
    }

    private static float[] GenerateCubeVertices()
    {
        // Each vertex: position(3) + normal(3) + texcoord(2) = 8 floats
        return new float[]
        {
            // Back face
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
            
            // Front face
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
            
            // Left face
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            
            // Right face
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            
            // Bottom face
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
            
            // Top face
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
        };
    }

    public void Dispose()
    {
        if (!_disposed)
        {
            // Clean up cached meshes
            foreach (var cached in _meshCache.Values)
            {
                DeleteMesh(cached);
            }
            _meshCache.Clear();
            
            _gl.DeleteBuffer(_vbo);
            _gl.DeleteVertexArray(_vao);
            _shader?.Dispose();
            _materialManager?.Dispose();
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}

/// <summary>
/// Light source for rendering
/// </summary>
public class LightSource
{
    public Vector3 Position { get; set; }
    public Vector3 Color { get; set; }
    public float Intensity { get; set; } = 1.0f;
}
