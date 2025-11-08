using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Material type determines which textures and properties to use
/// </summary>
public enum MaterialType
{
    // Structural materials
    Hull,
    Armor,
    
    // Natural materials
    Rock,
    Ice,
    Grass,
    Sand,
    Snow,
    Water,
    Lava,
    
    // Industrial materials
    Metal,
    Titanium,
    Naonite,
    Crystal,
    
    // Environmental
    GasCloud,
    Nebula,
    Plasma,
    
    // Special
    Energy,
    Shield,
    Glow
}

/// <summary>
/// Texture material definition with procedural properties
/// </summary>
public class TextureMaterial
{
    public MaterialType Type { get; set; }
    public string Name { get; set; } = "";
    
    // Base color (RGB)
    public Vector3 BaseColor { get; set; } = Vector3.One;
    public Vector3 SecondaryColor { get; set; } = Vector3.One;
    
    // Physical properties
    public float Roughness { get; set; } = 0.5f; // 0 = smooth/shiny, 1 = rough/matte
    public float Metallic { get; set; } = 0.0f; // 0 = dielectric, 1 = metallic
    public float Emissive { get; set; } = 0.0f; // 0 = no glow, 1 = full glow
    
    // Procedural properties
    public float NoiseScale { get; set; } = 1.0f; // Scale of noise texture
    public float NoiseStrength { get; set; } = 0.1f; // How much noise affects color
    public float BumpStrength { get; set; } = 0.5f; // Normal map intensity
    
    // Pattern properties
    public TexturePattern Pattern { get; set; } = TexturePattern.Uniform;
    public float PatternScale { get; set; } = 1.0f;
    
    // Transparency
    public float Opacity { get; set; } = 1.0f; // 0 = transparent, 1 = opaque
    
    // Animation (for gases, energy, etc.)
    public bool Animated { get; set; } = false;
    public float AnimationSpeed { get; set; } = 1.0f;
}

/// <summary>
/// Texture pattern types for procedural generation
/// </summary>
public enum TexturePattern
{
    Uniform,      // Solid color with noise variation
    Striped,      // Horizontal or vertical stripes
    Banded,       // Concentric bands (for gas giants)
    Paneled,      // Hull panels with rivets
    Hexagonal,    // Hex pattern
    Cracked,      // Cracked/fractured
    Crystalline,  // Crystal structure
    Swirled,      // Turbulent swirl (gas clouds)
    Spotted,      // Random spots/patches
    Weathered     // Wear and tear
}

/// <summary>
/// Library of predefined texture materials
/// </summary>
public static class MaterialLibrary
{
    private static Dictionary<MaterialType, TextureMaterial> _materials = new();
    
    static MaterialLibrary()
    {
        InitializeMaterials();
    }
    
    private static void InitializeMaterials()
    {
        // Structural materials
        _materials[MaterialType.Hull] = new TextureMaterial
        {
            Type = MaterialType.Hull,
            Name = "Hull Plating",
            BaseColor = new Vector3(0.5f, 0.5f, 0.5f), // Light gray
            SecondaryColor = new Vector3(0.3f, 0.3f, 0.3f), // Dark gray
            Roughness = 0.6f,
            Metallic = 0.8f,
            NoiseScale = 2.0f,
            NoiseStrength = 0.15f,
            BumpStrength = 0.3f,
            Pattern = TexturePattern.Paneled,
            PatternScale = 4.0f
        };
        
        _materials[MaterialType.Armor] = new TextureMaterial
        {
            Type = MaterialType.Armor,
            Name = "Armor Plating",
            BaseColor = new Vector3(0.3f, 0.3f, 0.35f), // Dark blue-gray
            SecondaryColor = new Vector3(0.2f, 0.2f, 0.25f),
            Roughness = 0.4f,
            Metallic = 0.9f,
            NoiseScale = 1.5f,
            NoiseStrength = 0.1f,
            BumpStrength = 0.5f,
            Pattern = TexturePattern.Paneled,
            PatternScale = 3.0f
        };
        
        // Natural materials
        _materials[MaterialType.Rock] = new TextureMaterial
        {
            Type = MaterialType.Rock,
            Name = "Asteroid Rock",
            BaseColor = new Vector3(0.4f, 0.4f, 0.4f), // Gray
            SecondaryColor = new Vector3(0.3f, 0.25f, 0.2f), // Brown tint
            Roughness = 0.9f,
            Metallic = 0.0f,
            NoiseScale = 3.0f,
            NoiseStrength = 0.3f,
            BumpStrength = 0.8f,
            Pattern = TexturePattern.Cracked,
            PatternScale = 2.0f
        };
        
        _materials[MaterialType.Ice] = new TextureMaterial
        {
            Type = MaterialType.Ice,
            Name = "Ice",
            BaseColor = new Vector3(0.8f, 0.9f, 1.0f), // Light blue-white
            SecondaryColor = new Vector3(0.6f, 0.7f, 0.9f),
            Roughness = 0.2f,
            Metallic = 0.0f,
            NoiseScale = 4.0f,
            NoiseStrength = 0.15f,
            BumpStrength = 0.4f,
            Pattern = TexturePattern.Cracked,
            PatternScale = 3.0f,
            Opacity = 0.9f
        };
        
        _materials[MaterialType.Grass] = new TextureMaterial
        {
            Type = MaterialType.Grass,
            Name = "Grass",
            BaseColor = new Vector3(0.2f, 0.6f, 0.2f), // Green
            SecondaryColor = new Vector3(0.3f, 0.5f, 0.2f), // Yellow-green
            Roughness = 0.8f,
            Metallic = 0.0f,
            NoiseScale = 8.0f,
            NoiseStrength = 0.4f,
            BumpStrength = 0.6f,
            Pattern = TexturePattern.Spotted,
            PatternScale = 5.0f
        };
        
        _materials[MaterialType.Sand] = new TextureMaterial
        {
            Type = MaterialType.Sand,
            Name = "Sand",
            BaseColor = new Vector3(0.9f, 0.8f, 0.5f), // Sandy yellow
            SecondaryColor = new Vector3(0.8f, 0.6f, 0.4f), // Orange
            Roughness = 0.9f,
            Metallic = 0.0f,
            NoiseScale = 10.0f,
            NoiseStrength = 0.2f,
            BumpStrength = 0.3f,
            Pattern = TexturePattern.Uniform,
            PatternScale = 1.0f
        };
        
        _materials[MaterialType.Snow] = new TextureMaterial
        {
            Type = MaterialType.Snow,
            Name = "Snow",
            BaseColor = new Vector3(0.95f, 0.95f, 1.0f), // White with blue tint
            SecondaryColor = new Vector3(0.9f, 0.9f, 0.95f),
            Roughness = 0.7f,
            Metallic = 0.0f,
            NoiseScale = 6.0f,
            NoiseStrength = 0.1f,
            BumpStrength = 0.4f,
            Pattern = TexturePattern.Spotted,
            PatternScale = 4.0f
        };
        
        _materials[MaterialType.Water] = new TextureMaterial
        {
            Type = MaterialType.Water,
            Name = "Water",
            BaseColor = new Vector3(0.1f, 0.3f, 0.6f), // Deep blue
            SecondaryColor = new Vector3(0.2f, 0.5f, 0.8f), // Light blue
            Roughness = 0.1f,
            Metallic = 0.0f,
            NoiseScale = 5.0f,
            NoiseStrength = 0.3f,
            BumpStrength = 0.2f,
            Pattern = TexturePattern.Swirled,
            PatternScale = 2.0f,
            Opacity = 0.7f,
            Animated = true,
            AnimationSpeed = 0.5f
        };
        
        _materials[MaterialType.Lava] = new TextureMaterial
        {
            Type = MaterialType.Lava,
            Name = "Lava",
            BaseColor = new Vector3(1.0f, 0.3f, 0.0f), // Bright orange-red
            SecondaryColor = new Vector3(0.8f, 0.1f, 0.0f), // Dark red
            Roughness = 0.8f,
            Metallic = 0.0f,
            Emissive = 0.9f,
            NoiseScale = 3.0f,
            NoiseStrength = 0.5f,
            BumpStrength = 0.7f,
            Pattern = TexturePattern.Cracked,
            PatternScale = 2.0f,
            Animated = true,
            AnimationSpeed = 0.3f
        };
        
        // Industrial materials
        _materials[MaterialType.Metal] = new TextureMaterial
        {
            Type = MaterialType.Metal,
            Name = "Metal",
            BaseColor = new Vector3(0.7f, 0.7f, 0.7f), // Silver-gray
            SecondaryColor = new Vector3(0.5f, 0.5f, 0.5f),
            Roughness = 0.5f,
            Metallic = 1.0f,
            NoiseScale = 2.0f,
            NoiseStrength = 0.1f,
            BumpStrength = 0.3f,
            Pattern = TexturePattern.Weathered,
            PatternScale = 3.0f
        };
        
        _materials[MaterialType.Titanium] = new TextureMaterial
        {
            Type = MaterialType.Titanium,
            Name = "Titanium",
            BaseColor = new Vector3(0.5f, 0.6f, 0.8f), // Blue-gray
            SecondaryColor = new Vector3(0.4f, 0.5f, 0.7f),
            Roughness = 0.4f,
            Metallic = 1.0f,
            NoiseScale = 1.5f,
            NoiseStrength = 0.08f,
            BumpStrength = 0.2f,
            Pattern = TexturePattern.Paneled,
            PatternScale = 4.0f
        };
        
        _materials[MaterialType.Naonite] = new TextureMaterial
        {
            Type = MaterialType.Naonite,
            Name = "Naonite",
            BaseColor = new Vector3(0.2f, 0.8f, 0.3f), // Bright green
            SecondaryColor = new Vector3(0.1f, 0.6f, 0.2f),
            Roughness = 0.3f,
            Metallic = 0.9f,
            Emissive = 0.1f,
            NoiseScale = 2.0f,
            NoiseStrength = 0.15f,
            BumpStrength = 0.3f,
            Pattern = TexturePattern.Hexagonal,
            PatternScale = 3.0f
        };
        
        _materials[MaterialType.Crystal] = new TextureMaterial
        {
            Type = MaterialType.Crystal,
            Name = "Crystal",
            BaseColor = new Vector3(0.6f, 0.8f, 1.0f), // Light blue
            SecondaryColor = new Vector3(0.8f, 0.9f, 1.0f),
            Roughness = 0.1f,
            Metallic = 0.0f,
            Emissive = 0.3f,
            NoiseScale = 4.0f,
            NoiseStrength = 0.2f,
            BumpStrength = 0.8f,
            Pattern = TexturePattern.Crystalline,
            PatternScale = 2.0f,
            Opacity = 0.8f
        };
        
        // Environmental
        _materials[MaterialType.GasCloud] = new TextureMaterial
        {
            Type = MaterialType.GasCloud,
            Name = "Gas Cloud",
            BaseColor = new Vector3(0.7f, 0.5f, 0.9f), // Purple
            SecondaryColor = new Vector3(0.5f, 0.3f, 0.7f),
            Roughness = 1.0f,
            Metallic = 0.0f,
            Emissive = 0.5f,
            NoiseScale = 1.0f,
            NoiseStrength = 0.6f,
            Pattern = TexturePattern.Swirled,
            PatternScale = 1.0f,
            Opacity = 0.3f,
            Animated = true,
            AnimationSpeed = 0.2f
        };
        
        _materials[MaterialType.Nebula] = new TextureMaterial
        {
            Type = MaterialType.Nebula,
            Name = "Nebula",
            BaseColor = new Vector3(1.0f, 0.4f, 0.7f), // Pink
            SecondaryColor = new Vector3(0.4f, 0.6f, 1.0f), // Blue
            Roughness = 1.0f,
            Metallic = 0.0f,
            Emissive = 0.7f,
            NoiseScale = 0.5f,
            NoiseStrength = 0.8f,
            Pattern = TexturePattern.Swirled,
            PatternScale = 0.5f,
            Opacity = 0.2f,
            Animated = true,
            AnimationSpeed = 0.1f
        };
        
        _materials[MaterialType.Plasma] = new TextureMaterial
        {
            Type = MaterialType.Plasma,
            Name = "Plasma",
            BaseColor = new Vector3(0.0f, 1.0f, 1.0f), // Cyan
            SecondaryColor = new Vector3(0.5f, 0.5f, 1.0f), // Light blue
            Roughness = 0.5f,
            Metallic = 0.0f,
            Emissive = 1.0f,
            NoiseScale = 2.0f,
            NoiseStrength = 0.7f,
            Pattern = TexturePattern.Swirled,
            PatternScale = 1.5f,
            Opacity = 0.6f,
            Animated = true,
            AnimationSpeed = 1.0f
        };
        
        // Special
        _materials[MaterialType.Energy] = new TextureMaterial
        {
            Type = MaterialType.Energy,
            Name = "Energy",
            BaseColor = new Vector3(0.0f, 0.8f, 1.0f), // Bright cyan
            SecondaryColor = new Vector3(0.5f, 1.0f, 1.0f),
            Roughness = 0.0f,
            Metallic = 0.0f,
            Emissive = 1.0f,
            NoiseScale = 8.0f,
            NoiseStrength = 0.5f,
            Pattern = TexturePattern.Striped,
            PatternScale = 6.0f,
            Animated = true,
            AnimationSpeed = 2.0f
        };
        
        _materials[MaterialType.Shield] = new TextureMaterial
        {
            Type = MaterialType.Shield,
            Name = "Shield",
            BaseColor = new Vector3(0.3f, 0.6f, 1.0f), // Blue
            SecondaryColor = new Vector3(0.5f, 0.8f, 1.0f),
            Roughness = 0.2f,
            Metallic = 0.0f,
            Emissive = 0.4f,
            NoiseScale = 5.0f,
            NoiseStrength = 0.3f,
            Pattern = TexturePattern.Hexagonal,
            PatternScale = 8.0f,
            Opacity = 0.4f,
            Animated = true,
            AnimationSpeed = 0.5f
        };
        
        _materials[MaterialType.Glow] = new TextureMaterial
        {
            Type = MaterialType.Glow,
            Name = "Glow",
            BaseColor = new Vector3(1.0f, 1.0f, 0.5f), // Yellow-white
            SecondaryColor = new Vector3(1.0f, 0.8f, 0.3f),
            Roughness = 0.0f,
            Metallic = 0.0f,
            Emissive = 1.0f,
            NoiseScale = 4.0f,
            NoiseStrength = 0.2f,
            Pattern = TexturePattern.Uniform,
            Animated = true,
            AnimationSpeed = 1.5f
        };
    }
    
    /// <summary>
    /// Get a material by type
    /// </summary>
    public static TextureMaterial GetMaterial(MaterialType type)
    {
        return _materials.GetValueOrDefault(type, _materials[MaterialType.Metal]);
    }
    
    /// <summary>
    /// Get all available materials
    /// </summary>
    public static IReadOnlyDictionary<MaterialType, TextureMaterial> GetAllMaterials()
    {
        return _materials;
    }
}
