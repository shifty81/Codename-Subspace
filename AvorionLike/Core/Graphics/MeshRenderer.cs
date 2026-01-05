using Silk.NET.OpenGL;
using System;
using System.Collections.Generic;
using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Renders 3D meshes loaded from model files
/// Supports instanced rendering and efficient mesh management
/// </summary>
public class MeshRenderer : IDisposable
{
    private readonly GL _gl;
    private readonly Logger _logger = Logger.Instance;
    private Shader? _shader;
    private readonly Dictionary<MeshData, MeshBuffers> _meshBuffers;
    private readonly Dictionary<string, uint> _textureCache;
    private bool _disposed = false;
    
    // Lighting configuration
    private static readonly Vector3 DefaultLightPosition = new Vector3(100, 200, 100);
    private static readonly Vector3 DefaultLightColor = new Vector3(1.0f, 1.0f, 1.0f);
    
    /// <summary>
    /// Represents OpenGL buffers for a mesh
    /// </summary>
    private class MeshBuffers
    {
        public uint VAO { get; set; }
        public uint VBO { get; set; }
        public uint EBO { get; set; }
        public int IndexCount { get; set; }
    }
    
    /// <summary>
    /// PBR texture set for a ship
    /// </summary>
    public class ShipTextures
    {
        public uint? BaseColorTexture { get; set; }
        public uint? NormalTexture { get; set; }
        public uint? MetallicRoughnessTexture { get; set; }
        public uint? EmissiveTexture { get; set; }
    }
    
    public MeshRenderer(GL gl)
    {
        _gl = gl;
        _meshBuffers = new Dictionary<MeshData, MeshBuffers>();
        _textureCache = new Dictionary<string, uint>();
        InitializeShader();
    }
    
    private void InitializeShader()
    {
        // Vertex shader
        string vertexShaderSource = @"
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
                FragPos = vec3(model * vec4(aPosition, 1.0));
                Normal = mat3(transpose(inverse(model))) * aNormal;
                TexCoord = aTexCoord;
                gl_Position = projection * view * model * vec4(aPosition, 1.0);
            }
        ";
        
        // Fragment shader with texture support (PBR-like)
        string fragmentShaderSource = @"
            #version 330 core
            out vec4 FragColor;
            
            in vec3 FragPos;
            in vec3 Normal;
            in vec2 TexCoord;
            
            uniform vec3 objectColor;
            uniform vec3 lightPos;
            uniform vec3 lightColor;
            uniform vec3 viewPos;
            uniform bool useTexture;
            uniform sampler2D textureSampler;
            uniform sampler2D normalMap;
            uniform sampler2D metallicMap;
            uniform sampler2D emissiveMap;
            uniform bool hasNormalMap;
            uniform bool hasMetallicMap;
            uniform bool hasEmissiveMap;
            
            void main()
            {
                // Get base color (from texture or uniform)
                vec3 baseColor = useTexture ? texture(textureSampler, TexCoord).rgb : objectColor;
                
                // Get normal (from normal map or vertex normal)
                vec3 norm = normalize(Normal);
                if (hasNormalMap && useTexture)
                {
                    vec3 normalMapValue = texture(normalMap, TexCoord).rgb * 2.0 - 1.0;
                    // Simple normal mapping (proper TBN would be better)
                    norm = normalize(Normal + normalMapValue * 0.5);
                }
                
                // Get metallic and roughness
                float metallic = 0.3;
                float roughness = 0.5;
                if (hasMetallicMap && useTexture)
                {
                    vec4 metallicRoughness = texture(metallicMap, TexCoord);
                    metallic = metallicRoughness.b; // Metallic in blue channel
                    roughness = metallicRoughness.g; // Roughness in green channel
                }
                
                // Ambient
                float ambientStrength = 0.3;
                vec3 ambient = ambientStrength * lightColor;
                
                // Diffuse
                vec3 lightDir = normalize(lightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * lightColor;
                
                // Specular
                float specularStrength = mix(0.5, 0.9, metallic);
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, norm);
                float shininess = mix(32.0, 128.0, 1.0 - roughness);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
                vec3 specular = specularStrength * spec * lightColor;
                
                // Emissive
                vec3 emissive = vec3(0.0);
                if (hasEmissiveMap && useTexture)
                {
                    emissive = texture(emissiveMap, TexCoord).rgb;
                }
                
                vec3 result = (ambient + diffuse + specular) * baseColor + emissive;
                FragColor = vec4(result, 1.0);
            }
        ";
        
        _shader = new Shader(_gl, vertexShaderSource, fragmentShaderSource);
        _logger.Info("MeshRenderer", "Mesh renderer shader initialized with texture support");
    }
    
    /// <summary>
    /// Prepares a mesh for rendering by creating GPU buffers
    /// </summary>
    public void PrepareMesh(MeshData mesh)
    {
        if (_meshBuffers.ContainsKey(mesh))
        {
            _logger.Debug("MeshRenderer", $"Mesh '{mesh.Name}' already prepared");
            return;
        }
        
        if (!mesh.IsValid(out string errorMessage))
        {
            _logger.Error("MeshRenderer", $"Cannot prepare invalid mesh '{mesh.Name}': {errorMessage}");
            return;
        }
        
        var buffers = CreateMeshBuffers(mesh);
        _meshBuffers[mesh] = buffers;
        
        _logger.Debug("MeshRenderer", $"Mesh '{mesh.Name}' prepared: {mesh.VertexCount} vertices, {mesh.TriangleCount} triangles");
    }
    
    private unsafe MeshBuffers CreateMeshBuffers(MeshData mesh)
    {
        var buffers = new MeshBuffers
        {
            VAO = _gl.GenVertexArray(),
            VBO = _gl.GenBuffer(),
            EBO = _gl.GenBuffer(),
            IndexCount = mesh.Indices.Length
        };
        
        _gl.BindVertexArray(buffers.VAO);
        
        // Interleave vertex data: position, normal, texcoord
        var vertexData = new List<float>();
        for (int i = 0; i < mesh.Vertices.Length; i++)
        {
            // Position
            vertexData.Add(mesh.Vertices[i].X);
            vertexData.Add(mesh.Vertices[i].Y);
            vertexData.Add(mesh.Vertices[i].Z);
            
            // Normal
            vertexData.Add(mesh.Normals[i].X);
            vertexData.Add(mesh.Normals[i].Y);
            vertexData.Add(mesh.Normals[i].Z);
            
            // TexCoord
            vertexData.Add(mesh.TexCoords[i].X);
            vertexData.Add(mesh.TexCoords[i].Y);
        }
        
        var vertexArray = vertexData.ToArray();
        
        // Upload vertex data
        _gl.BindBuffer(BufferTargetARB.ArrayBuffer, buffers.VBO);
        fixed (float* v = &vertexArray[0])
        {
            _gl.BufferData(BufferTargetARB.ArrayBuffer, (nuint)(vertexArray.Length * sizeof(float)), v, BufferUsageARB.StaticDraw);
        }
        
        // Upload index data
        _gl.BindBuffer(BufferTargetARB.ElementArrayBuffer, buffers.EBO);
        fixed (uint* i = &mesh.Indices[0])
        {
            _gl.BufferData(BufferTargetARB.ElementArrayBuffer, (nuint)(mesh.Indices.Length * sizeof(uint)), i, BufferUsageARB.StaticDraw);
        }
        
        int stride = 8 * sizeof(float); // 3 (pos) + 3 (normal) + 2 (texcoord)
        
        // Position attribute
        _gl.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, (uint)stride, (void*)0);
        _gl.EnableVertexAttribArray(0);
        
        // Normal attribute
        _gl.VertexAttribPointer(1, 3, VertexAttribPointerType.Float, false, (uint)stride, (void*)(3 * sizeof(float)));
        _gl.EnableVertexAttribArray(1);
        
        // TexCoord attribute
        _gl.VertexAttribPointer(2, 2, VertexAttribPointerType.Float, false, (uint)stride, (void*)(6 * sizeof(float)));
        _gl.EnableVertexAttribArray(2);
        
        _gl.BindVertexArray(0);
        
        return buffers;
    }
    
    /// <summary>
    /// Renders a mesh at the specified transformation
    /// </summary>
    public unsafe void RenderMesh(MeshData mesh, Matrix4x4 modelMatrix, Vector3 color, Matrix4x4 viewMatrix, Matrix4x4 projectionMatrix, Vector3 cameraPosition)
    {
        RenderMesh(mesh, modelMatrix, color, viewMatrix, projectionMatrix, cameraPosition, null);
    }
    
    /// <summary>
    /// Renders a mesh with optional textures
    /// </summary>
    public unsafe void RenderMesh(MeshData mesh, Matrix4x4 modelMatrix, Vector3 color, Matrix4x4 viewMatrix, Matrix4x4 projectionMatrix, Vector3 cameraPosition, ShipTextures? textures)
    {
        if (_shader == null)
        {
            _logger.Error("MeshRenderer", "Shader not initialized");
            return;
        }
        
        // Prepare mesh if not already done
        if (!_meshBuffers.ContainsKey(mesh))
        {
            PrepareMesh(mesh);
        }
        
        if (!_meshBuffers.TryGetValue(mesh, out var buffers))
        {
            _logger.Warning("MeshRenderer", $"Failed to get buffers for mesh '{mesh.Name}'");
            return;
        }
        
        _shader.Use();
        
        // Set matrices
        _shader.SetMatrix4("model", modelMatrix);
        _shader.SetMatrix4("view", viewMatrix);
        _shader.SetMatrix4("projection", projectionMatrix);
        
        // Set lighting uniforms
        _shader.SetVector3("objectColor", color);
        _shader.SetVector3("lightPos", DefaultLightPosition);
        _shader.SetVector3("lightColor", DefaultLightColor);
        _shader.SetVector3("viewPos", cameraPosition);
        
        // Set texture uniforms
        bool useTexture = textures?.BaseColorTexture.HasValue ?? false;
        _shader.SetInt("useTexture", useTexture ? 1 : 0);
        
        if (useTexture && textures != null)
        {
            // Bind base color texture
            if (textures.BaseColorTexture.HasValue)
            {
                _gl.ActiveTexture(TextureUnit.Texture0);
                _gl.BindTexture(TextureTarget.Texture2D, textures.BaseColorTexture.Value);
                _shader.SetInt("textureSampler", 0);
            }
            
            // Bind normal map
            if (textures.NormalTexture.HasValue)
            {
                _gl.ActiveTexture(TextureUnit.Texture1);
                _gl.BindTexture(TextureTarget.Texture2D, textures.NormalTexture.Value);
                _shader.SetInt("normalMap", 1);
                _shader.SetInt("hasNormalMap", 1);
            }
            else
            {
                _shader.SetInt("hasNormalMap", 0);
            }
            
            // Bind metallic/roughness map
            if (textures.MetallicRoughnessTexture.HasValue)
            {
                _gl.ActiveTexture(TextureUnit.Texture2);
                _gl.BindTexture(TextureTarget.Texture2D, textures.MetallicRoughnessTexture.Value);
                _shader.SetInt("metallicMap", 2);
                _shader.SetInt("hasMetallicMap", 1);
            }
            else
            {
                _shader.SetInt("hasMetallicMap", 0);
            }
            
            // Bind emissive map
            if (textures.EmissiveTexture.HasValue)
            {
                _gl.ActiveTexture(TextureUnit.Texture3);
                _gl.BindTexture(TextureTarget.Texture2D, textures.EmissiveTexture.Value);
                _shader.SetInt("emissiveMap", 3);
                _shader.SetInt("hasEmissiveMap", 1);
            }
            else
            {
                _shader.SetInt("hasEmissiveMap", 0);
            }
        }
        
        // Render
        _gl.BindVertexArray(buffers.VAO);
        _gl.DrawElements(PrimitiveType.Triangles, (uint)buffers.IndexCount, DrawElementsType.UnsignedInt, (void*)0);
        _gl.BindVertexArray(0);
        
        // Unbind textures
        if (useTexture)
        {
            _gl.ActiveTexture(TextureUnit.Texture0);
            _gl.BindTexture(TextureTarget.Texture2D, 0);
        }
    }
    
    /// <summary>
    /// Renders multiple instances of a mesh
    /// </summary>
    public void RenderMeshInstanced(MeshData mesh, List<(Matrix4x4 transform, Vector3 color)> instances, Matrix4x4 viewMatrix, Matrix4x4 projectionMatrix, Vector3 cameraPosition)
    {
        if (instances.Count == 0)
            return;
        
        foreach (var (transform, color) in instances)
        {
            RenderMesh(mesh, transform, color, viewMatrix, projectionMatrix, cameraPosition);
        }
    }
    
    /// <summary>
    /// Removes a mesh from GPU memory
    /// </summary>
    public void ReleaseMesh(MeshData mesh)
    {
        if (_meshBuffers.TryGetValue(mesh, out var buffers))
        {
            _gl.DeleteVertexArray(buffers.VAO);
            _gl.DeleteBuffer(buffers.VBO);
            _gl.DeleteBuffer(buffers.EBO);
            _meshBuffers.Remove(mesh);
            
            _logger.Debug("MeshRenderer", $"Mesh '{mesh.Name}' released from GPU");
        }
    }
    
    /// <summary>
    /// Gets statistics about loaded meshes
    /// </summary>
    public (int MeshCount, long EstimatedGPUMemory) GetStats()
    {
        long memory = 0;
        
        foreach (var kvp in _meshBuffers)
        {
            var mesh = kvp.Key;
            // Estimate: 8 floats per vertex (pos+normal+texcoord) + indices
            memory += mesh.VertexCount * 8 * sizeof(float);
            memory += mesh.Indices.Length * sizeof(uint);
        }
        
        return (_meshBuffers.Count, memory);
    }
    
    /// <summary>
    /// Load a texture from file path
    /// </summary>
    public uint? LoadTexture(string texturePath)
    {
        if (string.IsNullOrEmpty(texturePath))
            return null;
        
        // Check cache
        if (_textureCache.TryGetValue(texturePath, out var cachedTexture))
        {
            return cachedTexture;
        }
        
        try
        {
            // Get full path via AssetManager
            var fullPath = AssetManager.Instance.GetAssetPath(texturePath);
            
            if (!File.Exists(fullPath))
            {
                _logger.Warning("MeshRenderer", $"Texture file not found: {texturePath}");
                return null;
            }
            
            // Load image using SixLabors.ImageSharp
            using var image = SixLabors.ImageSharp.Image.Load<SixLabors.ImageSharp.PixelFormats.Rgba32>(fullPath);
            
            // Flip image vertically (OpenGL expects bottom-left origin)
            image.Mutate(x => x.Flip(SixLabors.ImageSharp.Processing.FlipMode.Vertical));
            
            // Create OpenGL texture
            uint textureId = _gl.GenTexture();
            _gl.BindTexture(TextureTarget.Texture2D, textureId);
            
            // Copy image data to GPU
            var pixels = new byte[image.Width * image.Height * 4];
            image.CopyPixelDataTo(pixels);
            
            unsafe
            {
                fixed (byte* ptr = pixels)
                {
                    _gl.TexImage2D(
                        TextureTarget.Texture2D,
                        0,
                        InternalFormat.Rgba,
                        (uint)image.Width,
                        (uint)image.Height,
                        0,
                        PixelFormat.Rgba,
                        PixelType.UnsignedByte,
                        ptr
                    );
                }
            }
            
            // Set texture parameters
            _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapS, (int)GLEnum.Repeat);
            _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureWrapT, (int)GLEnum.Repeat);
            _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMinFilter, (int)GLEnum.LinearMipmapLinear);
            _gl.TexParameter(TextureTarget.Texture2D, TextureParameterName.TextureMagFilter, (int)GLEnum.Linear);
            
            // Generate mipmaps
            _gl.GenerateMipmap(TextureTarget.Texture2D);
            
            _gl.BindTexture(TextureTarget.Texture2D, 0);
            
            // Cache the texture
            _textureCache[texturePath] = textureId;
            
            _logger.Info("MeshRenderer", $"Loaded texture: {texturePath} ({image.Width}x{image.Height})");
            
            return textureId;
        }
        catch (Exception ex)
        {
            _logger.Error("MeshRenderer", $"Failed to load texture {texturePath}: {ex.Message}", ex);
            return null;
        }
    }
    
    /// <summary>
    /// Load ship textures from directory
    /// </summary>
    public ShipTextures? LoadShipTextures(string baseTexturePath, string normalPath = "", string metallicPath = "", string emissivePath = "")
    {
        var textures = new ShipTextures();
        
        textures.BaseColorTexture = LoadTexture(baseTexturePath);
        
        if (!string.IsNullOrEmpty(normalPath))
            textures.NormalTexture = LoadTexture(normalPath);
        
        if (!string.IsNullOrEmpty(metallicPath))
            textures.MetallicRoughnessTexture = LoadTexture(metallicPath);
        
        if (!string.IsNullOrEmpty(emissivePath))
            textures.EmissiveTexture = LoadTexture(emissivePath);
        
        // Return null if no textures loaded
        if (!textures.BaseColorTexture.HasValue && 
            !textures.NormalTexture.HasValue && 
            !textures.MetallicRoughnessTexture.HasValue && 
            !textures.EmissiveTexture.HasValue)
        {
            return null;
        }
        
        return textures;
    }
    
    public void Dispose()
    {
        if (_disposed)
            return;
        
        // Release all mesh buffers
        foreach (var buffers in _meshBuffers.Values)
        {
            _gl.DeleteVertexArray(buffers.VAO);
            _gl.DeleteBuffer(buffers.VBO);
            _gl.DeleteBuffer(buffers.EBO);
        }
        
        // Release all textures
        foreach (var textureId in _textureCache.Values)
        {
            _gl.DeleteTexture(textureId);
        }
        
        _meshBuffers.Clear();
        _textureCache.Clear();
        _shader?.Dispose();
        
        _disposed = true;
        _logger.Info("MeshRenderer", "MeshRenderer disposed");
    }
}
