using AvorionLike.Core.Graphics;
using AvorionLike.Core.Logging;
using System.Numerics;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Utility to check for and load the Ulysses starter ship model
/// Handles multiple file formats and fallback scenarios
/// </summary>
public static class UlyssesModelLoader
{
    private static readonly Logger _logger = Logger.Instance;
    
    /// <summary>
    /// Possible locations for the Ulysses model file
    /// Checks multiple paths and formats
    /// </summary>
    private static readonly string[] ModelSearchPaths = new[]
    {
        // Primary location - exported formats (Assimp-compatible)
        "Models/ships/Ulysses/source/ulysses.obj",
        "Models/ships/Ulysses/source/ulysses.fbx",
        "Models/ships/Ulysses/source/ulysses.gltf",
        "Models/ships/Ulysses/source/ulysses.glb",
        "Models/ships/Ulysses/ulysses.obj",
        "Models/ships/Ulysses/ulysses.fbx",
        "Models/ships/Ulysses/ulysses.gltf",
        "Models/ships/Ulysses/ulysses.glb",
        
        // Alternative locations in hulls
        "Models/ships/hulls/ulysses.obj",
        "Models/ships/hulls/ulysses.fbx",
        "Models/ships/hulls/ulysses.gltf",
        "Models/ships/hulls/ulysses.glb",
        
        // GameData locations (legacy support)
        "../GameData/Assets/Models/ships/Ulysses/source/ulysses.obj",
        "../GameData/Assets/Models/ships/Ulysses/ulysses.obj",
        "../GameData/Assets/Models/ships/hulls/ulysses.obj",
        
        // Root Assets locations
        "../Assets/Models/ships/Ulysses/source/ulysses.obj",
        "../Assets/Models/ships/Ulysses/ulysses.obj",
        "../Assets/Models/ships/hulls/ulysses.obj",
        "Assets/Models/ships/Ulysses/source/ulysses.obj",
        "Assets/Models/ships/Ulysses/ulysses.obj",
        "Assets/Models/ships/hulls/ulysses.obj"
    };
    
    /// <summary>
    /// Check if the Ulysses model file exists
    /// </summary>
    public static (bool exists, string? path, string? format) CheckForUlyssesModel()
    {
        var assetManager = AssetManager.Instance;
        
        foreach (var searchPath in ModelSearchPaths)
        {
            try
            {
                var fullPath = assetManager.GetAssetPath(searchPath);
                
                if (File.Exists(fullPath))
                {
                    var extension = Path.GetExtension(fullPath).ToLower();
                    var format = extension switch
                    {
                        ".obj" => "Wavefront OBJ",
                        ".fbx" => "Autodesk FBX",
                        ".gltf" => "glTF",
                        ".glb" => "glTF Binary",
                        ".dae" => "Collada",
                        _ => "Unknown"
                    };
                    
                    _logger.Info("UlyssesLoader", $"Found Ulysses model: {searchPath} ({format})");
                    return (true, searchPath, format);
                }
            }
            catch (Exception ex)
            {
                _logger.Debug("UlyssesLoader", $"Error checking path {searchPath}: {ex.Message}");
            }
        }
        
        return (false, null, null);
    }
    
    /// <summary>
    /// Load the Ulysses model if available
    /// </summary>
    public static List<MeshData>? LoadUlyssesModel()
    {
        var (exists, path, format) = CheckForUlyssesModel();
        
        if (!exists || path == null)
        {
            _logger.Warning("UlyssesLoader", "Ulysses model not found in any expected location");
            LogExpectedLocations();
            return null;
        }
        
        try
        {
            var assetManager = AssetManager.Instance;
            
            // LoadModel expects paths relative to Assets/Models/, but our paths are relative to Assets/
            // So we need to strip the "Models/" prefix if present
            var modelPath = path;
            const string modelsPrefix = "Models/";
            const string modelsPrefixAlt = "Models\\";
            
            if (modelPath.StartsWith(modelsPrefix, StringComparison.OrdinalIgnoreCase))
            {
                modelPath = modelPath.Substring(modelsPrefix.Length);
            }
            else if (modelPath.StartsWith(modelsPrefixAlt, StringComparison.OrdinalIgnoreCase))
            {
                modelPath = modelPath.Substring(modelsPrefixAlt.Length);
            }
            
            var meshes = assetManager.LoadModel(modelPath);
            
            _logger.Info("UlyssesLoader", $"Successfully loaded Ulysses model from {path}");
            _logger.Info("UlyssesLoader", $"Model contains {meshes.Count} mesh(es):");
            
            foreach (var mesh in meshes)
            {
                _logger.Info("UlyssesLoader", 
                    $"  - {mesh.Name}: {mesh.VertexCount} vertices, {mesh.TriangleCount} triangles");
            }
            
            return meshes;
        }
        catch (Exception ex)
        {
            _logger.Error("UlyssesLoader", $"Error loading Ulysses model: {ex.Message}", ex);
            return null;
        }
    }
    
    /// <summary>
    /// Get Ulysses model info without loading it
    /// </summary>
    public static UlyssesModelInfo GetModelInfo()
    {
        var (exists, path, format) = CheckForUlyssesModel();
        
        if (!exists || path == null)
        {
            return new UlyssesModelInfo
            {
                IsAvailable = false,
                Message = "Ulysses model file not found. Using procedural generation."
            };
        }
        
        try
        {
            var assetManager = AssetManager.Instance;
            var fullPath = assetManager.GetAssetPath(path);
            var fileInfo = new FileInfo(fullPath);
            
            return new UlyssesModelInfo
            {
                IsAvailable = true,
                Path = path,
                Format = format,
                FileSizeKB = fileInfo.Length / 1024,
                Message = $"Ulysses model ready: {format} format, {fileInfo.Length / 1024}KB"
            };
        }
        catch (Exception ex)
        {
            return new UlyssesModelInfo
            {
                IsAvailable = false,
                Message = $"Error reading model info: {ex.Message}"
            };
        }
    }
    
    /// <summary>
    /// Log expected locations for the Ulysses model
    /// Helpful for users who want to add the file
    /// </summary>
    private static void LogExpectedLocations()
    {
        _logger.Info("UlyssesLoader", "Expected Ulysses model locations (in order of preference):");
        _logger.Info("UlyssesLoader", "  1. Assets/Models/ships/Ulysses/source/ulysses.obj (Wavefront OBJ)");
        _logger.Info("UlyssesLoader", "  2. Assets/Models/ships/Ulysses/source/ulysses.fbx (Autodesk FBX)");
        _logger.Info("UlyssesLoader", "  3. Assets/Models/ships/Ulysses/source/ulysses.gltf (glTF)");
        _logger.Info("UlyssesLoader", "  4. Assets/Models/ships/Ulysses/source/ulysses.glb (glTF Binary)");
        _logger.Info("UlyssesLoader", "  5. Assets/Models/ships/hulls/ulysses.obj");
        _logger.Info("UlyssesLoader", "");
        _logger.Info("UlyssesLoader", "To use a custom Ulysses model:");
        _logger.Info("UlyssesLoader", "  1. Export your model from Blender to OBJ, FBX, or glTF format");
        _logger.Info("UlyssesLoader", "  2. Place the exported file in Assets/Models/ships/Ulysses/source/");
        _logger.Info("UlyssesLoader", "  3. Name it 'ulysses' with appropriate extension (.obj, .fbx, .gltf, .glb)");
        _logger.Info("UlyssesLoader", "  4. Restart the game to load the model");
        _logger.Info("UlyssesLoader", "");
        _logger.Info("UlyssesLoader", "Note: .blend files are not supported directly. Please export from Blender first.");
        _logger.Info("UlyssesLoader", "Supported formats: .obj .fbx .gltf .glb .dae and 40+ more via Assimp");
    }
    
    /// <summary>
    /// Create a test/example Ulysses.blend file info document
    /// </summary>
    public static string GetUlyssesModelGuide()
    {
        return @"# Ulysses Model File Guide

## Where to Place the File

Place your Ulysses model in one of these locations:
- **Preferred**: `Assets/Models/ships/Ulysses/source/ulysses.obj`
- Alternative: `Assets/Models/ships/Ulysses/ulysses.fbx`
- Alternative: `Assets/Models/ships/hulls/ulysses.gltf`

## Supported Formats

The game supports multiple 3D model formats via Assimp:
1. **.obj** (Wavefront) - Simple, widely supported
2. **.fbx** (Autodesk) - Good for animations
3. **.gltf/.glb** - Modern, efficient
4. **.dae** (Collada) - Open standard
5. Many others via Assimp library

**Note**: .blend (Blender) files are NOT supported directly.
Please export your model from Blender to one of the above formats first.

## Blender Export Instructions

To export from Blender:
- **OBJ**: File > Export > Wavefront (.obj) — Triangulate Faces, Write Normals, Include UVs
- **FBX**: File > Export > FBX (.fbx) — Apply Scalings, Include Mesh/Normals/UVs
- **GLTF**: File > Export > glTF 2.0 (.glb/.gltf) — Binary format, Include Normals/Textures

## Model Specifications

Your Ulysses model should have:
- **Scale**: 1 unit = 1 meter
- **Length**: ~15-20 meters (corvette-class)
- **Origin**: Centered at (0,0,0)
- **Forward**: Positive Z-axis
- **Up**: Positive Y-axis

## Recommended Features

- Forward cockpit section
- Mid-section hull
- Rear engine section
- 2-4 weapon hardpoint locations
- Visible thruster nozzles
- Landing gear (optional)
- Interior space (if doing interiors)

## No Model? No Problem!

If no custom model is found, the game will:
- Generate a procedural Ulysses using modular parts
- Still provide full functionality
- Allow all customization options

The model is purely visual - all stats and equipment are configured separately.
";
    }
}

/// <summary>
/// Information about the Ulysses model file
/// </summary>
public class UlyssesModelInfo
{
    public bool IsAvailable { get; set; }
    public string? Path { get; set; }
    public string? Format { get; set; }
    public long FileSizeKB { get; set; }
    public string Message { get; set; } = "";
    
    public override string ToString()
    {
        if (!IsAvailable)
            return Message;
        
        return $"Ulysses Model: {Format} format\nPath: {Path}\nSize: {FileSizeKB}KB";
    }
}
