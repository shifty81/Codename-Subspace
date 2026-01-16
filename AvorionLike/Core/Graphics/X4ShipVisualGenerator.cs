using System.Numerics;
using AvorionLike.Core.Modular;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Generates X4: Foundations-inspired detailed ship visuals
/// Creates procedural ship meshes with X4's characteristic sleek, angular designs
/// </summary>
public class X4ShipVisualGenerator
{
    private readonly Random _random;
    
    public X4ShipVisualGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate enhanced visual geometry for an X4-style ship
    /// Adds detail geometry, engine glows, and X4-characteristic shapes
    /// </summary>
    public EnhancedShipVisuals GenerateShipVisuals(X4ShipClass shipClass, X4DesignStyle style, string material)
    {
        var visuals = new EnhancedShipVisuals
        {
            ShipClass = shipClass,
            DesignStyle = style
        };
        
        // Generate base hull shape with X4-style geometry
        visuals.HullGeometry = GenerateHullGeometry(shipClass, style);
        
        // Add engine details with glow effects
        visuals.EngineDetails = GenerateEngineDetails(shipClass, style);
        
        // Add surface details (panels, vents, antennas)
        visuals.SurfaceDetails = GenerateSurfaceDetails(shipClass, style);
        
        // Generate weapon hardpoints and mounts
        visuals.WeaponMounts = GenerateWeaponMounts(shipClass);
        
        // Add lighting fixtures and glow areas
        visuals.LightingDetails = GenerateLightingDetails(shipClass);
        
        return visuals;
    }
    
    /// <summary>
    /// Generate X4-style hull geometry with characteristic sleek lines
    /// </summary>
    private List<DetailGeometry> GenerateHullGeometry(X4ShipClass shipClass, X4DesignStyle style)
    {
        var geometry = new List<DetailGeometry>();
        
        // Get base dimensions based on ship class
        var dimensions = GetShipDimensions(shipClass);
        
        switch (style)
        {
            case X4DesignStyle.Sleek:
                // Paranid-style: elegant curves and sweeping lines
                geometry.AddRange(GenerateSleekHull(dimensions));
                break;
                
            case X4DesignStyle.Aggressive:
                // Split-style: angular, sharp, aggressive lines
                geometry.AddRange(GenerateAggressiveHull(dimensions));
                break;
                
            case X4DesignStyle.Durable:
                // Teladi-style: chunky, industrial, robust
                geometry.AddRange(GenerateDurableHull(dimensions));
                break;
                
            case X4DesignStyle.Advanced:
                // Terran-style: high-tech, clean lines, advanced
                geometry.AddRange(GenerateAdvancedHull(dimensions));
                break;
                
            case X4DesignStyle.Alien:
                // Xenon-style: otherworldly, unique geometry
                geometry.AddRange(GenerateAlienHull(dimensions));
                break;
                
            default: // Balanced (Argon-style)
                geometry.AddRange(GenerateBalancedHull(dimensions));
                break;
        }
        
        return geometry;
    }
    
    /// <summary>
    /// Generate sleek, elegant hull (Paranid-inspired)
    /// </summary>
    private List<DetailGeometry> GenerateSleekHull(Vector3 dimensions)
    {
        var geometry = new List<DetailGeometry>();
        
        // Main hull with tapered nose
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.TaperedCylinder,
            Position = Vector3.Zero,
            Scale = new Vector3(dimensions.X * 0.4f, dimensions.Y * 0.5f, dimensions.Z),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.8f, 0.85f, 0.9f), // Light metallic
            Metallic = 0.8f,
            Smoothness = 0.9f
        });
        
        // Swept-back wings
        for (int side = -1; side <= 1; side += 2)
        {
            geometry.Add(new DetailGeometry
            {
                Type = GeometryType.SweptWing,
                Position = new Vector3(side * dimensions.X * 0.6f, 0, dimensions.Z * 0.2f),
                Scale = new Vector3(dimensions.X * 0.4f, dimensions.Y * 0.1f, dimensions.Z * 0.6f),
                Rotation = new Vector3(0, 0, side * 15f),
                Color = new Vector3(0.75f, 0.8f, 0.85f),
                Metallic = 0.7f,
                Smoothness = 0.85f
            });
        }
        
        // Elegant cockpit canopy
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.BubbleCanopy,
            Position = new Vector3(0, dimensions.Y * 0.3f, dimensions.Z * 0.8f),
            Scale = new Vector3(dimensions.X * 0.3f, dimensions.Y * 0.4f, dimensions.Z * 0.3f),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.2f, 0.4f, 0.6f), // Tinted glass
            Metallic = 0.1f,
            Smoothness = 0.95f,
            Emissive = 0.1f
        });
        
        return geometry;
    }
    
    /// <summary>
    /// Generate aggressive, angular hull (Split-inspired)
    /// </summary>
    private List<DetailGeometry> GenerateAggressiveHull(Vector3 dimensions)
    {
        var geometry = new List<DetailGeometry>();
        
        // Angular main hull with sharp nose
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.AngularWedge,
            Position = Vector3.Zero,
            Scale = new Vector3(dimensions.X * 0.5f, dimensions.Y * 0.5f, dimensions.Z),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.7f, 0.2f, 0.1f), // Red/orange aggressive
            Metallic = 0.6f,
            Smoothness = 0.4f
        });
        
        // Sharp wings with weapon pylons
        for (int side = -1; side <= 1; side += 2)
        {
            geometry.Add(new DetailGeometry
            {
                Type = GeometryType.SharpWing,
                Position = new Vector3(side * dimensions.X * 0.7f, -dimensions.Y * 0.1f, 0),
                Scale = new Vector3(dimensions.X * 0.3f, dimensions.Y * 0.15f, dimensions.Z * 0.8f),
                Rotation = new Vector3(0, 0, side * 5f),
                Color = new Vector3(0.65f, 0.15f, 0.05f),
                Metallic = 0.5f,
                Smoothness = 0.3f
            });
        }
        
        // Armored cockpit
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.ArmoredCockpit,
            Position = new Vector3(0, dimensions.Y * 0.2f, dimensions.Z * 0.7f),
            Scale = new Vector3(dimensions.X * 0.25f, dimensions.Y * 0.3f, dimensions.Z * 0.25f),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.3f, 0.3f, 0.3f),
            Metallic = 0.8f,
            Smoothness = 0.5f
        });
        
        return geometry;
    }
    
    /// <summary>
    /// Generate durable, industrial hull (Teladi-inspired)
    /// </summary>
    private List<DetailGeometry> GenerateDurableHull(Vector3 dimensions)
    {
        var geometry = new List<DetailGeometry>();
        
        // Chunky main hull
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.BoxHull,
            Position = Vector3.Zero,
            Scale = new Vector3(dimensions.X * 0.7f, dimensions.Y * 0.6f, dimensions.Z * 0.9f),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.5f, 0.55f, 0.5f), // Greenish industrial
            Metallic = 0.4f,
            Smoothness = 0.3f
        });
        
        // Cargo pods/reinforced sections
        for (int side = -1; side <= 1; side += 2)
        {
            geometry.Add(new DetailGeometry
            {
                Type = GeometryType.CargoPod,
                Position = new Vector3(side * dimensions.X * 0.6f, -dimensions.Y * 0.2f, -dimensions.Z * 0.2f),
                Scale = new Vector3(dimensions.X * 0.3f, dimensions.Y * 0.4f, dimensions.Z * 0.5f),
                Rotation = Vector3.Zero,
                Color = new Vector3(0.45f, 0.5f, 0.45f),
                Metallic = 0.3f,
                Smoothness = 0.2f
            });
        }
        
        // Heavy reinforced cockpit
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.ReinforcedCockpit,
            Position = new Vector3(0, dimensions.Y * 0.3f, dimensions.Z * 0.6f),
            Scale = new Vector3(dimensions.X * 0.4f, dimensions.Y * 0.4f, dimensions.Z * 0.3f),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.4f, 0.45f, 0.4f),
            Metallic = 0.6f,
            Smoothness = 0.4f
        });
        
        return geometry;
    }
    
    /// <summary>
    /// Generate advanced, high-tech hull (Terran-inspired)
    /// </summary>
    private List<DetailGeometry> GenerateAdvancedHull(Vector3 dimensions)
    {
        var geometry = new List<DetailGeometry>();
        
        // Streamlined main hull with clean lines
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.StreamlinedHull,
            Position = Vector3.Zero,
            Scale = new Vector3(dimensions.X * 0.5f, dimensions.Y * 0.5f, dimensions.Z),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.85f, 0.9f, 0.95f), // Clean white/blue
            Metallic = 0.7f,
            Smoothness = 0.85f
        });
        
        // Integrated wings with tech panels
        for (int side = -1; side <= 1; side += 2)
        {
            geometry.Add(new DetailGeometry
            {
                Type = GeometryType.TechWing,
                Position = new Vector3(side * dimensions.X * 0.5f, 0, dimensions.Z * 0.1f),
                Scale = new Vector3(dimensions.X * 0.35f, dimensions.Y * 0.12f, dimensions.Z * 0.7f),
                Rotation = new Vector3(0, 0, side * 3f),
                Color = new Vector3(0.8f, 0.85f, 0.9f),
                Metallic = 0.75f,
                Smoothness = 0.8f
            });
        }
        
        // Advanced sensor array/cockpit
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.SensorArray,
            Position = new Vector3(0, dimensions.Y * 0.25f, dimensions.Z * 0.75f),
            Scale = new Vector3(dimensions.X * 0.3f, dimensions.Y * 0.35f, dimensions.Z * 0.3f),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.3f, 0.5f, 0.7f),
            Metallic = 0.5f,
            Smoothness = 0.9f,
            Emissive = 0.2f
        });
        
        return geometry;
    }
    
    /// <summary>
    /// Generate alien, otherworldly hull (Xenon-inspired)
    /// </summary>
    private List<DetailGeometry> GenerateAlienHull(Vector3 dimensions)
    {
        var geometry = new List<DetailGeometry>();
        
        // Organic-looking main hull
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.OrganicHull,
            Position = Vector3.Zero,
            Scale = new Vector3(dimensions.X * 0.45f, dimensions.Y * 0.55f, dimensions.Z),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.15f, 0.15f, 0.2f), // Dark with slight blue
            Metallic = 0.9f,
            Smoothness = 0.7f,
            Emissive = 0.15f
        });
        
        // Asymmetric appendages
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.AlienAppendage,
            Position = new Vector3(dimensions.X * 0.4f, dimensions.Y * 0.2f, dimensions.Z * 0.3f),
            Scale = new Vector3(dimensions.X * 0.3f, dimensions.Y * 0.2f, dimensions.Z * 0.4f),
            Rotation = new Vector3(15f, 20f, 10f),
            Color = new Vector3(0.1f, 0.1f, 0.15f),
            Metallic = 0.85f,
            Smoothness = 0.6f
        });
        
        // Glowing core/sensor
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.GlowingCore,
            Position = new Vector3(0, 0, dimensions.Z * 0.6f),
            Scale = new Vector3(dimensions.X * 0.25f, dimensions.Y * 0.25f, dimensions.Z * 0.2f),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.8f, 0.2f, 0.2f), // Red glow
            Metallic = 0.3f,
            Smoothness = 0.5f,
            Emissive = 0.8f
        });
        
        return geometry;
    }
    
    /// <summary>
    /// Generate balanced, versatile hull (Argon-inspired)
    /// </summary>
    private List<DetailGeometry> GenerateBalancedHull(Vector3 dimensions)
    {
        var geometry = new List<DetailGeometry>();
        
        // Balanced main hull
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.StandardHull,
            Position = Vector3.Zero,
            Scale = new Vector3(dimensions.X * 0.5f, dimensions.Y * 0.5f, dimensions.Z),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.6f, 0.65f, 0.7f), // Gray metallic
            Metallic = 0.6f,
            Smoothness = 0.6f
        });
        
        // Standard wings
        for (int side = -1; side <= 1; side += 2)
        {
            geometry.Add(new DetailGeometry
            {
                Type = GeometryType.StandardWing,
                Position = new Vector3(side * dimensions.X * 0.55f, 0, dimensions.Z * 0.15f),
                Scale = new Vector3(dimensions.X * 0.35f, dimensions.Y * 0.12f, dimensions.Z * 0.65f),
                Rotation = new Vector3(0, 0, side * 8f),
                Color = new Vector3(0.55f, 0.6f, 0.65f),
                Metallic = 0.55f,
                Smoothness = 0.55f
            });
        }
        
        // Standard cockpit
        geometry.Add(new DetailGeometry
        {
            Type = GeometryType.StandardCockpit,
            Position = new Vector3(0, dimensions.Y * 0.25f, dimensions.Z * 0.7f),
            Scale = new Vector3(dimensions.X * 0.3f, dimensions.Y * 0.35f, dimensions.Z * 0.3f),
            Rotation = Vector3.Zero,
            Color = new Vector3(0.3f, 0.4f, 0.5f),
            Metallic = 0.4f,
            Smoothness = 0.8f,
            Emissive = 0.05f
        });
        
        return geometry;
    }
    
    /// <summary>
    /// Generate engine details with glow effects
    /// </summary>
    private List<EngineDetail> GenerateEngineDetails(X4ShipClass shipClass, X4DesignStyle style)
    {
        var details = new List<EngineDetail>();
        var dimensions = GetShipDimensions(shipClass);
        
        // Number of engines based on ship class
        int engineCount = shipClass switch
        {
            X4ShipClass.Fighter_Light => 2,
            X4ShipClass.Fighter_Heavy => 2,
            X4ShipClass.Miner_Small => 2,
            X4ShipClass.Corvette => 2,
            X4ShipClass.Frigate => 4,
            X4ShipClass.Gunboat => 3,
            X4ShipClass.Miner_Medium => 2,
            X4ShipClass.Freighter_Medium => 4,
            X4ShipClass.Destroyer => 4,
            X4ShipClass.Freighter_Large => 6,
            X4ShipClass.Miner_Large => 4,
            X4ShipClass.Battleship => 6,
            X4ShipClass.Carrier => 8,
            X4ShipClass.Builder => 4,
            _ => 2
        };
        
        // Engine glow color based on style
        Vector3 glowColor = style switch
        {
            X4DesignStyle.Aggressive => new Vector3(1.0f, 0.3f, 0.1f), // Orange/red
            X4DesignStyle.Advanced => new Vector3(0.3f, 0.6f, 1.0f), // Blue
            X4DesignStyle.Alien => new Vector3(0.8f, 0.1f, 0.3f), // Purple/red
            _ => new Vector3(0.4f, 0.7f, 1.0f) // Standard blue
        };
        
        // Place engines symmetrically
        for (int i = 0; i < engineCount; i++)
        {
            float spacing = dimensions.X / (engineCount / 2 + 1);
            int side = (i % 2 == 0) ? 1 : -1;
            int row = i / 2;
            
            details.Add(new EngineDetail
            {
                Position = new Vector3(
                    side * spacing * (row + 1) * 0.8f,
                    -dimensions.Y * 0.2f,
                    -dimensions.Z * 0.5f
                ),
                Scale = new Vector3(
                    dimensions.X * 0.15f,
                    dimensions.Y * 0.15f,
                    dimensions.Z * 0.3f
                ),
                GlowColor = glowColor,
                GlowIntensity = 1.5f,
                ParticleEffects = true
            });
        }
        
        return details;
    }
    
    /// <summary>
    /// Generate surface details (panels, vents, antennas)
    /// </summary>
    private List<SurfaceDetail> GenerateSurfaceDetails(X4ShipClass shipClass, X4DesignStyle style)
    {
        var details = new List<SurfaceDetail>();
        var dimensions = GetShipDimensions(shipClass);
        
        // Detail density based on ship class (larger ships get more detail)
        int detailCount = shipClass switch
        {
            X4ShipClass.Fighter_Light => 5,
            X4ShipClass.Fighter_Heavy => 8,
            X4ShipClass.Corvette => 12,
            X4ShipClass.Frigate => 18,
            X4ShipClass.Destroyer => 25,
            X4ShipClass.Battleship => 35,
            X4ShipClass.Carrier => 40,
            _ => 10
        };
        
        for (int i = 0; i < detailCount; i++)
        {
            var detailType = (SurfaceDetailType)_random.Next((int)SurfaceDetailType.Count);
            
            details.Add(new SurfaceDetail
            {
                Type = detailType,
                Position = new Vector3(
                    (float)(_random.NextDouble() - 0.5) * dimensions.X,
                    (float)(_random.NextDouble() - 0.5) * dimensions.Y,
                    (float)(_random.NextDouble() - 0.5) * dimensions.Z
                ),
                Scale = new Vector3(
                    dimensions.X * 0.05f,
                    dimensions.Y * 0.05f,
                    dimensions.Z * 0.05f
                ),
                Rotation = new Vector3(
                    (float)_random.NextDouble() * 360f,
                    (float)_random.NextDouble() * 360f,
                    (float)_random.NextDouble() * 360f
                )
            });
        }
        
        return details;
    }
    
    /// <summary>
    /// Generate weapon hardpoints and mounts
    /// </summary>
    private List<WeaponMount> GenerateWeaponMounts(X4ShipClass shipClass)
    {
        var mounts = new List<WeaponMount>();
        var dimensions = GetShipDimensions(shipClass);
        
        // Weapon count based on ship class
        int weaponCount = shipClass switch
        {
            X4ShipClass.Fighter_Light => 2,
            X4ShipClass.Fighter_Heavy => 4,
            X4ShipClass.Corvette => 4,
            X4ShipClass.Frigate => 6,
            X4ShipClass.Gunboat => 8,
            X4ShipClass.Destroyer => 10,
            X4ShipClass.Battleship => 16,
            X4ShipClass.Carrier => 12,
            _ => 2
        };
        
        for (int i = 0; i < weaponCount; i++)
        {
            int side = (i % 2 == 0) ? 1 : -1;
            float positionZ = dimensions.Z * (0.3f + (float)i / weaponCount * 0.4f);
            
            mounts.Add(new WeaponMount
            {
                Position = new Vector3(
                    side * dimensions.X * 0.4f,
                    dimensions.Y * 0.1f,
                    positionZ
                ),
                MountType = (i < weaponCount / 2) ? WeaponMountType.Fixed : WeaponMountType.Turret,
                Size = WeaponSize.Medium
            });
        }
        
        return mounts;
    }
    
    /// <summary>
    /// Generate lighting details (running lights, navigation lights)
    /// </summary>
    private List<LightingDetail> GenerateLightingDetails(X4ShipClass shipClass)
    {
        var lights = new List<LightingDetail>();
        var dimensions = GetShipDimensions(shipClass);
        
        // Navigation lights (red/green on wings)
        lights.Add(new LightingDetail
        {
            Position = new Vector3(dimensions.X * 0.6f, 0, dimensions.Z * 0.3f),
            Color = new Vector3(0.0f, 1.0f, 0.0f), // Green (starboard)
            Intensity = 0.8f,
            BlinkPattern = BlinkPattern.Steady
        });
        
        lights.Add(new LightingDetail
        {
            Position = new Vector3(-dimensions.X * 0.6f, 0, dimensions.Z * 0.3f),
            Color = new Vector3(1.0f, 0.0f, 0.0f), // Red (port)
            Intensity = 0.8f,
            BlinkPattern = BlinkPattern.Steady
        });
        
        // Forward running lights
        lights.Add(new LightingDetail
        {
            Position = new Vector3(0, dimensions.Y * 0.2f, dimensions.Z * 0.9f),
            Color = new Vector3(1.0f, 1.0f, 1.0f), // White
            Intensity = 1.2f,
            BlinkPattern = BlinkPattern.Steady
        });
        
        // Rear position lights
        lights.Add(new LightingDetail
        {
            Position = new Vector3(0, 0, -dimensions.Z * 0.5f),
            Color = new Vector3(1.0f, 1.0f, 1.0f),
            Intensity = 0.6f,
            BlinkPattern = BlinkPattern.Slow
        });
        
        return lights;
    }
    
    /// <summary>
    /// Get base dimensions for a ship class
    /// </summary>
    private Vector3 GetShipDimensions(X4ShipClass shipClass)
    {
        return shipClass switch
        {
            X4ShipClass.Fighter_Light => new Vector3(10f, 3f, 15f),
            X4ShipClass.Fighter_Heavy => new Vector3(12f, 4f, 18f),
            X4ShipClass.Miner_Small => new Vector3(15f, 8f, 20f),
            X4ShipClass.Corvette => new Vector3(20f, 8f, 35f),
            X4ShipClass.Frigate => new Vector3(30f, 12f, 60f),
            X4ShipClass.Gunboat => new Vector3(25f, 10f, 50f),
            X4ShipClass.Miner_Medium => new Vector3(40f, 20f, 70f),
            X4ShipClass.Freighter_Medium => new Vector3(50f, 25f, 80f),
            X4ShipClass.Destroyer => new Vector3(60f, 25f, 120f),
            X4ShipClass.Freighter_Large => new Vector3(100f, 50f, 180f),
            X4ShipClass.Miner_Large => new Vector3(80f, 40f, 140f),
            X4ShipClass.Battleship => new Vector3(120f, 50f, 250f),
            X4ShipClass.Carrier => new Vector3(200f, 80f, 400f),
            X4ShipClass.Builder => new Vector3(70f, 35f, 120f),
            _ => new Vector3(20f, 8f, 35f)
        };
    }
}

/// <summary>
/// Enhanced visual data for a ship
/// </summary>
public class EnhancedShipVisuals
{
    public X4ShipClass ShipClass { get; set; }
    public X4DesignStyle DesignStyle { get; set; }
    public List<DetailGeometry> HullGeometry { get; set; } = new();
    public List<EngineDetail> EngineDetails { get; set; } = new();
    public List<SurfaceDetail> SurfaceDetails { get; set; } = new();
    public List<WeaponMount> WeaponMounts { get; set; } = new();
    public List<LightingDetail> LightingDetails { get; set; } = new();
}

/// <summary>
/// Detailed geometry element
/// </summary>
public class DetailGeometry
{
    public GeometryType Type { get; set; }
    public Vector3 Position { get; set; }
    public Vector3 Scale { get; set; }
    public Vector3 Rotation { get; set; }
    public Vector3 Color { get; set; }
    public float Metallic { get; set; }
    public float Smoothness { get; set; }
    public float Emissive { get; set; }
}

/// <summary>
/// Engine visual detail with glow
/// </summary>
public class EngineDetail
{
    public Vector3 Position { get; set; }
    public Vector3 Scale { get; set; }
    public Vector3 GlowColor { get; set; }
    public float GlowIntensity { get; set; }
    public bool ParticleEffects { get; set; }
}

/// <summary>
/// Surface detail element (panels, vents, antennas)
/// </summary>
public class SurfaceDetail
{
    public SurfaceDetailType Type { get; set; }
    public Vector3 Position { get; set; }
    public Vector3 Scale { get; set; }
    public Vector3 Rotation { get; set; }
}

/// <summary>
/// Weapon mount location
/// </summary>
public class WeaponMount
{
    public Vector3 Position { get; set; }
    public WeaponMountType MountType { get; set; }
    public WeaponSize Size { get; set; }
}

/// <summary>
/// Lighting detail (navigation lights, running lights)
/// </summary>
public class LightingDetail
{
    public Vector3 Position { get; set; }
    public Vector3 Color { get; set; }
    public float Intensity { get; set; }
    public BlinkPattern BlinkPattern { get; set; }
}

// Enums for visual elements
public enum GeometryType
{
    TaperedCylinder, SweptWing, BubbleCanopy,
    AngularWedge, SharpWing, ArmoredCockpit,
    BoxHull, CargoPod, ReinforcedCockpit,
    StreamlinedHull, TechWing, SensorArray,
    OrganicHull, AlienAppendage, GlowingCore,
    StandardHull, StandardWing, StandardCockpit
}

public enum SurfaceDetailType
{
    Panel, Vent, Antenna, Sensor, Light, Decal, Count
}

public enum WeaponMountType
{
    Fixed, Turret, Missile
}

public enum WeaponSize
{
    Small, Medium, Large
}

public enum BlinkPattern
{
    Steady, Slow, Fast, Pulse
}
