using System.Numerics;
using AvorionLike.Core.Voxel;
using AvorionLike.Core.Resources;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Generates voxel-based asteroids with embedded resources
/// </summary>
public class AsteroidVoxelGenerator
{
    private readonly Random _random;
    
    public AsteroidVoxelGenerator(int seed = 0)
    {
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Generate a voxel asteroid with procedural shape and resources
    /// </summary>
    public List<VoxelBlock> GenerateAsteroid(AsteroidData asteroidData, int voxelResolution = 8)
    {
        var blocks = new List<VoxelBlock>();
        float size = asteroidData.Size;
        Vector3 center = asteroidData.Position;
        
        // Generate noise offset for variety
        float noiseOffsetX = (float)_random.NextDouble() * 100f;
        float noiseOffsetY = (float)_random.NextDouble() * 100f;
        float noiseOffsetZ = (float)_random.NextDouble() * 100f;
        
        // Determine voxel size based on asteroid size
        float voxelSize = size / voxelResolution;
        float halfSize = size / 2f;
        
        // Generate voxel grid
        for (int x = 0; x < voxelResolution; x++)
        {
            for (int y = 0; y < voxelResolution; y++)
            {
                for (int z = 0; z < voxelResolution; z++)
                {
                    // Calculate world position
                    Vector3 localPos = new Vector3(
                        x * voxelSize - halfSize,
                        y * voxelSize - halfSize,
                        z * voxelSize - halfSize
                    );
                    Vector3 worldPos = center + localPos;
                    
                    // Use SDF and noise to determine if this voxel should exist
                    float distanceFromCenter = localPos.Length();
                    float baseRadius = size / 2.5f; // Base sphere
                    
                    // Add noise distortion for irregular shape
                    float noiseScale = 0.1f;
                    float noise = NoiseGenerator.FractalNoise3D(
                        (worldPos.X + noiseOffsetX) * noiseScale,
                        (worldPos.Y + noiseOffsetY) * noiseScale,
                        (worldPos.Z + noiseOffsetZ) * noiseScale,
                        octaves: 3,
                        persistence: 0.5f
                    );
                    
                    // Apply noise to radius (creates bumpy surface)
                    float distortedRadius = baseRadius * (1.0f + (noise - 0.5f) * 0.6f);
                    
                    // Check if voxel is inside asteroid
                    if (distanceFromCenter <= distortedRadius)
                    {
                        // Determine material/resource type
                        string material = DetermineMaterial(worldPos, asteroidData.ResourceType, noiseOffsetX);
                        
                        // Create voxel block
                        var block = new VoxelBlock(
                            worldPos,
                            new Vector3(voxelSize, voxelSize, voxelSize),
                            material,
                            BlockType.Hull
                        );
                        
                        blocks.Add(block);
                    }
                }
            }
        }
        
        return blocks;
    }
    
    /// <summary>
    /// Determine material type based on position and primary resource
    /// Creates resource veins within the asteroid
    /// </summary>
    private string DetermineMaterial(Vector3 position, string primaryResource, float seed)
    {
        // Use noise to create resource veins
        float veinNoise = NoiseGenerator.FractalNoise3D(
            (position.X + seed) * 0.05f,
            (position.Y + seed) * 0.05f,
            (position.Z + seed) * 0.05f,
            octaves: 2
        );
        
        // Primary resource appears in veins (60% of asteroid)
        if (veinNoise > 0.3f)
        {
            return primaryResource;
        }
        
        // Secondary/tertiary resources based on additional noise
        float secondaryNoise = NoiseGenerator.PerlinNoise3D(
            position.X * 0.08f,
            position.Y * 0.08f,
            position.Z * 0.08f
        );
        
        // Add variety with secondary resources
        if (secondaryNoise > 0.7f)
        {
            return GetSecondaryResource(primaryResource);
        }
        else if (secondaryNoise > 0.5f)
        {
            return GetTertiaryResource(primaryResource);
        }
        
        // Default to primary resource
        return primaryResource;
    }
    
    /// <summary>
    /// Get a secondary resource type based on primary
    /// </summary>
    private string GetSecondaryResource(string primary)
    {
        return primary switch
        {
            "Iron" => "Titanium",
            "Titanium" => "Naonite",
            "Naonite" => "Trinium",
            "Trinium" => "Xanion",
            "Xanion" => "Ogonite",
            "Ogonite" => "Avorion",
            "Avorion" => "Ogonite",
            _ => "Iron"
        };
    }
    
    /// <summary>
    /// Get a tertiary (common) resource type
    /// </summary>
    private string GetTertiaryResource(string primary)
    {
        // Common resources appear regardless of primary type
        var commonResources = new[] { "Iron", "Titanium" };
        return commonResources[_random.Next(commonResources.Length)];
    }
    
    /// <summary>
    /// Generate a simple spherical asteroid (faster generation)
    /// </summary>
    public List<VoxelBlock> GenerateSimpleAsteroid(Vector3 position, float size, string material, int segments = 6)
    {
        var blocks = new List<VoxelBlock>();
        float voxelSize = size / segments;
        float halfSize = size / 2f;
        float radius = size / 2.2f;
        
        for (int x = 0; x < segments; x++)
        {
            for (int y = 0; y < segments; y++)
            {
                for (int z = 0; z < segments; z++)
                {
                    Vector3 localPos = new Vector3(
                        x * voxelSize - halfSize,
                        y * voxelSize - halfSize,
                        z * voxelSize - halfSize
                    );
                    
                    if (localPos.Length() <= radius)
                    {
                        blocks.Add(new VoxelBlock(
                            position + localPos,
                            new Vector3(voxelSize, voxelSize, voxelSize),
                            material,
                            BlockType.Hull
                        ));
                    }
                }
            }
        }
        
        return blocks;
    }
    
    /// <summary>
    /// Generate asteroid with specific shape using SDF
    /// </summary>
    public List<VoxelBlock> GenerateShapedAsteroid(
        Vector3 position,
        float size,
        string material,
        Func<Vector3, float> sdfFunction,
        int resolution = 8)
    {
        var blocks = new List<VoxelBlock>();
        float voxelSize = size / resolution;
        float halfSize = size / 2f;
        
        for (int x = 0; x < resolution; x++)
        {
            for (int y = 0; y < resolution; y++)
            {
                for (int z = 0; z < resolution; z++)
                {
                    Vector3 localPos = new Vector3(
                        x * voxelSize - halfSize,
                        y * voxelSize - halfSize,
                        z * voxelSize - halfSize
                    );
                    Vector3 worldPos = position + localPos;
                    
                    // Use SDF to determine if voxel exists
                    float sdf = sdfFunction(localPos);
                    if (sdf <= 0)
                    {
                        blocks.Add(new VoxelBlock(
                            worldPos,
                            new Vector3(voxelSize, voxelSize, voxelSize),
                            material,
                            BlockType.Hull
                        ));
                    }
                }
            }
        }
        
        return blocks;
    }
}
