using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using Assimp;
using Assimp.Configs;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Loads 3D models from files using Assimp library.
/// Supports OBJ, FBX, GLTF, and many other formats.
/// </summary>
public class ModelLoader
{
    private readonly Logger _logger = Logger.Instance;
    private readonly AssimpContext _importer;
    
    public ModelLoader()
    {
        _importer = new AssimpContext();
        
        // Configure importer for optimal performance
        _importer.SetConfig(new NormalSmoothingAngleConfig(66.0f));
    }
    
    /// <summary>
    /// Loads a 3D model from a file
    /// </summary>
    /// <param name="filePath">Path to the model file</param>
    /// <param name="postProcessFlags">Post-processing flags (optional)</param>
    /// <returns>List of meshes in the model</returns>
    public List<MeshData> LoadModel(string filePath, PostProcessSteps? postProcessFlags = null)
    {
        if (!File.Exists(filePath))
        {
            _logger.Error("ModelLoader", $"Model file not found: {filePath}");
            throw new FileNotFoundException($"Model file not found: {filePath}");
        }
        
        _logger.Info("ModelLoader", $"Loading 3D model: {filePath}");
        
        try
        {
            // Default post-processing flags
            var flags = postProcessFlags ?? 
                        (PostProcessSteps.Triangulate |
                         PostProcessSteps.GenerateNormals |
                         PostProcessSteps.CalculateTangentSpace |
                         PostProcessSteps.JoinIdenticalVertices |
                         PostProcessSteps.OptimizeMeshes |
                         PostProcessSteps.FlipUVs);
            
            // Import the scene
            var scene = _importer.ImportFile(filePath, flags);
            
            if (scene == null || !scene.HasMeshes)
            {
                _logger.Error("ModelLoader", $"Failed to load model or model has no meshes: {filePath}");
                throw new InvalidDataException($"Failed to load model or model has no meshes: {filePath}");
            }
            
            _logger.Info("ModelLoader", $"Model loaded successfully: {scene.MeshCount} meshes found");
            
            // Convert all meshes
            var meshes = new List<MeshData>();
            foreach (var assimpMesh in scene.Meshes)
            {
                var meshData = ConvertMesh(assimpMesh);
                meshes.Add(meshData);
                
                _logger.Debug("ModelLoader", $"  Mesh '{meshData.Name}': {meshData.VertexCount} vertices, {meshData.TriangleCount} triangles");
            }
            
            return meshes;
        }
        catch (Exception ex)
        {
            _logger.Error("ModelLoader", $"Error loading model {filePath}: {ex.Message}", ex);
            throw;
        }
    }
    
    /// <summary>
    /// Converts an Assimp mesh to our MeshData format
    /// </summary>
    private MeshData ConvertMesh(Mesh assimpMesh)
    {
        var meshData = new MeshData
        {
            Name = string.IsNullOrEmpty(assimpMesh.Name) ? "Unnamed Mesh" : assimpMesh.Name,
            MaterialName = assimpMesh.MaterialIndex >= 0 ? $"Material_{assimpMesh.MaterialIndex}" : string.Empty
        };
        
        // Convert vertices
        meshData.Vertices = assimpMesh.Vertices
            .Select(v => new Vector3(v.X, v.Y, v.Z))
            .ToArray();
        
        // Convert normals
        if (assimpMesh.HasNormals)
        {
            meshData.Normals = assimpMesh.Normals
                .Select(n => new Vector3(n.X, n.Y, n.Z))
                .ToArray();
        }
        else
        {
            // Generate default normals if not present
            meshData.Normals = new Vector3[meshData.Vertices.Length];
            for (int i = 0; i < meshData.Normals.Length; i++)
            {
                meshData.Normals[i] = Vector3.UnitY; // Default up vector
            }
        }
        
        // Convert texture coordinates (use first UV channel)
        if (assimpMesh.HasTextureCoords(0))
        {
            meshData.TexCoords = assimpMesh.TextureCoordinateChannels[0]
                .Select(uv => new Vector2(uv.X, uv.Y))
                .ToArray();
        }
        else
        {
            // Generate default UVs if not present
            meshData.TexCoords = new Vector2[meshData.Vertices.Length];
            for (int i = 0; i < meshData.TexCoords.Length; i++)
            {
                meshData.TexCoords[i] = Vector2.Zero;
            }
        }
        
        // Convert vertex colors (use first color channel)
        if (assimpMesh.HasVertexColors(0))
        {
            meshData.Colors = assimpMesh.VertexColorChannels[0]
                .Select(c => new Vector4(c.R, c.G, c.B, c.A))
                .ToArray();
        }
        
        // Convert indices
        var indices = new List<uint>();
        foreach (var face in assimpMesh.Faces)
        {
            if (face.IndexCount == 3)
            {
                indices.Add((uint)face.Indices[0]);
                indices.Add((uint)face.Indices[1]);
                indices.Add((uint)face.Indices[2]);
            }
            else
            {
                _logger.Warning("ModelLoader", $"Face with {face.IndexCount} indices found (expected 3). This face will be skipped.");
            }
        }
        meshData.Indices = indices.ToArray();
        
        // Validate the mesh
        if (!meshData.IsValid(out string errorMessage))
        {
            _logger.Warning("ModelLoader", $"Mesh '{meshData.Name}' validation failed: {errorMessage}");
        }
        
        return meshData;
    }
    
    /// <summary>
    /// Checks if a file format is supported by Assimp
    /// </summary>
    public bool IsSupportedFormat(string extension)
    {
        extension = extension.TrimStart('.');
        var supportedFormats = _importer.GetSupportedImportFormats();
        return supportedFormats.Any(format => format.Equals(extension, StringComparison.OrdinalIgnoreCase));
    }
    
    /// <summary>
    /// Gets a list of all supported file formats
    /// </summary>
    public string[] GetSupportedFormats()
    {
        return _importer.GetSupportedImportFormats();
    }
    
    /// <summary>
    /// Disposes of the Assimp importer
    /// </summary>
    public void Dispose()
    {
        _importer?.Dispose();
    }
}
