namespace AvorionLike.Core.Config;

/// <summary>
/// Debug configuration for voxel rendering and world generation diagnostics
/// </summary>
public static class DebugConfig
{
    /// <summary>
    /// Enable two-sided rendering (disable backface culling) for voxel meshes
    /// Renders both front and back faces by disabling OpenGL backface culling
    /// Helps diagnose face culling issues while preserving hollow structure visibility
    /// Toggle with F7
    /// Default is TRUE to prevent hollow-looking blocks from incorrect face winding
    /// </summary>
    public static bool TwoSidedRendering { get; set; } = true;

    /// <summary>
    /// Bypass frustum and occlusion culling - render all chunks regardless of visibility
    /// Helps diagnose culling issues
    /// Toggle with F8
    /// </summary>
    public static bool BypassCulling { get; set; } = false;

    /// <summary>
    /// Show wireframe AABB visualization for all loaded chunks
    /// Helps diagnose bounding box issues
    /// Toggle with F11
    /// </summary>
    public static bool ShowAABBs { get; set; } = false;

    /// <summary>
    /// Display generation task and result counts on-screen
    /// Helps diagnose world generation issues
    /// Toggle with F12
    /// </summary>
    public static bool ShowGenStats { get; set; } = false;

    /// <summary>
    /// Enable verbose logging for world generation
    /// </summary>
    public static bool VerboseWorldGenLogging { get; set; } = true;
}
