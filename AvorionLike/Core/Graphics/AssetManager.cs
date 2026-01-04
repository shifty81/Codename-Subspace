using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Manages loading, caching, and retrieval of game assets (3D models, textures, etc.)
/// Implements singleton pattern for global access
/// </summary>
public class AssetManager
{
    private static AssetManager? _instance;
    private static readonly object _lock = new();
    
    private readonly Logger _logger = Logger.Instance;
    private readonly ModelLoader _modelLoader;
    private readonly Dictionary<string, List<MeshData>> _loadedModels;
    private readonly string _assetBasePath;
    
    public static AssetManager Instance
    {
        get
        {
            if (_instance == null)
            {
                lock (_lock)
                {
                    _instance ??= new AssetManager();
                }
            }
            return _instance;
        }
    }
    
    private AssetManager()
    {
        _modelLoader = new ModelLoader();
        _loadedModels = new Dictionary<string, List<MeshData>>();
        
        // Determine asset base path (relative to executable)
        var appDirectory = AppDomain.CurrentDomain.BaseDirectory;
        _assetBasePath = Path.Combine(appDirectory, "Assets");
        
        // Create Assets directory if it doesn't exist
        if (!Directory.Exists(_assetBasePath))
        {
            _logger.Info("AssetManager", $"Creating assets directory: {_assetBasePath}");
            Directory.CreateDirectory(_assetBasePath);
            
            // Create subdirectories
            Directory.CreateDirectory(Path.Combine(_assetBasePath, "Models"));
            Directory.CreateDirectory(Path.Combine(_assetBasePath, "Textures"));
            Directory.CreateDirectory(Path.Combine(_assetBasePath, "Audio"));
        }
        
        _logger.Info("AssetManager", $"AssetManager initialized. Asset path: {_assetBasePath}");
    }
    
    /// <summary>
    /// Gets the full path to an asset file
    /// </summary>
    public string GetAssetPath(string relativePath)
    {
        return Path.Combine(_assetBasePath, relativePath);
    }
    
    /// <summary>
    /// Loads a 3D model from the Assets/Models directory
    /// Uses caching - subsequent requests for the same model will return cached data
    /// </summary>
    /// <param name="modelPath">Relative path from Assets/Models (e.g., "ships/fighter.obj")</param>
    /// <returns>List of meshes in the model</returns>
    public List<MeshData> LoadModel(string modelPath)
    {
        // Check cache first
        if (_loadedModels.TryGetValue(modelPath, out var cachedModel))
        {
            _logger.Debug("AssetManager", $"Returning cached model: {modelPath}");
            return cachedModel;
        }
        
        // Normalize path separators to ensure cross-platform compatibility
        var normalizedPath = modelPath.Replace('/', Path.DirectorySeparatorChar).Replace('\\', Path.DirectorySeparatorChar);
        
        // Build full path
        var fullPath = Path.Combine(_assetBasePath, "Models", normalizedPath);
        
        if (!File.Exists(fullPath))
        {
            _logger.Error("AssetManager", $"Model file not found: {fullPath}");
            throw new FileNotFoundException($"Model file not found: {modelPath}");
        }
        
        // Load the model
        var meshes = _modelLoader.LoadModel(fullPath);
        
        // Cache it
        _loadedModels[modelPath] = meshes;
        
        _logger.Info("AssetManager", $"Model loaded and cached: {modelPath} ({meshes.Count} meshes)");
        
        return meshes;
    }
    
    /// <summary>
    /// Loads a 3D model from an absolute path (bypasses asset directory structure)
    /// </summary>
    public List<MeshData> LoadModelFromPath(string absolutePath)
    {
        // Check cache using absolute path as key
        if (_loadedModels.TryGetValue(absolutePath, out var cachedModel))
        {
            _logger.Debug("AssetManager", $"Returning cached model: {absolutePath}");
            return cachedModel;
        }
        
        // Load the model
        var meshes = _modelLoader.LoadModel(absolutePath);
        
        // Cache it
        _loadedModels[absolutePath] = meshes;
        
        _logger.Info("AssetManager", $"Model loaded and cached: {absolutePath} ({meshes.Count} meshes)");
        
        return meshes;
    }
    
    /// <summary>
    /// Preloads a model into cache without returning it
    /// Useful for loading screen preloading
    /// </summary>
    public void PreloadModel(string modelPath)
    {
        LoadModel(modelPath);
    }
    
    /// <summary>
    /// Unloads a model from cache
    /// </summary>
    public void UnloadModel(string modelPath)
    {
        if (_loadedModels.Remove(modelPath))
        {
            _logger.Info("AssetManager", $"Model unloaded from cache: {modelPath}");
        }
    }
    
    /// <summary>
    /// Clears all cached assets
    /// </summary>
    public void ClearCache()
    {
        _loadedModels.Clear();
        _logger.Info("AssetManager", "Asset cache cleared");
    }
    
    /// <summary>
    /// Gets statistics about cached assets
    /// </summary>
    public (int ModelCount, int TotalMeshes, long EstimatedMemoryBytes) GetCacheStats()
    {
        int modelCount = _loadedModels.Count;
        int totalMeshes = 0;
        long estimatedMemory = 0;
        
        foreach (var meshList in _loadedModels.Values)
        {
            totalMeshes += meshList.Count;
            foreach (var mesh in meshList)
            {
                // Estimate memory usage
                // Vertices: 12 bytes each (3 floats)
                // Normals: 12 bytes each
                // TexCoords: 8 bytes each (2 floats)
                // Indices: 4 bytes each
                estimatedMemory += mesh.Vertices.Length * 12;
                estimatedMemory += mesh.Normals.Length * 12;
                estimatedMemory += mesh.TexCoords.Length * 8;
                estimatedMemory += mesh.Indices.Length * 4;
                
                if (mesh.Colors != null)
                {
                    estimatedMemory += mesh.Colors.Length * 16; // 4 floats
                }
            }
        }
        
        return (modelCount, totalMeshes, estimatedMemory);
    }
    
    /// <summary>
    /// Checks if a model exists in the asset directory
    /// </summary>
    public bool ModelExists(string modelPath)
    {
        // Normalize path separators to ensure cross-platform compatibility
        var normalizedPath = modelPath.Replace('/', Path.DirectorySeparatorChar).Replace('\\', Path.DirectorySeparatorChar);
        var fullPath = Path.Combine(_assetBasePath, "Models", normalizedPath);
        return File.Exists(fullPath);
    }
    
    /// <summary>
    /// Gets a list of all model files in the Assets/Models directory
    /// </summary>
    public string[] GetAvailableModels()
    {
        var modelsPath = Path.Combine(_assetBasePath, "Models");
        if (!Directory.Exists(modelsPath))
            return Array.Empty<string>();
        
        var supportedExtensions = new[] { ".obj", ".fbx", ".gltf", ".glb", ".dae", ".blend", ".3ds" };
        var files = Directory.GetFiles(modelsPath, "*.*", SearchOption.AllDirectories)
            .Where(file => supportedExtensions.Contains(Path.GetExtension(file).ToLower()))
            .Select(file => Path.GetRelativePath(modelsPath, file))
            .ToArray();
        
        return files;
    }
    
    /// <summary>
    /// Creates a placeholder cube mesh for testing or fallback
    /// </summary>
    public MeshData CreatePlaceholderCube(float size = 1.0f)
    {
        float halfSize = size * 0.5f;
        
        var cube = new MeshData
        {
            Name = "Placeholder Cube",
            Vertices = new[]
            {
                // Front face
                new Vector3(-halfSize, -halfSize, halfSize),
                new Vector3(halfSize, -halfSize, halfSize),
                new Vector3(halfSize, halfSize, halfSize),
                new Vector3(-halfSize, halfSize, halfSize),
                
                // Back face
                new Vector3(-halfSize, -halfSize, -halfSize),
                new Vector3(-halfSize, halfSize, -halfSize),
                new Vector3(halfSize, halfSize, -halfSize),
                new Vector3(halfSize, -halfSize, -halfSize),
                
                // Top face
                new Vector3(-halfSize, halfSize, -halfSize),
                new Vector3(-halfSize, halfSize, halfSize),
                new Vector3(halfSize, halfSize, halfSize),
                new Vector3(halfSize, halfSize, -halfSize),
                
                // Bottom face
                new Vector3(-halfSize, -halfSize, -halfSize),
                new Vector3(halfSize, -halfSize, -halfSize),
                new Vector3(halfSize, -halfSize, halfSize),
                new Vector3(-halfSize, -halfSize, halfSize),
                
                // Right face
                new Vector3(halfSize, -halfSize, -halfSize),
                new Vector3(halfSize, halfSize, -halfSize),
                new Vector3(halfSize, halfSize, halfSize),
                new Vector3(halfSize, -halfSize, halfSize),
                
                // Left face
                new Vector3(-halfSize, -halfSize, -halfSize),
                new Vector3(-halfSize, -halfSize, halfSize),
                new Vector3(-halfSize, halfSize, halfSize),
                new Vector3(-halfSize, halfSize, -halfSize),
            },
            
            Normals = new[]
            {
                // Front
                Vector3.UnitZ, Vector3.UnitZ, Vector3.UnitZ, Vector3.UnitZ,
                // Back
                -Vector3.UnitZ, -Vector3.UnitZ, -Vector3.UnitZ, -Vector3.UnitZ,
                // Top
                Vector3.UnitY, Vector3.UnitY, Vector3.UnitY, Vector3.UnitY,
                // Bottom
                -Vector3.UnitY, -Vector3.UnitY, -Vector3.UnitY, -Vector3.UnitY,
                // Right
                Vector3.UnitX, Vector3.UnitX, Vector3.UnitX, Vector3.UnitX,
                // Left
                -Vector3.UnitX, -Vector3.UnitX, -Vector3.UnitX, -Vector3.UnitX,
            },
            
            TexCoords = new[]
            {
                // Front
                new Vector2(0, 0), new Vector2(1, 0), new Vector2(1, 1), new Vector2(0, 1),
                // Back
                new Vector2(0, 0), new Vector2(1, 0), new Vector2(1, 1), new Vector2(0, 1),
                // Top
                new Vector2(0, 0), new Vector2(1, 0), new Vector2(1, 1), new Vector2(0, 1),
                // Bottom
                new Vector2(0, 0), new Vector2(1, 0), new Vector2(1, 1), new Vector2(0, 1),
                // Right
                new Vector2(0, 0), new Vector2(1, 0), new Vector2(1, 1), new Vector2(0, 1),
                // Left
                new Vector2(0, 0), new Vector2(1, 0), new Vector2(1, 1), new Vector2(0, 1),
            },
            
            Indices = new uint[]
            {
                // Front
                0, 1, 2, 0, 2, 3,
                // Back
                4, 5, 6, 4, 6, 7,
                // Top
                8, 9, 10, 8, 10, 11,
                // Bottom
                12, 13, 14, 12, 14, 15,
                // Right
                16, 17, 18, 16, 18, 19,
                // Left
                20, 21, 22, 20, 22, 23,
            }
        };
        
        return cube;
    }
}
