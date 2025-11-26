using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Rendering mode for the voxel renderer
/// </summary>
public enum RenderingMode
{
    /// <summary>
    /// Physically Based Rendering - realistic materials with metallic/roughness workflow
    /// Best for games with a realistic, professional look
    /// </summary>
    PBR,
    
    /// <summary>
    /// Non-Photorealistic Rendering - stylized cel-shading with outlines
    /// Best for games with a unique, stylized, or less serious tone (comic book/anime style)
    /// </summary>
    NPR,
    
    /// <summary>
    /// Hybrid mode - PBR with NPR edge detection for better block visibility
    /// Recommended for voxel-based games to highlight modular components
    /// </summary>
    Hybrid
}

/// <summary>
/// Configuration for the rendering system
/// Addresses visual issues on blocks by providing flexible rendering options
/// </summary>
public class RenderingConfiguration
{
    private static RenderingConfiguration? _instance;
    public static RenderingConfiguration Instance => _instance ??= new RenderingConfiguration();
    
    /// <summary>
    /// Current rendering mode (PBR, NPR, or Hybrid)
    /// </summary>
    public RenderingMode Mode { get; set; } = RenderingMode.Hybrid;
    
    // === NPR Settings ===
    
    /// <summary>
    /// Enable edge detection for block outlines (NPR/Hybrid mode)
    /// </summary>
    public bool EnableEdgeDetection { get; set; } = true;
    
    /// <summary>
    /// Edge thickness for outlines (1.0 = standard, 2.0 = thick)
    /// </summary>
    public float EdgeThickness { get; set; } = 1.2f;
    
    /// <summary>
    /// Edge color for outlines (default: dark grey for subtle edges)
    /// </summary>
    public Vector3 EdgeColor { get; set; } = new Vector3(0.1f, 0.1f, 0.15f);
    
    /// <summary>
    /// Enable cel-shading with discrete light bands (NPR mode)
    /// </summary>
    public bool EnableCelShading { get; set; } = false;
    
    /// <summary>
    /// Number of shading bands for cel-shading (3-8)
    /// </summary>
    public int CelShadingBands { get; set; } = 4;
    
    // === PBR Settings ===
    
    /// <summary>
    /// Enable ambient occlusion between adjacent blocks
    /// This helps define block boundaries and adds depth
    /// </summary>
    public bool EnableAmbientOcclusion { get; set; } = true;
    
    /// <summary>
    /// Strength of ambient occlusion effect (0.0 - 1.0)
    /// </summary>
    public float AmbientOcclusionStrength { get; set; } = 0.35f;
    
    /// <summary>
    /// Enable per-material properties (different blocks look different)
    /// </summary>
    public bool EnablePerMaterialProperties { get; set; } = true;
    
    /// <summary>
    /// Enable procedural surface details (panel lines, vents, etc.)
    /// </summary>
    public bool EnableProceduralDetails { get; set; } = true;
    
    /// <summary>
    /// Strength of procedural detail overlay (0.0 - 1.0)
    /// </summary>
    public float ProceduralDetailStrength { get; set; } = 0.5f;
    
    // === Block-Specific Settings ===
    
    /// <summary>
    /// Enable glow effects on functional blocks (engines, shields, generators)
    /// </summary>
    public bool EnableBlockGlow { get; set; } = true;
    
    /// <summary>
    /// Intensity of block glow effects (0.5 - 2.0)
    /// </summary>
    public float BlockGlowIntensity { get; set; } = 1.0f;
    
    /// <summary>
    /// Enable block type coloring (different blocks get tinted by type)
    /// </summary>
    public bool EnableBlockTypeColoring { get; set; } = true;
    
    // === Visual Quality Settings ===
    
    /// <summary>
    /// Enable rim lighting for dramatic edge highlights
    /// </summary>
    public bool EnableRimLighting { get; set; } = true;
    
    /// <summary>
    /// Strength of rim lighting effect
    /// </summary>
    public float RimLightingStrength { get; set; } = 0.4f;
    
    /// <summary>
    /// Enable environment reflections on metallic surfaces
    /// </summary>
    public bool EnableEnvironmentReflections { get; set; } = true;
    
    /// <summary>
    /// Apply a preset configuration
    /// </summary>
    public void ApplyPreset(RenderingPreset preset)
    {
        switch (preset)
        {
            case RenderingPreset.RealisticPBR:
                Mode = RenderingMode.PBR;
                EnableEdgeDetection = false;
                EnableCelShading = false;
                EnableAmbientOcclusion = true;
                AmbientOcclusionStrength = 0.4f;
                EnablePerMaterialProperties = true;
                EnableProceduralDetails = true;
                ProceduralDetailStrength = 0.6f;
                EnableBlockGlow = true;
                BlockGlowIntensity = 0.8f;
                EnableRimLighting = true;
                RimLightingStrength = 0.3f;
                EnableEnvironmentReflections = true;
                break;
                
            case RenderingPreset.StylizedNPR:
                Mode = RenderingMode.NPR;
                EnableEdgeDetection = true;
                EdgeThickness = 1.5f;
                EnableCelShading = true;
                CelShadingBands = 4;
                EnableAmbientOcclusion = false;
                EnablePerMaterialProperties = true;
                EnableProceduralDetails = false;
                EnableBlockGlow = true;
                BlockGlowIntensity = 1.2f;
                EnableRimLighting = true;
                RimLightingStrength = 0.5f;
                EnableEnvironmentReflections = false;
                break;
                
            case RenderingPreset.HybridBalanced:
                Mode = RenderingMode.Hybrid;
                EnableEdgeDetection = true;
                EdgeThickness = 1.0f;
                EdgeColor = new Vector3(0.15f, 0.15f, 0.2f);
                EnableCelShading = false;
                EnableAmbientOcclusion = true;
                AmbientOcclusionStrength = 0.35f;
                EnablePerMaterialProperties = true;
                EnableProceduralDetails = true;
                ProceduralDetailStrength = 0.4f;
                EnableBlockGlow = true;
                BlockGlowIntensity = 1.0f;
                EnableRimLighting = true;
                RimLightingStrength = 0.4f;
                EnableEnvironmentReflections = true;
                break;
                
            case RenderingPreset.Performance:
                Mode = RenderingMode.PBR;
                EnableEdgeDetection = false;
                EnableCelShading = false;
                EnableAmbientOcclusion = false;
                EnablePerMaterialProperties = false;
                EnableProceduralDetails = false;
                EnableBlockGlow = false;
                EnableRimLighting = false;
                EnableEnvironmentReflections = false;
                break;
        }
    }
}

/// <summary>
/// Preset configurations for different visual styles
/// </summary>
public enum RenderingPreset
{
    /// <summary>
    /// Full PBR with all realistic effects enabled
    /// </summary>
    RealisticPBR,
    
    /// <summary>
    /// Stylized NPR with cel-shading and outlines
    /// </summary>
    StylizedNPR,
    
    /// <summary>
    /// Hybrid mode balancing PBR and edge detection (Recommended)
    /// </summary>
    HybridBalanced,
    
    /// <summary>
    /// Minimal effects for performance
    /// </summary>
    Performance
}
