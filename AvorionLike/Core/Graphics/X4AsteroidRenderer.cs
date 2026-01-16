using System.Numerics;
using AvorionLike.Core.Procedural;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// X4-inspired asteroid renderer with detailed organic shapes
/// Replaces blocky voxel cubes with realistic, irregular asteroid models
/// </summary>
public class X4AsteroidRenderer
{
    private readonly Random _random;
    
    public X4AsteroidRenderer(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate detailed asteroid mesh with organic, irregular shape
    /// X4-style quality with surface details
    /// </summary>
    public DetailedAsteroid GenerateDetailedAsteroid(AsteroidData asteroidData, int detailLevel = 2)
    {
        var asteroid = new DetailedAsteroid
        {
            Data = asteroidData,
            Position = asteroidData.Position,
            Size = asteroidData.Size
        };
        
        // Determine asteroid shape archetype
        asteroid.ShapeType = (AsteroidShapeType)_random.Next((int)AsteroidShapeType.Count);
        
        // Generate base mesh with organic shape
        asteroid.BaseMesh = GenerateOrganicAsteroidMesh(asteroid.Size, asteroid.ShapeType, detailLevel);
        
        // Add surface details (craters, cracks, protrusions)
        asteroid.SurfaceFeatures = GenerateSurfaceFeatures(asteroid.Size, asteroid.ShapeType);
        
        // Determine material appearance
        asteroid.MaterialData = GenerateMaterialData(asteroidData.ResourceType);
        
        // Add optional resource veins visible on surface
        if (!string.IsNullOrEmpty(asteroidData.ResourceType))
        {
            asteroid.ResourceVeins = GenerateResourceVeins(asteroidData.ResourceType);
        }
        
        return asteroid;
    }
    
    /// <summary>
    /// Generate organic asteroid base mesh using deformed sphere/ellipsoid
    /// </summary>
    private AsteroidMesh GenerateOrganicAsteroidMesh(float size, AsteroidShapeType shapeType, int detailLevel)
    {
        var mesh = new AsteroidMesh();
        
        // Base resolution (vertices per axis)
        int resolution = 8 + (detailLevel * 8); // 8, 16, 24 vertices
        
        // Shape parameters based on type
        Vector3 baseScale = GetShapeScale(shapeType);
        
        var vertices = new List<Vector3>();
        var triangles = new List<int>();
        
        // Generate deformed sphere using UV sphere approach
        for (int lat = 0; lat <= resolution; lat++)
        {
            float theta = lat * (float)Math.PI / resolution;
            float sinTheta = (float)Math.Sin(theta);
            float cosTheta = (float)Math.Cos(theta);
            
            for (int lon = 0; lon <= resolution; lon++)
            {
                float phi = lon * 2f * (float)Math.PI / resolution;
                float sinPhi = (float)Math.Sin(phi);
                float cosPhi = (float)Math.Cos(phi);
                
                // Base spherical coordinates
                Vector3 point = new Vector3(
                    cosPhi * sinTheta,
                    cosTheta,
                    sinPhi * sinTheta
                );
                
                // Apply shape scaling
                point *= baseScale;
                
                // Add multi-octave noise for organic irregularity
                float noiseValue = 0f;
                float frequency = 2f;
                float amplitude = 1f;
                
                for (int octave = 0; octave < 4; octave++)
                {
                    noiseValue += NoiseGenerator.PerlinNoise3D(
                        point.X * frequency,
                        point.Y * frequency,
                        point.Z * frequency
                    ) * amplitude;
                    
                    frequency *= 2f;
                    amplitude *= 0.5f;
                }
                
                // Apply noise deformation (10-40% variation based on shape)
                float deformAmount = shapeType switch
                {
                    AsteroidShapeType.VeryIrregular => 0.4f,
                    AsteroidShapeType.Irregular => 0.25f,
                    AsteroidShapeType.Chunky => 0.15f,
                    _ => 0.2f
                };
                
                float radiusVariation = 1f + noiseValue * deformAmount;
                point = Vector3.Normalize(point) * size * radiusVariation;
                
                vertices.Add(point);
            }
        }
        
        // Generate triangle indices
        for (int lat = 0; lat < resolution; lat++)
        {
            for (int lon = 0; lon < resolution; lon++)
            {
                int first = lat * (resolution + 1) + lon;
                int second = first + resolution + 1;
                
                triangles.Add(first);
                triangles.Add(second);
                triangles.Add(first + 1);
                
                triangles.Add(second);
                triangles.Add(second + 1);
                triangles.Add(first + 1);
            }
        }
        
        mesh.Vertices = vertices.ToArray();
        mesh.Triangles = triangles.ToArray();
        mesh.Normals = CalculateNormals(vertices.ToArray(), triangles.ToArray());
        mesh.UVs = GenerateAsteroidUVs(vertices.ToArray());
        
        return mesh;
    }
    
    /// <summary>
    /// Get shape scaling factors for different asteroid types
    /// </summary>
    private Vector3 GetShapeScale(AsteroidShapeType shapeType)
    {
        return shapeType switch
        {
            AsteroidShapeType.Spherical => new Vector3(1.0f, 1.0f, 1.0f),
            AsteroidShapeType.Elongated => new Vector3(0.6f, 0.6f, 1.8f),
            AsteroidShapeType.Flat => new Vector3(1.4f, 0.4f, 1.2f),
            AsteroidShapeType.Chunky => new Vector3(1.2f, 1.4f, 0.9f),
            AsteroidShapeType.Irregular => new Vector3(1.3f, 0.8f, 1.5f),
            AsteroidShapeType.VeryIrregular => new Vector3(1.6f, 0.7f, 1.3f),
            _ => new Vector3(1.0f, 1.0f, 1.0f)
        };
    }
    
    /// <summary>
    /// Calculate vertex normals from triangles
    /// </summary>
    private Vector3[] CalculateNormals(Vector3[] vertices, int[] triangles)
    {
        var normals = new Vector3[vertices.Length];
        
        // Accumulate face normals
        for (int i = 0; i < triangles.Length; i += 3)
        {
            int i1 = triangles[i];
            int i2 = triangles[i + 1];
            int i3 = triangles[i + 2];
            
            Vector3 v1 = vertices[i1];
            Vector3 v2 = vertices[i2];
            Vector3 v3 = vertices[i3];
            
            Vector3 edge1 = v2 - v1;
            Vector3 edge2 = v3 - v1;
            Vector3 faceNormal = Vector3.Cross(edge1, edge2);
            
            normals[i1] += faceNormal;
            normals[i2] += faceNormal;
            normals[i3] += faceNormal;
        }
        
        // Normalize
        for (int i = 0; i < normals.Length; i++)
        {
            normals[i] = Vector3.Normalize(normals[i]);
        }
        
        return normals;
    }
    
    /// <summary>
    /// Generate UV coordinates for asteroid texture mapping
    /// </summary>
    private Vector2[] GenerateAsteroidUVs(Vector3[] vertices)
    {
        var uvs = new Vector2[vertices.Length];
        
        for (int i = 0; i < vertices.Length; i++)
        {
            Vector3 v = Vector3.Normalize(vertices[i]);
            
            // Spherical UV mapping
            float u = 0.5f + (float)Math.Atan2(v.Z, v.X) / (2f * (float)Math.PI);
            float vCoord = 0.5f - (float)Math.Asin(v.Y) / (float)Math.PI;
            
            uvs[i] = new Vector2(u, vCoord);
        }
        
        return uvs;
    }
    
    /// <summary>
    /// Generate surface features (craters, cracks, protrusions)
    /// </summary>
    private List<SurfaceFeature> GenerateSurfaceFeatures(float size, AsteroidShapeType shapeType)
    {
        var features = new List<SurfaceFeature>();
        
        // Number of features based on size and shape
        int featureCount = (int)(size * 0.5f) + _random.Next(5, 15);
        
        for (int i = 0; i < featureCount; i++)
        {
            var featureType = (SurfaceFeatureType)_random.Next((int)SurfaceFeatureType.Count);
            
            // Random position on surface (spherical coordinates)
            float theta = (float)(_random.NextDouble() * Math.PI);
            float phi = (float)(_random.NextDouble() * 2 * Math.PI);
            
            Vector3 position = new Vector3(
                (float)(Math.Cos(phi) * Math.Sin(theta)),
                (float)Math.Cos(theta),
                (float)(Math.Sin(phi) * Math.Sin(theta))
            ) * size;
            
            features.Add(new SurfaceFeature
            {
                Type = featureType,
                Position = position,
                Scale = size * (0.05f + (float)_random.NextDouble() * 0.15f),
                Depth = featureType == SurfaceFeatureType.Crater ? 
                    size * (0.02f + (float)_random.NextDouble() * 0.08f) : 0f,
                Rotation = (float)(_random.NextDouble() * 360f)
            });
        }
        
        return features;
    }
    
    /// <summary>
    /// Generate material appearance data based on resource type
    /// </summary>
    private AsteroidMaterialData GenerateMaterialData(string resourceType)
    {
        var material = new AsteroidMaterialData();
        
        switch (resourceType?.ToLower())
        {
            case "iron":
                material.BaseColor = new Vector3(0.4f, 0.35f, 0.3f);
                material.Metallic = 0.6f;
                material.Roughness = 0.8f;
                material.SpecularColor = new Vector3(0.5f, 0.45f, 0.4f);
                break;
                
            case "titanium":
                material.BaseColor = new Vector3(0.5f, 0.5f, 0.55f);
                material.Metallic = 0.7f;
                material.Roughness = 0.6f;
                material.SpecularColor = new Vector3(0.6f, 0.6f, 0.65f);
                break;
                
            case "naonite":
                material.BaseColor = new Vector3(0.3f, 0.5f, 0.4f);
                material.Metallic = 0.5f;
                material.Roughness = 0.7f;
                material.SpecularColor = new Vector3(0.4f, 0.6f, 0.5f);
                material.EmissiveStrength = 0.1f;
                break;
                
            case "trinium":
                material.BaseColor = new Vector3(0.4f, 0.5f, 0.6f);
                material.Metallic = 0.8f;
                material.Roughness = 0.5f;
                material.SpecularColor = new Vector3(0.5f, 0.6f, 0.7f);
                material.EmissiveStrength = 0.15f;
                break;
                
            case "xanion":
                material.BaseColor = new Vector3(0.6f, 0.4f, 0.5f);
                material.Metallic = 0.7f;
                material.Roughness = 0.4f;
                material.SpecularColor = new Vector3(0.7f, 0.5f, 0.6f);
                material.EmissiveStrength = 0.2f;
                break;
                
            case "ogonite":
                material.BaseColor = new Vector3(0.5f, 0.6f, 0.3f);
                material.Metallic = 0.9f;
                material.Roughness = 0.3f;
                material.SpecularColor = new Vector3(0.6f, 0.7f, 0.4f);
                material.EmissiveStrength = 0.25f;
                break;
                
            case "avorion":
                material.BaseColor = new Vector3(0.7f, 0.3f, 0.3f);
                material.Metallic = 0.95f;
                material.Roughness = 0.2f;
                material.SpecularColor = new Vector3(0.8f, 0.4f, 0.4f);
                material.EmissiveStrength = 0.3f;
                break;
                
            default: // Generic rock
                material.BaseColor = new Vector3(0.5f, 0.45f, 0.4f);
                material.Metallic = 0.1f;
                material.Roughness = 0.9f;
                material.SpecularColor = new Vector3(0.3f, 0.3f, 0.3f);
                break;
        }
        
        return material;
    }
    
    /// <summary>
    /// Generate visible resource veins on asteroid surface
    /// </summary>
    private List<ResourceVein> GenerateResourceVeins(string resourceType)
    {
        var veins = new List<ResourceVein>();
        int veinCount = 3 + _random.Next(5);
        
        // Vein color based on resource
        Vector3 veinColor = resourceType?.ToLower() switch
        {
            "iron" => new Vector3(0.6f, 0.5f, 0.4f),
            "titanium" => new Vector3(0.7f, 0.7f, 0.75f),
            "naonite" => new Vector3(0.4f, 0.7f, 0.5f),
            "trinium" => new Vector3(0.5f, 0.7f, 0.8f),
            "xanion" => new Vector3(0.8f, 0.5f, 0.7f),
            "ogonite" => new Vector3(0.7f, 0.8f, 0.4f),
            "avorion" => new Vector3(0.9f, 0.4f, 0.4f),
            _ => new Vector3(0.6f, 0.6f, 0.6f)
        };
        
        for (int i = 0; i < veinCount; i++)
        {
            veins.Add(new ResourceVein
            {
                StartPoint = RandomSurfacePoint(),
                EndPoint = RandomSurfacePoint(),
                Width = 0.5f + (float)_random.NextDouble() * 1.5f,
                Color = veinColor,
                Intensity = 0.3f + (float)_random.NextDouble() * 0.4f,
                Glow = 0.2f + (float)_random.NextDouble() * 0.3f
            });
        }
        
        return veins;
    }
    
    /// <summary>
    /// Generate random point on unit sphere surface
    /// </summary>
    private Vector3 RandomSurfacePoint()
    {
        float theta = (float)(_random.NextDouble() * Math.PI);
        float phi = (float)(_random.NextDouble() * 2 * Math.PI);
        
        return new Vector3(
            (float)(Math.Cos(phi) * Math.Sin(theta)),
            (float)Math.Cos(theta),
            (float)(Math.Sin(phi) * Math.Sin(theta))
        );
    }
}

/// <summary>
/// Detailed asteroid with full rendering data
/// </summary>
public class DetailedAsteroid
{
    public AsteroidData Data { get; set; }
    public Vector3 Position { get; set; }
    public float Size { get; set; }
    public AsteroidShapeType ShapeType { get; set; }
    public AsteroidMesh BaseMesh { get; set; } = null!;
    public List<SurfaceFeature> SurfaceFeatures { get; set; } = new();
    public AsteroidMaterialData MaterialData { get; set; } = null!;
    public List<ResourceVein>? ResourceVeins { get; set; }
}

/// <summary>
/// Asteroid mesh data
/// </summary>
public class AsteroidMesh
{
    public Vector3[] Vertices { get; set; } = Array.Empty<Vector3>();
    public int[] Triangles { get; set; } = Array.Empty<int>();
    public Vector3[] Normals { get; set; } = Array.Empty<Vector3>();
    public Vector2[] UVs { get; set; } = Array.Empty<Vector2>();
}

/// <summary>
/// Asteroid shape archetypes
/// </summary>
public enum AsteroidShapeType
{
    Spherical,      // Round
    Elongated,      // Cigar/needle shape
    Flat,           // Pancake/disc
    Chunky,         // Potato-like
    Irregular,      // Moderately irregular
    VeryIrregular,  // Highly deformed
    Count
}

/// <summary>
/// Surface feature (craters, cracks, protrusions)
/// </summary>
public class SurfaceFeature
{
    public SurfaceFeatureType Type { get; set; }
    public Vector3 Position { get; set; }
    public float Scale { get; set; }
    public float Depth { get; set; }
    public float Rotation { get; set; }
}

/// <summary>
/// Surface feature types
/// </summary>
public enum SurfaceFeatureType
{
    Crater,
    Crack,
    Boulder,
    Ridge,
    Depression,
    Count
}

/// <summary>
/// Asteroid material appearance
/// </summary>
public class AsteroidMaterialData
{
    public Vector3 BaseColor { get; set; }
    public float Metallic { get; set; }
    public float Roughness { get; set; }
    public Vector3 SpecularColor { get; set; }
    public float EmissiveStrength { get; set; }
}

/// <summary>
/// Resource vein visible on surface
/// </summary>
public class ResourceVein
{
    public Vector3 StartPoint { get; set; }
    public Vector3 EndPoint { get; set; }
    public float Width { get; set; }
    public Vector3 Color { get; set; }
    public float Intensity { get; set; }
    public float Glow { get; set; }
}
