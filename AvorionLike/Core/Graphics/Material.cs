using Silk.NET.OpenGL;
using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Material properties for rendering
/// Supports both color-based and texture-based materials
/// 
/// Enhanced Material Colors with Vibrant Appearance and Shiny Finish:
/// - Iron: Polished Steel Grey (0.72, 0.72, 0.75) - High Metallic, Mirror-like
/// - Titanium: Brilliant Silver-Blue (0.82, 0.87, 0.95) - Very High Metallic, Highly Reflective
/// - Naonite: Vivid Emerald Green (0.15, 0.92, 0.35) with Bright Green Glow - Metallic Sheen
/// - Trinium: Brilliant Sapphire Blue (0.25, 0.65, 1.0) with Cyan Glow - Metallic with Glow
/// - Xanion: Brilliant Gold (1.0, 0.82, 0.15) with Golden Glow - Mirror Gold Finish
/// - Ogonite: Fiery Orange-Red (1.0, 0.4, 0.15) with Fire Glow - Metallic with Intense Glow
/// - Avorion: Royal Purple (0.85, 0.2, 1.0) with Purple Aura - Mirror Finish with Intense Glow
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
    /// Enhanced for vibrant colors and shiny, mirror-like finishes
    /// </summary>
    public static Material FromMaterialType(string materialType)
    {
        return materialType.ToLower() switch
        {
            "iron" => new Material
            {
                Name = "Iron",
                BaseColor = new Vector3(0.72f, 0.72f, 0.75f), // Polished steel grey
                Metallic = 0.95f,  // Highly metallic for shiny look
                Roughness = 0.15f  // Low roughness for mirror-like finish
            },
            "titanium" => new Material
            {
                Name = "Titanium",
                BaseColor = new Vector3(0.82f, 0.87f, 0.95f), // Brilliant silver-blue
                Metallic = 0.98f,  // Near-mirror metallic
                Roughness = 0.08f  // Very low roughness for highly reflective surface
            },
            "naonite" => new Material
            {
                Name = "Naonite",
                BaseColor = new Vector3(0.15f, 0.92f, 0.35f), // Vivid emerald green
                Metallic = 0.75f,
                Roughness = 0.2f,
                EmissiveColor = new Vector3(0.1f, 0.6f, 0.2f), // Brighter green glow
                EmissiveStrength = 0.5f
            },
            "trinium" => new Material
            {
                Name = "Trinium",
                BaseColor = new Vector3(0.25f, 0.65f, 1.0f), // Brilliant sapphire blue
                Metallic = 0.85f,
                Roughness = 0.15f,
                EmissiveColor = new Vector3(0.2f, 0.5f, 0.8f), // Cyan glow
                EmissiveStrength = 0.45f
            },
            "xanion" => new Material
            {
                Name = "Xanion",
                BaseColor = new Vector3(1.0f, 0.82f, 0.15f), // Brilliant gold
                Metallic = 0.99f,  // Mirror gold finish
                Roughness = 0.05f, // Near-perfect mirror
                EmissiveColor = new Vector3(0.6f, 0.5f, 0.1f), // Golden glow
                EmissiveStrength = 0.55f
            },
            "ogonite" => new Material
            {
                Name = "Ogonite",
                BaseColor = new Vector3(1.0f, 0.4f, 0.15f), // Fiery orange-red
                Metallic = 0.88f,
                Roughness = 0.12f,
                EmissiveColor = new Vector3(0.7f, 0.25f, 0.1f), // Fire glow
                EmissiveStrength = 0.6f
            },
            "avorion" => new Material
            {
                Name = "Avorion",
                BaseColor = new Vector3(0.85f, 0.2f, 1.0f), // Royal purple
                Metallic = 0.97f,  // Mirror finish
                Roughness = 0.05f, // Near-perfect mirror
                EmissiveColor = new Vector3(0.5f, 0.15f, 0.7f), // Purple aura
                EmissiveStrength = 0.8f // Intense glow for the rare material
            },
            _ => new Material
            {
                Name = "Default",
                BaseColor = new Vector3(0.6f, 0.6f, 0.6f),
                Metallic = 0.7f,
                Roughness = 0.3f
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
