using Silk.NET.OpenGL;
using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Material properties for rendering
/// Supports both color-based and texture-based materials
/// 
/// Visual Reference: Dark industrial aesthetic inspired by reference concept art.
/// Ships use dark gunmetal hulls with weathered metallic finish and orange/amber emissive accents.
/// 
/// Material Colors (dark industrial base with emissive accents per tier):
/// - Iron: Dark Gunmetal (0.25, 0.24, 0.23) - Weathered industrial metal
/// - Titanium: Dark Steel Grey (0.35, 0.36, 0.38) - Slightly lighter industrial metal
/// - Naonite: Dark Hull (0.22, 0.26, 0.24) with Subtle Green Glow
/// - Trinium: Dark Hull (0.24, 0.27, 0.30) with Subtle Blue Glow
/// - Xanion: Dark Hull (0.28, 0.26, 0.22) with Warm Golden Glow
/// - Ogonite: Dark Hull (0.30, 0.24, 0.20) with Orange-Amber Glow (engine/weapon accents)
/// - Avorion: Dark Hull (0.28, 0.22, 0.30) with Purple Aura
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
    /// Dark industrial aesthetic: weathered gunmetal hulls with emissive accents per material tier
    /// See VISUAL_REFERENCE_GUIDE.md for target look
    /// </summary>
    public static Material FromMaterialType(string materialType)
    {
        return materialType.ToLower() switch
        {
            "iron" => new Material
            {
                Name = "Iron",
                BaseColor = new Vector3(0.25f, 0.24f, 0.23f), // Dark gunmetal grey
                Metallic = 0.88f,  // Clearly metallic
                Roughness = 0.50f  // Weathered industrial finish
            },
            "titanium" => new Material
            {
                Name = "Titanium",
                BaseColor = new Vector3(0.35f, 0.36f, 0.38f), // Dark steel grey
                Metallic = 0.90f,
                Roughness = 0.40f  // Slightly smoother than iron
            },
            "naonite" => new Material
            {
                Name = "Naonite",
                BaseColor = new Vector3(0.22f, 0.26f, 0.24f), // Dark hull with green tint
                Metallic = 0.85f,
                Roughness = 0.45f,
                EmissiveColor = new Vector3(0.08f, 0.35f, 0.15f), // Subtle green glow
                EmissiveStrength = 0.3f
            },
            "trinium" => new Material
            {
                Name = "Trinium",
                BaseColor = new Vector3(0.24f, 0.27f, 0.30f), // Dark hull with blue tint
                Metallic = 0.87f,
                Roughness = 0.42f,
                EmissiveColor = new Vector3(0.12f, 0.30f, 0.50f), // Subtle blue glow
                EmissiveStrength = 0.3f
            },
            "xanion" => new Material
            {
                Name = "Xanion",
                BaseColor = new Vector3(0.28f, 0.26f, 0.22f), // Dark hull with warm tint
                Metallic = 0.90f,
                Roughness = 0.38f,
                EmissiveColor = new Vector3(0.50f, 0.38f, 0.08f), // Golden glow
                EmissiveStrength = 0.35f
            },
            "ogonite" => new Material
            {
                Name = "Ogonite",
                BaseColor = new Vector3(0.30f, 0.24f, 0.20f), // Dark hull with warm tint
                Metallic = 0.88f,
                Roughness = 0.40f,
                EmissiveColor = new Vector3(1.0f, 0.6f, 0.15f), // Orange-amber glow (engine/weapon accents)
                EmissiveStrength = 0.5f
            },
            "avorion" => new Material
            {
                Name = "Avorion",
                BaseColor = new Vector3(0.28f, 0.22f, 0.30f), // Dark hull with purple tint
                Metallic = 0.92f,
                Roughness = 0.35f,
                EmissiveColor = new Vector3(0.40f, 0.12f, 0.55f), // Purple aura
                EmissiveStrength = 0.5f
            },
            _ => new Material
            {
                Name = "Default",
                BaseColor = new Vector3(0.25f, 0.25f, 0.25f), // Dark neutral
                Metallic = 0.80f,
                Roughness = 0.50f
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
