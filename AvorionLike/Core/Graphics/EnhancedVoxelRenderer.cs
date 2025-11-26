using Silk.NET.OpenGL;
using System.Numerics;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Enhanced voxel renderer with PBR-like lighting, glow effects, and better visuals
/// Supports NPR (Non-Photorealistic Rendering) and Hybrid rendering modes
/// Addresses visual issues on blocks through improved edge detection, ambient occlusion, and material handling
/// </summary>
public class EnhancedVoxelRenderer : IDisposable
{
    private readonly GL _gl;
    private Shader? _shader;
    private uint _vao;
    private uint _vbo;
    private MaterialManager? _materialManager;
    private bool _disposed = false;
    
    // Rendering configuration for NPR/PBR/Hybrid modes
    private RenderingConfiguration _config = RenderingConfiguration.Instance;

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
        // Main sun light - Bright warm white for primary illumination
        _lights.Add(new LightSource
        {
            Position = new Vector3(250, 350, 250),
            Color = new Vector3(1.0f, 0.97f, 0.92f), // Warm white
            Intensity = 2.0f // Brighter main light for better visibility
        });

        // Ambient fill light - Cool blue for space atmosphere and shadow fill
        _lights.Add(new LightSource
        {
            Position = new Vector3(-150, -80, 120),
            Color = new Vector3(0.4f, 0.55f, 0.9f),  // Cool blue
            Intensity = 0.6f // Increased for better shadow fill
        });

        // Rim/Back light - Creates dramatic edge highlights
        _lights.Add(new LightSource
        {
            Position = new Vector3(0, 80, -250),
            Color = new Vector3(0.75f, 0.85f, 1.0f),  // Bright blue-white
            Intensity = 0.8f  // Strong rim light for shiny edge effect
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
out vec3 LocalPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(aPosition, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    VertexColor = aColor;
    LocalPos = aPosition; // Pass local position for edge detection and AO
    gl_Position = projection * view * worldPos;
}
";

        string fragmentShader = @"
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 VertexColor;
in vec3 LocalPos;

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

// NPR/Rendering configuration uniforms
uniform int renderingMode;           // 0 = PBR, 1 = NPR, 2 = Hybrid
uniform bool enableEdgeDetection;    // Edge detection for block outlines
uniform float edgeThickness;         // Edge line thickness
uniform vec3 edgeColor;              // Edge line color
uniform bool enableCelShading;       // Cel-shading mode
uniform int celShadingBands;         // Number of cel-shading bands
uniform bool enableAmbientOcclusion; // Block edge ambient occlusion
uniform float aoStrength;            // AO intensity
uniform bool enableProceduralDetails;// Procedural surface patterns
uniform float proceduralStrength;    // Procedural detail intensity
uniform bool enableBlockGlow;        // Glow on functional blocks
uniform float blockGlowIntensity;    // Glow intensity
uniform bool enableRimLighting;      // Rim lighting effect
uniform float rimStrength;           // Rim light intensity
uniform bool enableEnvReflections;   // Environment reflections

// Constants
const float PI = 3.14159265359;
const vec3 ambientLight = vec3(0.18, 0.20, 0.25); // Cool-toned ambient for space

// Enhanced PBR GGX Distribution
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
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

// Enhanced Fresnel with roughness consideration for more realistic reflections
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Fresnel with roughness for IBL approximation
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Simple noise function for procedural details
float hash(vec3 p)
{
    p = fract(p * vec3(443.897, 441.423, 437.195));
    p += dot(p, p.yzx + 19.19);
    return fract((p.x + p.y) * p.z);
}

// Simulate environment reflection color based on view direction
vec3 getEnvironmentReflection(vec3 R, float roughness)
{
    if (!enableEnvReflections) return vec3(0.0);
    
    // Simulate space environment with stars and nebula hints
    float height = R.y * 0.5 + 0.5;
    
    // Base space color gradient (dark blue to black)
    vec3 spaceColor = mix(vec3(0.01, 0.02, 0.05), vec3(0.0, 0.0, 0.02), height);
    
    // Add subtle nebula coloring
    float nebula = sin(R.x * 3.0 + R.z * 2.0) * 0.5 + 0.5;
    nebula *= sin(R.y * 2.0 + R.x) * 0.5 + 0.5;
    vec3 nebulaColor = mix(vec3(0.1, 0.05, 0.15), vec3(0.05, 0.1, 0.2), nebula) * 0.3;
    
    // Star-like highlights based on reflection direction
    float stars = pow(max(0.0, hash(floor(R * 50.0)) - 0.97), 2.0) * 30.0;
    vec3 starColor = vec3(1.0, 0.98, 0.95) * stars;
    
    // Blend with roughness (rougher surfaces see blurrier reflections)
    float clarity = 1.0 - roughness * 0.8;
    return (spaceColor + nebulaColor + starColor * clarity) * clarity;
}

// === NPR TECHNIQUES ===

// Edge detection using local position to find block boundaries
float detectBlockEdge(vec3 localPos, float thickness)
{
    // Detect edges at block boundaries using fract of position
    vec3 blockPos = fract(localPos);
    
    // Distance to nearest edge in each dimension
    vec3 edgeDist = min(blockPos, 1.0 - blockPos);
    float minEdgeDist = min(min(edgeDist.x, edgeDist.y), edgeDist.z);
    
    // Smooth edge detection
    float edge = 1.0 - smoothstep(0.0, 0.08 * thickness, minEdgeDist);
    return edge;
}

// Advanced ambient occlusion between block edges
float calculateBlockAO(vec3 localPos, vec3 normal)
{
    if (!enableAmbientOcclusion) return 1.0;
    
    // Calculate AO based on proximity to edges (corners are darker)
    vec3 blockPos = fract(localPos);
    vec3 edgeDist = min(blockPos, 1.0 - blockPos);
    
    // Corners have lower AO (are darker)
    float cornerDist = length(edgeDist);
    float ao = smoothstep(0.0, 0.25, cornerDist);
    
    // Edge darkening
    float minEdge = min(min(edgeDist.x, edgeDist.y), edgeDist.z);
    float edgeAO = smoothstep(0.0, 0.15, minEdge);
    
    // Combine and apply strength
    float finalAO = mix(1.0, ao * edgeAO, aoStrength);
    return finalAO;
}

// Cel-shading: Quantize lighting to discrete bands
float celShade(float lightValue, int bands)
{
    if (!enableCelShading) return lightValue;
    
    // Quantize to discrete bands
    float bandSize = 1.0 / float(bands);
    float quantized = floor(lightValue / bandSize) * bandSize + bandSize * 0.5;
    return quantized;
}

// Add procedural panel lines and details to surfaces
vec3 addProceduralDetail(vec3 worldPos, vec3 baseColor, float blockType)
{
    if (!enableProceduralDetails) return baseColor;
    
    float strength = proceduralStrength;
    
    // Hull blocks (0) - sleek panel lines with metallic sheen
    if (blockType < 0.5) {
        float gridSize = 2.0;
        vec3 gridPos = fract(worldPos / gridSize);
        float panelLines = 0.0;
        
        // Create subtle grid lines (less pronounced for cleaner look)
        float lineWidth = 0.03;
        if (gridPos.x < lineWidth || gridPos.x > (1.0 - lineWidth) || 
            gridPos.y < lineWidth || gridPos.y > (1.0 - lineWidth) || 
            gridPos.z < lineWidth || gridPos.z > (1.0 - lineWidth)) {
            panelLines = -0.1 * strength; // Subtle panel separation
        }
        
        // Add very subtle noise for panel variation
        float noise = hash(floor(worldPos / gridSize)) * 0.03 * strength;
        return baseColor * (1.0 + panelLines + noise);
    }
    // Armor blocks (1) - heavier plating with beveled edges
    else if (blockType < 1.5) {
        float gridSize = 2.5;
        vec3 gridPos = fract(worldPos / gridSize);
        float armorDetail = 0.0;
        
        // Beveled edge effect
        float bevelWidth = 0.06;
        float edgeDist = min(min(gridPos.x, 1.0 - gridPos.x), min(gridPos.y, 1.0 - gridPos.y));
        edgeDist = min(edgeDist, min(gridPos.z, 1.0 - gridPos.z));
        
        if (edgeDist < bevelWidth) {
            armorDetail = -0.15 * (1.0 - edgeDist / bevelWidth) * strength; // Smooth dark edge
        }
        
        return baseColor * (1.0 + armorDetail);
    }
    // Engines/Thrusters (2-4) - glowing heat patterns
    else if (blockType < 4.5) {
        float ventSize = 1.2;
        vec3 ventPos = fract(worldPos / ventSize);
        
        // Glowing vent lines
        float ventGlow = sin(ventPos.y * 8.0) * 0.5 + 0.5;
        ventGlow = pow(ventGlow, 3.0) * 0.4 * strength;
        
        // Hot glow gradient
        vec3 hotColor = mix(vec3(1.0, 0.3, 0.1), vec3(1.0, 0.8, 0.2), ventGlow);
        return baseColor + hotColor * ventGlow * 0.5;
    }
    // Generators (5) - pulsing energy core
    else if (blockType < 5.5) {
        vec3 corePos = fract(worldPos * 1.5);
        float coreDist = length(corePos - 0.5);
        
        // Energy pulse rings
        float energyRing = sin(coreDist * 25.0) * 0.5 + 0.5;
        energyRing = pow(energyRing, 2.0) * 0.3 * strength;
        
        // Blue energy glow
        vec3 energyGlow = vec3(0.3, 0.5, 1.0) * (1.0 - coreDist * 1.5) * 0.5 * strength;
        return baseColor * (1.0 + energyRing * 0.2) + energyGlow;
    }
    // Shield Generators (6) - hexagonal energy pattern
    else if (blockType < 6.5) {
        vec3 hexPos = worldPos * 2.5;
        float hexPattern = abs(sin(hexPos.x) + sin(hexPos.x * 0.5 + hexPos.y * 0.866) + 
                               sin(hexPos.x * 0.5 - hexPos.y * 0.866));
        hexPattern = pow(hexPattern / 3.0, 0.5) * strength;
        
        // Cyan shield glow
        vec3 shieldGlow = vec3(0.2, 0.6, 0.9) * hexPattern * 0.4;
        return baseColor + shieldGlow;
    }
    
    return baseColor;
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    float NdotV = max(dot(N, V), 0.0);

    // Use vertex color as the base color
    vec3 blockColor = VertexColor.rgb;
    float blockType = VertexColor.a;
    
    // Boost color saturation for more vibrant appearance using HSV-like approach
    vec3 greyscale = vec3(dot(blockColor, vec3(0.299, 0.587, 0.114)));
    vec3 saturatedColor = greyscale + (blockColor - greyscale) * 1.3; // Increase saturation by 30%
    saturatedColor = clamp(saturatedColor, 0.0, 1.0);
    
    // Apply procedural details
    blockColor = addProceduralDetail(FragPos, saturatedColor, blockType);
    
    // === Calculate Ambient Occlusion ===
    float ao = calculateBlockAO(LocalPos, N);

    // Enhanced F0 for metals (higher base reflectivity for shinier look)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, blockColor, metallic);
    
    // Use lower roughness for more mirror-like reflections
    float effectiveRoughness = roughness * 0.7; // Make everything a bit shinier

    vec3 Lo = vec3(0.0);

    // Calculate lighting from each light source
    for(int i = 0; i < 3; i++)
    {
        vec3 L = normalize(lightPos[i] - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPos[i] - FragPos);
        float attenuation = 1.0 / (distance * distance * 0.00008 + 1.0); // Slower falloff
        vec3 radiance = lightColor[i] * lightIntensity[i] * attenuation;

        // Cook-Torrance BRDF with enhanced specular
        float NDF = DistributionGGX(N, H, effectiveRoughness);
        float G = GeometrySmith(N, V, L, effectiveRoughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * NdotV * max(dot(N, L), 0.0);
        vec3 specular = numerator / max(denominator, 0.001);
        
        // Boost specular for highly metallic materials (mirror effect)
        specular *= (1.0 + metallic * 0.5);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);
        
        // Apply cel-shading if enabled (NPR mode)
        if (enableCelShading && renderingMode == 1) {
            NdotL = celShade(NdotL, celShadingBands);
        }
        
        Lo += (kD * blockColor / PI + specular) * radiance * NdotL;
    }

    // Environment reflection for metallic surfaces (simulated IBL)
    vec3 R = reflect(-V, N);
    vec3 envReflection = getEnvironmentReflection(R, effectiveRoughness);
    vec3 kSEnv = fresnelSchlickRoughness(NdotV, F0, effectiveRoughness);
    vec3 envSpecular = envReflection * kSEnv * metallic * 0.8; // Scaled environment contribution

    // Enhanced rim lighting for dramatic edge glow
    vec3 rimColor = vec3(0.0);
    if (enableRimLighting) {
        float rimFactor = 1.0 - NdotV;
        rimFactor = pow(rimFactor, 3.0);
        rimColor = mix(vec3(0.5, 0.6, 0.8), blockColor, 0.3) * rimFactor * rimStrength;
    }

    // Ambient with AO applied
    vec3 ambient = ambientLight * blockColor * ao;
    
    // Enhanced emissive with bloom-like effect
    vec3 emissive = emissiveColor * emissiveStrength * 1.5; // Boosted emissive
    
    // Extra glow for functional blocks
    if (enableBlockGlow && blockType >= 2.0 && blockType < 7.0) {
        float functionalGlow = (blockType >= 5.0) ? 0.4 : 0.25;
        emissive += blockColor * functionalGlow * blockGlowIntensity;
    }

    // Combine all lighting components
    vec3 color = ambient + Lo + envSpecular + rimColor + emissive;
    
    // === NPR Edge Detection (Hybrid and NPR modes) ===
    if (enableEdgeDetection && (renderingMode == 1 || renderingMode == 2)) {
        float edge = detectBlockEdge(LocalPos, edgeThickness);
        
        // Blend edge color
        color = mix(color, edgeColor, edge * 0.8);
    }

    // ACES Filmic Tone Mapping (approximation by Krzysztof Narkowicz)
    // These coefficients produce a cinematic, film-like response curve
    // that handles high dynamic range while preserving color and contrast
    float a = 2.51;  // Shoulder strength
    float b = 0.03;  // Linear strength  
    float c = 2.43;  // Linear angle
    float d = 0.59;  // Toe strength
    float e = 0.14;  // Toe numerator
    color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    // Subtle color grading for more vibrant output
    color = mix(color, color * vec3(1.02, 1.0, 1.05), 0.3); // Slight warm-cool split

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
        
        // === Set NPR/Rendering Configuration Uniforms ===
        // This addresses visual issues on blocks by providing flexible rendering options
        _shader.SetInt("renderingMode", (int)_config.Mode);
        _shader.SetBool("enableEdgeDetection", _config.EnableEdgeDetection);
        _shader.SetFloat("edgeThickness", _config.EdgeThickness);
        _shader.SetVector3("edgeColor", _config.EdgeColor);
        _shader.SetBool("enableCelShading", _config.EnableCelShading);
        _shader.SetInt("celShadingBands", _config.CelShadingBands);
        _shader.SetBool("enableAmbientOcclusion", _config.EnableAmbientOcclusion);
        _shader.SetFloat("aoStrength", _config.AmbientOcclusionStrength);
        _shader.SetBool("enableProceduralDetails", _config.EnableProceduralDetails);
        _shader.SetFloat("proceduralStrength", _config.ProceduralDetailStrength);
        _shader.SetBool("enableBlockGlow", _config.EnableBlockGlow);
        _shader.SetFloat("blockGlowIntensity", _config.BlockGlowIntensity);
        _shader.SetBool("enableRimLighting", _config.EnableRimLighting);
        _shader.SetFloat("rimStrength", _config.RimLightingStrength);
        _shader.SetBool("enableEnvReflections", _config.EnableEnvironmentReflections);

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
    /// Set the rendering configuration for this renderer
    /// </summary>
    public void SetConfiguration(RenderingConfiguration config)
    {
        _config = config;
    }
    
    /// <summary>
    /// Get the current rendering configuration
    /// </summary>
    public RenderingConfiguration GetConfiguration()
    {
        return _config;
    }
    
    /// <summary>
    /// Apply a rendering preset to quickly change visual style
    /// </summary>
    public void ApplyPreset(RenderingPreset preset)
    {
        _config.ApplyPreset(preset);
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
            
            // Color (convert from uint RGB to RGB floats)
            uint color = optimizedMesh.Colors[i];
            vertexData[offset + 6] = ((color >> 16) & 0xFF) / 255.0f; // R
            vertexData[offset + 7] = ((color >> 8) & 0xFF) / 255.0f;  // G
            vertexData[offset + 8] = (color & 0xFF) / 255.0f;         // B
            
            // Alpha channel stores block type information
            vertexData[offset + 9] = optimizedMesh.BlockTypes[i];     // Block type (0=Hull, 1=Armor, 2=Engine, etc.)
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
