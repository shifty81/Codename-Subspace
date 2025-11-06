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
        // Main sun light
        _lights.Add(new LightSource
        {
            Position = new Vector3(200, 300, 200),
            Color = new Vector3(1.0f, 0.95f, 0.9f),
            Intensity = 1.0f
        });

        // Ambient fill light
        _lights.Add(new LightSource
        {
            Position = new Vector3(-100, -50, 100),
            Color = new Vector3(0.4f, 0.5f, 0.8f),
            Intensity = 0.3f
        });

        // Rim light
        _lights.Add(new LightSource
        {
            Position = new Vector3(0, 50, -200),
            Color = new Vector3(0.6f, 0.7f, 1.0f),
            Intensity = 0.4f
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
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(aPosition, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * worldPos;
}
";

        string fragmentShader = @"
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

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
const vec3 ambientLight = vec3(0.15, 0.15, 0.2); // Slight blue ambient for space

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

    // Fresnel for metals vs dielectrics
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, metallic);

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
        Lo += (kD * baseColor / PI + specular) * radiance * NdotL;
    }

    // Add ambient lighting
    vec3 ambient = ambientLight * baseColor;
    
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

    public void RenderVoxelStructure(VoxelStructureComponent structure, Camera camera, Vector3 entityPosition, float aspectRatio)
    {
        if (_shader == null || _materialManager == null) return;

        _shader.Use();
        _gl.BindVertexArray(_vao);

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

        // Render each voxel block
        foreach (var block in structure.Blocks)
        {
            // Get material for this block
            var material = _materialManager.GetMaterial(block.MaterialType);

            // Create model matrix for this block
            var model = Matrix4x4.CreateScale(block.Size) * 
                       Matrix4x4.CreateTranslation(entityPosition + block.Position);
            
            _shader.SetMatrix4("model", model);
            
            // Set material properties
            _shader.SetVector3("baseColor", material.BaseColor);
            _shader.SetVector3("emissiveColor", material.EmissiveColor);
            _shader.SetFloat("metallic", material.Metallic);
            _shader.SetFloat("roughness", material.Roughness);
            _shader.SetFloat("emissiveStrength", material.EmissiveStrength);

            _gl.DrawArrays(PrimitiveType.Triangles, 0, 36);
        }

        _gl.BindVertexArray(0);
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
