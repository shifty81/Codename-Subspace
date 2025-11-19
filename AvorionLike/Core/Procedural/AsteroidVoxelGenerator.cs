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
    
    /// <summary>
    /// Generate enhanced asteroid with craters, resource veins, and varied shapes
    /// </summary>
    public List<VoxelBlock> GenerateEnhancedAsteroid(AsteroidData asteroidData, int voxelResolution = 8)
    {
        var blocks = GenerateAsteroid(asteroidData, voxelResolution);
        
        // Add visible resource veins (glowing crystals)
        AddResourceVeins(blocks, asteroidData);
        
        // Add craters for visual interest
        AddCraters(blocks, asteroidData.Position, asteroidData.Size);
        
        // Add surface crystals or outcroppings
        AddSurfaceDetails(blocks, asteroidData);
        
        return blocks;
    }
    
    /// <summary>
    /// Add visible resource veins to asteroids (glowing materials)
    /// ENHANCED: Better color coding and visual distinction
    /// </summary>
    private void AddResourceVeins(List<VoxelBlock> blocks, AsteroidData asteroidData)
    {
        if (blocks.Count == 0) return;
        
        // Calculate vein density based on resource type
        float veinDensity = GetResourceVeinDensity(asteroidData.ResourceType);
        int veinCount = (int)(blocks.Count * veinDensity);
        
        // Get color for this resource type
        uint resourceColor = GetResourceColor(asteroidData.ResourceType);
        
        for (int i = 0; i < veinCount; i++)
        {
            if (_random.NextDouble() > 0.4) continue; // 40% chance per vein (increased from 30%)
            
            var block = blocks[_random.Next(blocks.Count)];
            
            // Make resource blocks glowing with distinct color
            block.MaterialType = GetGlowingMaterial(asteroidData.ResourceType);
            block.ColorRGB = resourceColor;  // Apply resource-specific color
            
            // Add small crystal protrusions for visual interest
            if (_random.NextDouble() < 0.6)  // Increased from 0.5 for more crystals
            {
                var crystalSize = new Vector3(0.6f, 1.2f + (float)_random.NextDouble() * 0.8f, 0.6f);  // Smaller, more refined
                var crystalOffset = new Vector3(
                    (float)(_random.NextDouble() - 0.5) * 2,
                    block.Size.Y / 2 + crystalSize.Y / 2,
                    (float)(_random.NextDouble() - 0.5) * 2
                );
                
                var crystal = new VoxelBlock(
                    block.Position + crystalOffset,
                    crystalSize,
                    GetGlowingMaterial(asteroidData.ResourceType),
                    BlockType.Hull
                );
                crystal.ColorRGB = resourceColor;  // Match vein color
                blocks.Add(crystal);
            }
        }
    }
    
    /// <summary>
    /// Add craters to asteroid surface
    /// </summary>
    private void AddCraters(List<VoxelBlock> blocks, Vector3 asteroidCenter, float asteroidSize)
    {
        if (blocks.Count == 0) return;
        
        int craterCount = 2 + _random.Next(4); // 2-5 craters
        
        for (int i = 0; i < craterCount; i++)
        {
            // Pick a random surface point
            var surfaceBlocks = blocks
                .OrderByDescending(b => (b.Position - asteroidCenter).Length())
                .Take(blocks.Count / 3)
                .ToList();
            
            if (surfaceBlocks.Count == 0) continue;
            
            var craterCenter = surfaceBlocks[_random.Next(surfaceBlocks.Count)].Position;
            float craterRadius = asteroidSize * 0.1f + (float)_random.NextDouble() * asteroidSize * 0.15f;
            
            // Remove blocks within crater radius
            blocks.RemoveAll(b => 
                (b.Position - craterCenter).Length() < craterRadius &&
                (b.Position - asteroidCenter).Length() > asteroidSize * 0.3f // Don't create holes through asteroid
            );
        }
    }
    
    /// <summary>
    /// Add surface details like rock outcroppings
    /// </summary>
    private void AddSurfaceDetails(List<VoxelBlock> blocks, AsteroidData asteroidData)
    {
        if (blocks.Count == 0) return;
        
        var surfaceBlocks = blocks
            .OrderByDescending(b => (b.Position - asteroidData.Position).Length())
            .Take(blocks.Count / 4)
            .ToList();
        
        int detailCount = 5 + _random.Next(10); // 5-14 details
        
        for (int i = 0; i < detailCount; i++)
        {
            if (surfaceBlocks.Count == 0) continue;
            
            var baseBlock = surfaceBlocks[_random.Next(surfaceBlocks.Count)];
            Vector3 direction = Vector3.Normalize(baseBlock.Position - asteroidData.Position);
            
            // Add small rock protrusion
            int protrusions = 1 + _random.Next(3); // 1-3 stacked blocks
            for (int j = 0; j < protrusions; j++)
            {
                var detailSize = new Vector3(
                    0.5f + (float)_random.NextDouble() * 0.5f,
                    0.5f + (float)_random.NextDouble() * 0.5f,
                    0.5f + (float)_random.NextDouble() * 0.5f
                );
                
                var detail = new VoxelBlock(
                    baseBlock.Position + direction * (j + 1) * 0.6f,
                    detailSize,
                    baseBlock.MaterialType,
                    BlockType.Hull
                );
                blocks.Add(detail);
            }
        }
    }
    
    /// <summary>
    /// Get resource vein density based on resource type
    /// </summary>
    private float GetResourceVeinDensity(string resourceType)
    {
        return resourceType switch
        {
            "Avorion" => 0.25f,
            "Ogonite" => 0.20f,
            "Xanion" => 0.18f,
            "Trinium" => 0.15f,
            "Naonite" => 0.12f,
            "Titanium" => 0.10f,
            _ => 0.08f
        };
    }
    
    /// <summary>
    /// Get glowing material name for resource type
    /// </summary>
    private string GetGlowingMaterial(string resourceType)
    {
        return resourceType switch
        {
            "Avorion" => "Avorion",
            "Naonite" => "Naonite",
            "Crystal" => "Crystal",
            _ => resourceType
        };
    }
    
    /// <summary>
    /// Get distinct color for each resource type for better visual distinction
    /// </summary>
    private uint GetResourceColor(string resourceType)
    {
        return resourceType switch
        {
            "Avorion" => 0xFF0000,      // Red
            "Ogonite" => 0xFF8C00,      // Dark Orange
            "Xanion" => 0x00FF00,       // Green
            "Trinium" => 0x1E90FF,      // Dodger Blue
            "Naonite" => 0x9370DB,      // Medium Purple
            "Titanium" => 0xC0C0C0,     // Silver
            "Iron" => 0x808080,         // Gray
            "Crystal" => 0x00FFFF,      // Cyan
            _ => 0xFFFFFF               // White (default)
        };
    }
}
