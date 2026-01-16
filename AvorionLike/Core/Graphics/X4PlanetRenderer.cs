using System.Numerics;
using AvorionLike.Core.Procedural;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// X4-inspired planet renderer with detailed sphere generation
/// Creates realistic planets with surface details, atmospheres, and optional rings
/// </summary>
public class X4PlanetRenderer
{
    private readonly CelestialTextureGenerator _textureGen;
    private readonly Random _random;
    
    public X4PlanetRenderer(int seed = 0)
    {
        _textureGen = new CelestialTextureGenerator(seed);
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate detailed planet mesh with X4-style quality
    /// </summary>
    public DetailedPlanet GenerateDetailedPlanet(PlanetData planetData, int subdivisionLevel = 4)
    {
        var planet = new DetailedPlanet
        {
            Data = planetData,
            Position = planetData.Position,
            Radius = planetData.Size
        };
        
        // Generate sphere mesh with subdivision for smoothness
        planet.SphereMesh = GenerateIcosphere(planet.Radius, subdivisionLevel);
        
        // Apply surface details based on planet type
        planet.SurfaceDetails = GenerateSurfaceDetails(planetData);
        
        // Add atmosphere if applicable
        if (HasAtmosphere(planetData.Type))
        {
            planet.Atmosphere = GenerateAtmosphere(planetData);
        }
        
        // Add rings if applicable (gas giants, some rocky planets)
        if (ShouldHaveRings(planetData.Type))
        {
            planet.Rings = GenerateRings(planetData);
        }
        
        // Add clouds for habitable/ocean planets
        if (planetData.Type == PlanetType.Habitable || planetData.Type == PlanetType.Ocean)
        {
            planet.CloudLayer = GenerateCloudLayer(planetData);
        }
        
        // Generate lighting data
        planet.LightingData = GeneratePlanetLighting(planetData);
        
        return planet;
    }
    
    /// <summary>
    /// Generate icosphere mesh for smooth planetary surfaces
    /// Uses subdivision for X4-quality smoothness
    /// </summary>
    private SphereMesh GenerateIcosphere(float radius, int subdivisionLevel)
    {
        var mesh = new SphereMesh();
        
        // Start with icosahedron (20 faces, 12 vertices)
        const float t = (1.0f + (float)Math.Sqrt(5.0f)) / 2.0f;
        
        // Initial vertices
        var vertices = new List<Vector3>
        {
            new Vector3(-1, t, 0).Normalized() * radius,
            new Vector3(1, t, 0).Normalized() * radius,
            new Vector3(-1, -t, 0).Normalized() * radius,
            new Vector3(1, -t, 0).Normalized() * radius,
            new Vector3(0, -1, t).Normalized() * radius,
            new Vector3(0, 1, t).Normalized() * radius,
            new Vector3(0, -1, -t).Normalized() * radius,
            new Vector3(0, 1, -t).Normalized() * radius,
            new Vector3(t, 0, -1).Normalized() * radius,
            new Vector3(t, 0, 1).Normalized() * radius,
            new Vector3(-t, 0, -1).Normalized() * radius,
            new Vector3(-t, 0, 1).Normalized() * radius
        };
        
        // Initial faces (20 triangles)
        var triangles = new List<int>
        {
            0, 11, 5,  0, 5, 1,  0, 1, 7,  0, 7, 10,  0, 10, 11,
            1, 5, 9,  5, 11, 4,  11, 10, 2,  10, 7, 6,  7, 1, 8,
            3, 9, 4,  3, 4, 2,  3, 2, 6,  3, 6, 8,  3, 8, 9,
            4, 9, 5,  2, 4, 11,  6, 2, 10,  8, 6, 7,  9, 8, 1
        };
        
        // Subdivide
        for (int i = 0; i < subdivisionLevel; i++)
        {
            var newTriangles = new List<int>();
            var midpointCache = new Dictionary<long, int>();
            
            for (int t = 0; t < triangles.Count; t += 3)
            {
                int v1 = triangles[t];
                int v2 = triangles[t + 1];
                int v3 = triangles[t + 2];
                
                // Get midpoints (or create if new)
                int m1 = GetMidpoint(v1, v2, vertices, midpointCache, radius);
                int m2 = GetMidpoint(v2, v3, vertices, midpointCache, radius);
                int m3 = GetMidpoint(v3, v1, vertices, midpointCache, radius);
                
                // Create 4 new triangles
                newTriangles.AddRange(new[] { v1, m1, m3 });
                newTriangles.AddRange(new[] { v2, m2, m1 });
                newTriangles.AddRange(new[] { v3, m3, m2 });
                newTriangles.AddRange(new[] { m1, m2, m3 });
            }
            
            triangles = newTriangles;
        }
        
        mesh.Vertices = vertices.ToArray();
        mesh.Triangles = triangles.ToArray();
        mesh.Normals = vertices.Select(v => Vector3.Normalize(v)).ToArray();
        mesh.UVs = GenerateSphericalUVs(vertices.ToArray());
        
        return mesh;
    }
    
    /// <summary>
    /// Get or create midpoint vertex for subdivision
    /// </summary>
    private int GetMidpoint(int v1, int v2, List<Vector3> vertices, Dictionary<long, int> cache, float radius)
    {
        long key = ((long)Math.Min(v1, v2) << 32) | (long)Math.Max(v1, v2);
        
        if (cache.TryGetValue(key, out int index))
            return index;
        
        Vector3 mid = (vertices[v1] + vertices[v2]) / 2f;
        mid = Vector3.Normalize(mid) * radius;
        
        vertices.Add(mid);
        index = vertices.Count - 1;
        cache[key] = index;
        
        return index;
    }
    
    /// <summary>
    /// Generate spherical UV coordinates
    /// </summary>
    private Vector2[] GenerateSphericalUVs(Vector3[] vertices)
    {
        var uvs = new Vector2[vertices.Length];
        
        for (int i = 0; i < vertices.Length; i++)
        {
            Vector3 v = Vector3.Normalize(vertices[i]);
            
            float u = 0.5f + (float)Math.Atan2(v.Z, v.X) / (2f * (float)Math.PI);
            float v_coord = 0.5f - (float)Math.Asin(v.Y) / (float)Math.PI;
            
            uvs[i] = new Vector2(u, v_coord);
        }
        
        return uvs;
    }
    
    /// <summary>
    /// Generate surface details based on planet type
    /// </summary>
    private PlanetSurfaceDetails GenerateSurfaceDetails(PlanetData planetData)
    {
        var details = new PlanetSurfaceDetails();
        
        switch (planetData.Type)
        {
            case PlanetType.Rocky:
                details.CraterCount = 100 + _random.Next(200);
                details.MountainRanges = 5 + _random.Next(10);
                details.ColorVariation = 0.3f;
                details.BaseColor = new Vector3(0.6f, 0.5f, 0.4f);
                break;
                
            case PlanetType.Desert:
                details.CraterCount = 20 + _random.Next(50);
                details.DunePatterns = 50 + _random.Next(100);
                details.ColorVariation = 0.2f;
                details.BaseColor = new Vector3(0.9f, 0.7f, 0.4f);
                break;
                
            case PlanetType.Ice:
                details.CrackPatterns = 30 + _random.Next(70);
                details.IceCaps = true;
                details.ColorVariation = 0.15f;
                details.BaseColor = new Vector3(0.9f, 0.95f, 1.0f);
                break;
                
            case PlanetType.Lava:
                details.VolcanoCount = 10 + _random.Next(20);
                details.LavaFlows = 20 + _random.Next(40);
                details.ColorVariation = 0.4f;
                details.BaseColor = new Vector3(0.3f, 0.1f, 0.05f);
                details.EmissiveStrength = 0.8f;
                break;
                
            case PlanetType.Ocean:
                details.OceanCoverage = 0.7f + (float)_random.NextDouble() * 0.25f;
                details.LandMasses = 5 + _random.Next(10);
                details.ColorVariation = 0.25f;
                details.BaseColor = new Vector3(0.2f, 0.4f, 0.7f);
                break;
                
            case PlanetType.Habitable:
                details.ContinentCount = 3 + _random.Next(5);
                details.OceanCoverage = 0.5f + (float)_random.NextDouble() * 0.3f;
                details.ForestRegions = 10 + _random.Next(20);
                details.ColorVariation = 0.35f;
                details.BaseColor = new Vector3(0.3f, 0.5f, 0.7f);
                break;
                
            case PlanetType.GasGiant:
                details.BandCount = 8 + _random.Next(12);
                details.StormSystems = 2 + _random.Next(6);
                details.ColorVariation = 0.4f;
                details.BaseColor = _random.Next(3) switch
                {
                    0 => new Vector3(0.8f, 0.6f, 0.4f), // Jupiter-like
                    1 => new Vector3(0.3f, 0.5f, 0.9f), // Neptune-like
                    _ => new Vector3(0.9f, 0.85f, 0.6f)  // Saturn-like
                };
                break;
                
            case PlanetType.Toxic:
                details.ToxicClouds = 30 + _random.Next(50);
                details.AcidLakes = 10 + _random.Next(20);
                details.ColorVariation = 0.3f;
                details.BaseColor = new Vector3(0.5f, 0.7f, 0.3f);
                break;
        }
        
        return details;
    }
    
    /// <summary>
    /// Generate atmospheric layer
    /// </summary>
    private AtmosphereData GenerateAtmosphere(PlanetData planetData)
    {
        var atmosphere = new AtmosphereData
        {
            InnerRadius = planetData.Size,
            OuterRadius = planetData.Size * 1.05f // 5% larger than surface
        };
        
        switch (planetData.Type)
        {
            case PlanetType.Habitable:
                atmosphere.Color = new Vector3(0.5f, 0.7f, 1.0f); // Blue
                atmosphere.Density = 1.0f;
                atmosphere.ScatteringStrength = 0.8f;
                break;
                
            case PlanetType.Toxic:
                atmosphere.Color = new Vector3(0.7f, 0.9f, 0.4f); // Yellow-green
                atmosphere.Density = 1.3f;
                atmosphere.ScatteringStrength = 0.6f;
                break;
                
            case PlanetType.GasGiant:
                atmosphere.Color = new Vector3(0.8f, 0.8f, 0.7f); // Pale
                atmosphere.Density = 2.0f;
                atmosphere.ScatteringStrength = 1.0f;
                break;
                
            default:
                atmosphere.Color = new Vector3(0.6f, 0.6f, 0.7f); // Generic
                atmosphere.Density = 0.5f;
                atmosphere.ScatteringStrength = 0.4f;
                break;
        }
        
        return atmosphere;
    }
    
    /// <summary>
    /// Generate planetary rings
    /// </summary>
    private PlanetaryRings GenerateRings(PlanetData planetData)
    {
        var rings = new PlanetaryRings
        {
            InnerRadius = planetData.Size * 1.5f,
            OuterRadius = planetData.Size * 2.5f,
            Thickness = planetData.Size * 0.05f
        };
        
        // Ring appearance based on planet type
        if (planetData.Type == PlanetType.GasGiant)
        {
            rings.RingCount = 3 + _random.Next(4);
            rings.Color = new Vector3(0.8f, 0.75f, 0.7f); // Ice/rock
            rings.Opacity = 0.6f + (float)_random.NextDouble() * 0.3f;
            rings.HasGaps = true;
        }
        else
        {
            rings.RingCount = 1 + _random.Next(2);
            rings.Color = new Vector3(0.6f, 0.55f, 0.5f);
            rings.Opacity = 0.4f + (float)_random.NextDouble() * 0.2f;
            rings.HasGaps = _random.NextDouble() > 0.5;
        }
        
        return rings;
    }
    
    /// <summary>
    /// Generate cloud layer for atmospheric planets
    /// </summary>
    private CloudLayer GenerateCloudLayer(PlanetData planetData)
    {
        return new CloudLayer
        {
            Radius = planetData.Size * 1.02f, // Slightly above surface
            Coverage = 0.3f + (float)_random.NextDouble() * 0.4f,
            CloudColor = new Vector3(0.9f, 0.9f, 0.95f),
            Opacity = 0.6f,
            AnimationSpeed = 0.1f + (float)_random.NextDouble() * 0.2f,
            CloudScale = 5f + (float)_random.NextDouble() * 10f
        };
    }
    
    /// <summary>
    /// Generate lighting data for the planet
    /// </summary>
    private PlanetLightingData GeneratePlanetLighting(PlanetData planetData)
    {
        var lighting = new PlanetLightingData
        {
            AmbientColor = new Vector3(0.1f, 0.1f, 0.15f),
            SpecularStrength = planetData.Type switch
            {
                PlanetType.Ocean => 0.8f,
                PlanetType.Ice => 0.9f,
                PlanetType.GasGiant => 0.3f,
                _ => 0.2f
            },
            Roughness = planetData.Type switch
            {
                PlanetType.Rocky => 0.9f,
                PlanetType.Desert => 0.8f,
                PlanetType.Ice => 0.1f,
                PlanetType.Ocean => 0.2f,
                PlanetType.GasGiant => 0.6f,
                _ => 0.7f
            }
        };
        
        // Emissive for lava planets
        if (planetData.Type == PlanetType.Lava)
        {
            lighting.EmissiveColor = new Vector3(1.0f, 0.3f, 0.1f);
            lighting.EmissiveStrength = 0.8f;
        }
        
        return lighting;
    }
    
    /// <summary>
    /// Check if planet type should have atmosphere
    /// </summary>
    private bool HasAtmosphere(PlanetType type)
    {
        return type switch
        {
            PlanetType.Habitable => true,
            PlanetType.Ocean => true,
            PlanetType.Toxic => true,
            PlanetType.GasGiant => true,
            _ => false
        };
    }
    
    /// <summary>
    /// Check if planet should have rings
    /// </summary>
    private bool ShouldHaveRings(PlanetType type)
    {
        if (type == PlanetType.GasGiant)
            return _random.NextDouble() > 0.3; // 70% chance for gas giants
        
        return _random.NextDouble() > 0.9; // 10% chance for others
    }
}

/// <summary>
/// Detailed planet data with full rendering information
/// </summary>
public class DetailedPlanet
{
    public PlanetData Data { get; set; }
    public Vector3 Position { get; set; }
    public float Radius { get; set; }
    public SphereMesh SphereMesh { get; set; } = null!;
    public PlanetSurfaceDetails SurfaceDetails { get; set; } = null!;
    public AtmosphereData? Atmosphere { get; set; }
    public PlanetaryRings? Rings { get; set; }
    public CloudLayer? CloudLayer { get; set; }
    public PlanetLightingData LightingData { get; set; } = null!;
}

/// <summary>
/// Sphere mesh data
/// </summary>
public class SphereMesh
{
    public Vector3[] Vertices { get; set; } = Array.Empty<Vector3>();
    public int[] Triangles { get; set; } = Array.Empty<int>();
    public Vector3[] Normals { get; set; } = Array.Empty<Vector3>();
    public Vector2[] UVs { get; set; } = Array.Empty<Vector2>();
}

/// <summary>
/// Planet surface details
/// </summary>
public class PlanetSurfaceDetails
{
    public Vector3 BaseColor { get; set; }
    public float ColorVariation { get; set; }
    public float EmissiveStrength { get; set; }
    
    // Rocky/Desert
    public int CraterCount { get; set; }
    public int MountainRanges { get; set; }
    public int DunePatterns { get; set; }
    
    // Ice
    public int CrackPatterns { get; set; }
    public bool IceCaps { get; set; }
    
    // Lava
    public int VolcanoCount { get; set; }
    public int LavaFlows { get; set; }
    
    // Ocean/Habitable
    public float OceanCoverage { get; set; }
    public int LandMasses { get; set; }
    public int ContinentCount { get; set; }
    public int ForestRegions { get; set; }
    
    // Gas Giant
    public int BandCount { get; set; }
    public int StormSystems { get; set; }
    
    // Toxic
    public int ToxicClouds { get; set; }
    public int AcidLakes { get; set; }
}

/// <summary>
/// Atmospheric layer data
/// </summary>
public class AtmosphereData
{
    public float InnerRadius { get; set; }
    public float OuterRadius { get; set; }
    public Vector3 Color { get; set; }
    public float Density { get; set; }
    public float ScatteringStrength { get; set; }
}

/// <summary>
/// Planetary ring system
/// </summary>
public class PlanetaryRings
{
    public float InnerRadius { get; set; }
    public float OuterRadius { get; set; }
    public float Thickness { get; set; }
    public int RingCount { get; set; }
    public Vector3 Color { get; set; }
    public float Opacity { get; set; }
    public bool HasGaps { get; set; }
}

/// <summary>
/// Cloud layer data
/// </summary>
public class CloudLayer
{
    public float Radius { get; set; }
    public float Coverage { get; set; }
    public Vector3 CloudColor { get; set; }
    public float Opacity { get; set; }
    public float AnimationSpeed { get; set; }
    public float CloudScale { get; set; }
}

/// <summary>
/// Planet lighting parameters
/// </summary>
public class PlanetLightingData
{
    public Vector3 AmbientColor { get; set; }
    public float SpecularStrength { get; set; }
    public float Roughness { get; set; }
    public Vector3 EmissiveColor { get; set; }
    public float EmissiveStrength { get; set; }
}

/// <summary>
/// Extension methods for Vector3
/// </summary>
public static class Vector3Extensions
{
    public static Vector3 Normalized(this Vector3 v)
    {
        return Vector3.Normalize(v);
    }
}
