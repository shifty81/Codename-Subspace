using System.Numerics;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Decal pattern types that can be applied to ship blocks
/// Inspired by the reference image (1234.PNG) showing hazard stripes and color patterns
/// </summary>
public enum DecalPattern
{
    None,              // No decal
    HazardStripes,     // Yellow/black diagonal warning stripes
    RacingStripes,     // Horizontal stripes along length
    FactionMarking,    // Custom faction emblem/color
    RedAccent,         // Red accent stripes
    CheckerPattern,    // Checkerboard pattern
    CamoPattern,       // Camouflage pattern
    GlowStripes,       // Glowing stripes (for engines)
    NumberMarking,     // Hull numbers/identification
    WeatheringMarks    // Battle damage/wear decals
}

/// <summary>
/// Represents a decorative decal that can be applied to a voxel block
/// Decals are visual-only and don't affect block functionality
/// Can be applied during ship building/editing
/// </summary>
public class BlockDecal
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public DecalPattern Pattern { get; set; } = DecalPattern.None;
    
    // Color properties
    public uint PrimaryColor { get; set; } = 0xFFFFFF;   // Primary decal color
    public uint SecondaryColor { get; set; } = 0x000000; // Secondary color (for patterns)
    public uint AccentColor { get; set; } = 0xFF0000;    // Accent color (optional)
    
    // Pattern properties
    public float Scale { get; set; } = 1.0f;              // Scale of the pattern
    public float Rotation { get; set; } = 0f;             // Rotation in degrees
    public Vector2 Offset { get; set; } = Vector2.Zero;   // Offset on block surface
    public float Opacity { get; set; } = 1.0f;            // Transparency (0-1)
    
    // Application properties
    public BlockFace TargetFace { get; set; } = BlockFace.All; // Which face(s) to apply to
    public bool ApplyToAllFaces { get; set; } = false;    // Apply to all faces or specific face
    
    /// <summary>
    /// Create a decal with default settings
    /// </summary>
    public BlockDecal()
    {
    }
    
    /// <summary>
    /// Create a decal with specified pattern and colors
    /// </summary>
    public BlockDecal(DecalPattern pattern, uint primaryColor, uint secondaryColor = 0x000000)
    {
        Pattern = pattern;
        PrimaryColor = primaryColor;
        SecondaryColor = secondaryColor;
    }
    
    /// <summary>
    /// Clone this decal
    /// </summary>
    public BlockDecal Clone()
    {
        return new BlockDecal
        {
            Pattern = Pattern,
            PrimaryColor = PrimaryColor,
            SecondaryColor = SecondaryColor,
            AccentColor = AccentColor,
            Scale = Scale,
            Rotation = Rotation,
            Offset = Offset,
            Opacity = Opacity,
            TargetFace = TargetFace,
            ApplyToAllFaces = ApplyToAllFaces
        };
    }
}

/// <summary>
/// Block faces where decals can be applied
/// </summary>
[Flags]
public enum BlockFace
{
    None = 0,
    Top = 1 << 0,      // +Y face
    Bottom = 1 << 1,   // -Y face
    Front = 1 << 2,    // +Z face
    Back = 1 << 3,     // -Z face
    Right = 1 << 4,    // +X face
    Left = 1 << 5,     // -X face
    All = Top | Bottom | Front | Back | Right | Left
}

/// <summary>
/// Library of predefined decals matching common spaceship aesthetics
/// Based on reference image (1234.PNG) showing industrial ships with hazard patterns
/// </summary>
public static class DecalLibrary
{
    /// <summary>
    /// Get a hazard stripe decal (yellow/black diagonal stripes like on 1234.PNG wings)
    /// </summary>
    public static BlockDecal HazardStripes()
    {
        return new BlockDecal
        {
            Pattern = DecalPattern.HazardStripes,
            PrimaryColor = 0xFFCC00,  // Bright yellow-orange
            SecondaryColor = 0x000000, // Black
            Scale = 1.0f,
            Rotation = 45f, // Diagonal stripes
            ApplyToAllFaces = false,
            TargetFace = BlockFace.Top | BlockFace.Right | BlockFace.Left // Top and sides
        };
    }
    
    /// <summary>
    /// Get a red accent stripe decal (like on 1234.PNG trailing edges)
    /// </summary>
    public static BlockDecal RedAccentStripe()
    {
        return new BlockDecal
        {
            Pattern = DecalPattern.RedAccent,
            PrimaryColor = 0xFF3333,  // Bright red
            SecondaryColor = 0x000000, // Black trim
            Scale = 0.8f,
            ApplyToAllFaces = false,
            TargetFace = BlockFace.Top | BlockFace.Back
        };
    }
    
    /// <summary>
    /// Get a racing stripe decal (runs along ship length)
    /// </summary>
    public static BlockDecal RacingStripes(uint color)
    {
        return new BlockDecal
        {
            Pattern = DecalPattern.RacingStripes,
            PrimaryColor = color,
            SecondaryColor = 0x000000,
            Scale = 1.0f,
            ApplyToAllFaces = false,
            TargetFace = BlockFace.Top
        };
    }
    
    /// <summary>
    /// Get a faction marking decal
    /// </summary>
    public static BlockDecal FactionMarking(uint factionColor)
    {
        return new BlockDecal
        {
            Pattern = DecalPattern.FactionMarking,
            PrimaryColor = factionColor,
            SecondaryColor = 0xFFFFFF,
            Scale = 2.0f,
            ApplyToAllFaces = false,
            TargetFace = BlockFace.Left | BlockFace.Right // On sides
        };
    }
    
    /// <summary>
    /// Get an engine glow stripe decal (cyan glow like on 1234.PNG engines)
    /// </summary>
    public static BlockDecal EngineGlowStripes()
    {
        return new BlockDecal
        {
            Pattern = DecalPattern.GlowStripes,
            PrimaryColor = 0x00CED1,  // Cyan (matching 1234.PNG engines)
            SecondaryColor = 0x1E90FF, // Bright blue
            Scale = 1.0f,
            Opacity = 0.9f,
            ApplyToAllFaces = false,
            TargetFace = BlockFace.Back // On engine faces
        };
    }
    
    /// <summary>
    /// Get a checker pattern decal
    /// </summary>
    public static BlockDecal CheckerPattern(uint color1, uint color2)
    {
        return new BlockDecal
        {
            Pattern = DecalPattern.CheckerPattern,
            PrimaryColor = color1,
            SecondaryColor = color2,
            Scale = 0.5f,
            ApplyToAllFaces = true,
            TargetFace = BlockFace.All
        };
    }
    
    /// <summary>
    /// Get a weathering/damage decal
    /// </summary>
    public static BlockDecal WeatheringMarks()
    {
        return new BlockDecal
        {
            Pattern = DecalPattern.WeatheringMarks,
            PrimaryColor = 0x4A3F30,  // Dark brown rust
            SecondaryColor = 0x2F2F2F, // Dark gray wear
            Scale = 1.5f,
            Opacity = 0.7f,
            ApplyToAllFaces = true,
            TargetFace = BlockFace.All
        };
    }
}
