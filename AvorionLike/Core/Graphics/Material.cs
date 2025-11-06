using Silk.NET.OpenGL;
using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Material properties for rendering
/// Supports both color-based and texture-based materials
/// </summary>
public class Material
{
    public string Name { get; set; } = "Default";
    
    // Color properties
    public Vector3 BaseColor { get; set; } = new Vector3(1.0f, 1.0f, 1.0f);
    public Vector3 EmissiveColor { get; set; } = Vector3.Zero;
    public float Metallic { get; set; } = 0.0f;
    public float Roughness { get; set; } = 0.5f;
    public float EmissiveStrength { get; set; } = 0.0f;
    
    // Texture properties
    public uint? AlbedoTexture { get; set; }
    public uint? NormalTexture { get; set; }
    public uint? MetallicTexture { get; set; }
    public uint? RoughnessTexture { get; set; }
    public uint? EmissiveTexture { get; set; }
    
    public bool UseTextures => AlbedoTexture.HasValue;
    
    /// <summary>
    /// Get material color from hex RGB value
    /// </summary>
    public static Vector3 ColorFromRGB(uint rgb)
    {
        float r = ((rgb >> 16) & 0xFF) / 255.0f;
        float g = ((rgb >> 8) & 0xFF) / 255.0f;
        float b = (rgb & 0xFF) / 255.0f;
        return new Vector3(r, g, b);
    }
    
    /// <summary>
    /// Create material from voxel material type
    /// </summary>
    public static Material FromMaterialType(string materialType)
    {
        return materialType.ToLower() switch
        {
            "iron" => new Material
            {
                Name = "Iron",
                BaseColor = new Vector3(0.65f, 0.65f, 0.65f),
                Metallic = 0.8f,
                Roughness = 0.4f
            },
            "titanium" => new Material
            {
                Name = "Titanium",
                BaseColor = new Vector3(0.75f, 0.8f, 0.85f),
                Metallic = 0.9f,
                Roughness = 0.3f
            },
            "naonite" => new Material
            {
                Name = "Naonite",
                BaseColor = new Vector3(0.2f, 0.8f, 0.3f),
                Metallic = 0.5f,
                Roughness = 0.4f,
                EmissiveColor = new Vector3(0.1f, 0.4f, 0.15f),
                EmissiveStrength = 0.3f
            },
            "trinium" => new Material
            {
                Name = "Trinium",
                BaseColor = new Vector3(0.3f, 0.6f, 0.9f),
                Metallic = 0.6f,
                Roughness = 0.35f,
                EmissiveColor = new Vector3(0.15f, 0.3f, 0.5f),
                EmissiveStrength = 0.2f
            },
            "xanion" => new Material
            {
                Name = "Xanion",
                BaseColor = new Vector3(0.9f, 0.75f, 0.2f),
                Metallic = 0.95f,
                Roughness = 0.25f,
                EmissiveColor = new Vector3(0.5f, 0.4f, 0.1f),
                EmissiveStrength = 0.4f
            },
            "ogonite" => new Material
            {
                Name = "Ogonite",
                BaseColor = new Vector3(0.9f, 0.35f, 0.2f),
                Metallic = 0.7f,
                Roughness = 0.3f,
                EmissiveColor = new Vector3(0.5f, 0.2f, 0.1f),
                EmissiveStrength = 0.35f
            },
            "avorion" => new Material
            {
                Name = "Avorion",
                BaseColor = new Vector3(0.75f, 0.25f, 0.9f),
                Metallic = 0.85f,
                Roughness = 0.2f,
                EmissiveColor = new Vector3(0.4f, 0.15f, 0.5f),
                EmissiveStrength = 0.6f
            },
            _ => new Material
            {
                Name = "Default",
                BaseColor = new Vector3(0.5f, 0.5f, 0.5f),
                Metallic = 0.5f,
                Roughness = 0.5f
            }
        };
    }
}

/// <summary>
/// Manages materials and their GPU resources
/// </summary>
public class MaterialManager : IDisposable
{
    private readonly GL _gl;
    private readonly Dictionary<string, Material> _materials = new();
    private bool _disposed = false;

    public MaterialManager(GL gl)
    {
        _gl = gl;
        InitializeDefaultMaterials();
    }

    private void InitializeDefaultMaterials()
    {
        // Create default materials for each voxel type
        var materialTypes = new[] { "Iron", "Titanium", "Naonite", "Trinium", "Xanion", "Ogonite", "Avorion" };
        
        foreach (var type in materialTypes)
        {
            _materials[type] = Material.FromMaterialType(type);
        }
    }

    public Material GetMaterial(string name)
    {
        if (_materials.TryGetValue(name, out var material))
            return material;
        
        // Create on demand if not found
        var newMaterial = Material.FromMaterialType(name);
        _materials[name] = newMaterial;
        return newMaterial;
    }

    public void AddMaterial(string name, Material material)
    {
        _materials[name] = material;
    }

    public void Dispose()
    {
        if (!_disposed)
        {
            // Clean up any texture resources if needed
            _disposed = true;
        }
        GC.SuppressFinalize(this);
    }
}
