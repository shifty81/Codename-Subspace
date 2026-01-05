using System.Numerics;

namespace AvorionLike.Core.Modular;

/// <summary>
/// Paint scheme for ship visual customization (X4-inspired)
/// </summary>
public class ShipPaintScheme
{
    public Guid Id { get; set; } = Guid.NewGuid();
    public string Name { get; set; } = "Default";
    public string Pattern { get; set; } = "Solid"; // Solid, Striped, Camo, Checkered, etc.
    
    // Color channels (RGB 0-255)
    public (int R, int G, int B) PrimaryColor { get; set; } = (128, 128, 128);
    public (int R, int G, int B) SecondaryColor { get; set; } = (64, 64, 64);
    public (int R, int G, int B) AccentColor { get; set; } = (255, 128, 0);
    public (int R, int G, int B) GlowColor { get; set; } = (100, 150, 255);
    
    // Material properties
    public float Metallic { get; set; } = 0.7f;
    public float Roughness { get; set; } = 0.5f;
    public float Emissive { get; set; } = 0.1f;
    
    // Quality tier
    public PaintQuality Quality { get; set; } = PaintQuality.Basic;
    
    // Whether this is a faction/special paint
    public bool IsFactionPaint { get; set; } = false;
    public string FactionName { get; set; } = "";
}

/// <summary>
/// Paint quality tiers like X4
/// </summary>
public enum PaintQuality
{
    Basic,
    Advanced,
    Exceptional
}

/// <summary>
/// Component for ship paint customization
/// </summary>
public class ShipPaintComponent
{
    public Guid EntityId { get; set; }
    public ShipPaintScheme CurrentPaint { get; set; } = new();
    public bool LockPaint { get; set; } = false; // Prevent override from default changes
    
    /// <summary>
    /// Apply a paint scheme to this ship
    /// </summary>
    public void ApplyPaint(ShipPaintScheme paint)
    {
        CurrentPaint = paint;
    }
    
    /// <summary>
    /// Reset to default paint
    /// </summary>
    public void ResetToDefault()
    {
        CurrentPaint = PaintLibrary.GetDefaultPaint();
    }
}

/// <summary>
/// Library of available paint schemes
/// </summary>
public static class PaintLibrary
{
    private static readonly List<ShipPaintScheme> _paints = new();
    
    static PaintLibrary()
    {
        InitializeDefaultPaints();
    }
    
    /// <summary>
    /// Initialize default paint schemes
    /// </summary>
    private static void InitializeDefaultPaints()
    {
        // Basic military gray
        _paints.Add(new ShipPaintScheme
        {
            Name = "Military Gray",
            Pattern = "Solid",
            PrimaryColor = (128, 128, 128),
            SecondaryColor = (96, 96, 96),
            AccentColor = (180, 180, 180),
            GlowColor = (100, 150, 255),
            Quality = PaintQuality.Basic
        });
        
        // Space black
        _paints.Add(new ShipPaintScheme
        {
            Name = "Space Black",
            Pattern = "Solid",
            PrimaryColor = (32, 32, 32),
            SecondaryColor = (16, 16, 16),
            AccentColor = (64, 64, 64),
            GlowColor = (50, 100, 200),
            Quality = PaintQuality.Basic
        });
        
        // Bright white
        _paints.Add(new ShipPaintScheme
        {
            Name = "Pristine White",
            Pattern = "Solid",
            PrimaryColor = (240, 240, 240),
            SecondaryColor = (200, 200, 200),
            AccentColor = (255, 255, 255),
            GlowColor = (150, 200, 255),
            Quality = PaintQuality.Advanced
        });
        
        // Combat red
        _paints.Add(new ShipPaintScheme
        {
            Name = "Combat Red",
            Pattern = "Striped",
            PrimaryColor = (180, 30, 30),
            SecondaryColor = (120, 20, 20),
            AccentColor = (255, 50, 50),
            GlowColor = (255, 100, 100),
            Quality = PaintQuality.Advanced
        });
        
        // Navy blue
        _paints.Add(new ShipPaintScheme
        {
            Name = "Navy Blue",
            Pattern = "Solid",
            PrimaryColor = (30, 60, 120),
            SecondaryColor = (20, 40, 80),
            AccentColor = (50, 100, 180),
            GlowColor = (100, 150, 255),
            Quality = PaintQuality.Advanced
        });
        
        // Merchant gold
        _paints.Add(new ShipPaintScheme
        {
            Name = "Merchant Gold",
            Pattern = "Solid",
            PrimaryColor = (180, 140, 60),
            SecondaryColor = (120, 90, 40),
            AccentColor = (255, 200, 80),
            GlowColor = (255, 220, 150),
            Metallic = 0.9f,
            Quality = PaintQuality.Exceptional
        });
        
        // Stealth camo
        _paints.Add(new ShipPaintScheme
        {
            Name = "Stealth Camo",
            Pattern = "Camo",
            PrimaryColor = (50, 50, 60),
            SecondaryColor = (30, 30, 40),
            AccentColor = (70, 70, 80),
            GlowColor = (40, 60, 100),
            Emissive = 0.05f,
            Quality = PaintQuality.Exceptional
        });
        
        // Pirate black & red
        _paints.Add(new ShipPaintScheme
        {
            Name = "Pirate",
            Pattern = "Striped",
            PrimaryColor = (20, 20, 20),
            SecondaryColor = (150, 20, 20),
            AccentColor = (255, 50, 50),
            GlowColor = (255, 0, 0),
            Quality = PaintQuality.Advanced
        });
        
        // Explorer green
        _paints.Add(new ShipPaintScheme
        {
            Name = "Explorer",
            Pattern = "Solid",
            PrimaryColor = (60, 120, 80),
            SecondaryColor = (40, 80, 60),
            AccentColor = (100, 180, 120),
            GlowColor = (100, 255, 150),
            Quality = PaintQuality.Advanced
        });
        
        // Racing orange
        _paints.Add(new ShipPaintScheme
        {
            Name = "Racing Orange",
            Pattern = "Striped",
            PrimaryColor = (255, 140, 0),
            SecondaryColor = (200, 100, 0),
            AccentColor = (255, 180, 50),
            GlowColor = (255, 200, 100),
            Quality = PaintQuality.Exceptional
        });
    }
    
    /// <summary>
    /// Get all available paint schemes
    /// </summary>
    public static List<ShipPaintScheme> GetAllPaints()
    {
        return new List<ShipPaintScheme>(_paints);
    }
    
    /// <summary>
    /// Get paint schemes by quality
    /// </summary>
    public static List<ShipPaintScheme> GetPaintsByQuality(PaintQuality quality)
    {
        return _paints.Where(p => p.Quality == quality).ToList();
    }
    
    /// <summary>
    /// Get default paint scheme
    /// </summary>
    public static ShipPaintScheme GetDefaultPaint()
    {
        return _paints.FirstOrDefault() ?? new ShipPaintScheme();
    }
    
    /// <summary>
    /// Add a custom paint scheme
    /// </summary>
    public static void AddPaint(ShipPaintScheme paint)
    {
        _paints.Add(paint);
    }
    
    /// <summary>
    /// Create a custom paint scheme
    /// </summary>
    public static ShipPaintScheme CreateCustomPaint(
        string name,
        (int, int, int) primaryColor,
        (int, int, int) secondaryColor,
        (int, int, int) accentColor,
        string pattern = "Solid")
    {
        return new ShipPaintScheme
        {
            Name = name,
            Pattern = pattern,
            PrimaryColor = primaryColor,
            SecondaryColor = secondaryColor,
            AccentColor = accentColor,
            GlowColor = (100, 150, 255),
            Quality = PaintQuality.Basic
        };
    }
}
