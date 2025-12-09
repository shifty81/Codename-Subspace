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
    /// Generate a voxel asteroid with procedural shape and resources - ENHANCED for realistic rock appearance
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
        
        // Generate voxel grid with more organic, rock-like shapes
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
                    
                    // Use SDF and multi-octave noise for realistic rock shape
                    float distanceFromCenter = localPos.Length();
                    float baseRadius = size / 2.5f; // Base sphere
                    
                    // Multi-scale noise for realistic rocky surface
                    float noiseScale1 = 0.08f;  // Large features (big bumps and depressions)
                    float noise1 = NoiseGenerator.FractalNoise3D(
                        (worldPos.X + noiseOffsetX) * noiseScale1,
                        (worldPos.Y + noiseOffsetY) * noiseScale1,
                        (worldPos.Z + noiseOffsetZ) * noiseScale1,
                        octaves: 4,
                        persistence: 0.6f
                    );
                    
                    float noiseScale2 = 0.15f;  // Medium features
                    float noise2 = NoiseGenerator.FractalNoise3D(
                        (worldPos.X - noiseOffsetX) * noiseScale2,
                        (worldPos.Y - noiseOffsetY) * noiseScale2,
                        (worldPos.Z - noiseOffsetZ) * noiseScale2,
                        octaves: 3,
                        persistence: 0.5f
                    );
                    
                    float noiseScale3 = 0.25f;  // Small surface detail
                    float noise3 = NoiseGenerator.PerlinNoise3D(
                        worldPos.X * noiseScale3,
                        worldPos.Y * noiseScale3,
                        worldPos.Z * noiseScale3
                    );
                    
                    // Combine noise layers for realistic rocky appearance
                    float combinedNoise = noise1 * 0.6f + noise2 * 0.3f + noise3 * 0.1f;
                    
                    // Apply strong distortion for irregular, realistic rock shape
                    float distortedRadius = baseRadius * (1.0f + (combinedNoise - 0.5f) * 0.9f);
                    
                    // Add occasional large protrusions and deep crevices
                    float protrusion = NoiseGenerator.FractalNoise3D(
                        (worldPos.X + noiseOffsetZ) * 0.05f,
                        (worldPos.Y + noiseOffsetZ) * 0.05f,
                        (worldPos.Z + noiseOffsetZ) * 0.05f,
                        octaves: 2
                    );
                    if (protrusion > 0.7f)
                    {
                        distortedRadius *= 1.3f; // Large protrusion
                    }
                    else if (protrusion < 0.3f)
                    {
                        distortedRadius *= 0.85f; // Deep crevice
                    }
                    
                    // Check if voxel is inside asteroid
                    if (distanceFromCenter <= distortedRadius)
                    {
                        // Determine material/resource type with glowing veins
                        string material = DetermineMaterial(worldPos, asteroidData.ResourceType, noiseOffsetX);
                        
                        // Check if this block should be a glowing vein
                        bool isVein = IsVeinLocation(worldPos, asteroidData.ResourceType, noiseOffsetX);
                        
                        // Create voxel block
                        var block = new VoxelBlock(
                            worldPos,
                            new Vector3(voxelSize, voxelSize, voxelSize),
                            isVein ? GetGlowingMaterial(asteroidData.ResourceType) : material,
                            BlockType.Hull
                        );
                        
                        // Apply glowing color to vein blocks
                        if (isVein)
                        {
                            block.ColorRGB = GetResourceColor(asteroidData.ResourceType);
                        }
                        
                        blocks.Add(block);
                    }
                }
            }
        }
        
        return blocks;
    }
    
    /// <summary>
    /// Determine if a location should be part of a glowing resource vein
    /// </summary>
    private bool IsVeinLocation(Vector3 position, string primaryResource, float seed)
    {
        // Use 3D noise to create vein-like patterns
        float veinNoise = NoiseGenerator.FractalNoise3D(
            (position.X + seed) * 0.1f,
            (position.Y + seed) * 0.1f,
            (position.Z + seed) * 0.1f,
            octaves: 3,
            persistence: 0.6f
        );
        
        // Veins appear in concentrated bands
        return veinNoise > 0.6f && veinNoise < 0.75f;
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
    /// Add realistic craters to asteroid surface
    /// </summary>
    private void AddCraters(List<VoxelBlock> blocks, Vector3 asteroidCenter, float asteroidSize)
    {
        if (blocks.Count == 0) return;
        
        int craterCount = 3 + _random.Next(6); // 3-8 craters for more realistic appearance
        
        for (int i = 0; i < craterCount; i++)
        {
            // Pick a random surface point
            var surfaceBlocks = blocks
                .OrderByDescending(b => (b.Position - asteroidCenter).Length())
                .Take(blocks.Count / 3)
                .ToList();
            
            if (surfaceBlocks.Count == 0) continue;
            
            var craterCenter = surfaceBlocks[_random.Next(surfaceBlocks.Count)].Position;
            float craterRadius = asteroidSize * 0.08f + (float)_random.NextDouble() * asteroidSize * 0.18f;
            float craterDepth = craterRadius * 0.4f; // Crater depth relative to radius
            
            // Remove blocks within crater with depth falloff for realistic bowl shape
            blocks.RemoveAll(b => 
            {
                float distanceToCenter = (b.Position - craterCenter).Length();
                float distanceToCore = (b.Position - asteroidCenter).Length();
                
                // Create bowl-shaped crater with smooth falloff
                if (distanceToCenter < craterRadius && distanceToCore > asteroidSize * 0.25f)
                {
                    float depthFactor = 1.0f - (distanceToCenter / craterRadius);
                    float requiredDepth = craterDepth * depthFactor;
                    float actualDepth = distanceToCore - asteroidSize * 0.5f;
                    
                    return actualDepth < requiredDepth;
                }
                return false;
            });
        }
    }
    
    /// <summary>
    /// Add realistic surface details like rock outcroppings, spires, and irregular formations
    /// </summary>
    private void AddSurfaceDetails(List<VoxelBlock> blocks, AsteroidData asteroidData)
    {
        if (blocks.Count == 0) return;
        
        var surfaceBlocks = blocks
            .OrderByDescending(b => (b.Position - asteroidData.Position).Length())
            .Take(blocks.Count / 3)
            .ToList();
        
        int detailCount = 8 + _random.Next(15); // 8-22 details for more realistic rocky surface
        
        for (int i = 0; i < detailCount; i++)
        {
            if (surfaceBlocks.Count == 0) continue;
            
            var baseBlock = surfaceBlocks[_random.Next(surfaceBlocks.Count)];
            Vector3 direction = Vector3.Normalize(baseBlock.Position - asteroidData.Position);
            
            // Random feature type
            int featureType = _random.Next(3);
            
            if (featureType == 0)
            {
                // Sharp spire/needle
                int spireHeight = 2 + _random.Next(4); // 2-5 blocks tall
                for (int j = 0; j < spireHeight; j++)
                {
                    float sizeFactor = 1.0f - (j / (float)spireHeight) * 0.6f; // Taper toward tip
                    var detailSize = new Vector3(
                        0.4f + (float)_random.NextDouble() * 0.3f,
                        0.4f + (float)_random.NextDouble() * 0.3f,
                        0.6f + (float)_random.NextDouble() * 0.4f
                    ) * sizeFactor;
                    
                    var detail = new VoxelBlock(
                        baseBlock.Position + direction * (j + 1) * 0.7f,
                        detailSize,
                        baseBlock.MaterialType,
                        BlockType.Hull
                    );
                    blocks.Add(detail);
                }
            }
            else if (featureType == 1)
            {
                // Boulder/outcropping - wider, flatter
                int boulderLayers = 1 + _random.Next(3); // 1-3 layers
                for (int j = 0; j < boulderLayers; j++)
                {
                    var detailSize = new Vector3(
                        0.8f + (float)_random.NextDouble() * 0.6f,
                        0.4f + (float)_random.NextDouble() * 0.3f,
                        0.8f + (float)_random.NextDouble() * 0.6f
                    );
                    
                    // Random offset for irregular stacking
                    Vector3 offset = new Vector3(
                        ((float)_random.NextDouble() - 0.5f) * 0.4f,
                        ((float)_random.NextDouble() - 0.5f) * 0.4f,
                        0
                    );
                    
                    var detail = new VoxelBlock(
                        baseBlock.Position + direction * (j + 0.5f) * 0.5f + offset,
                        detailSize,
                        baseBlock.MaterialType,
                        BlockType.Hull
                    );
                    blocks.Add(detail);
                }
            }
            else
            {
                // Cluster of small rocks
                int clusterSize = 2 + _random.Next(4); // 2-5 small rocks
                for (int j = 0; j < clusterSize; j++)
                {
                    Vector3 clusterOffset = new Vector3(
                        ((float)_random.NextDouble() - 0.5f) * 1.2f,
                        ((float)_random.NextDouble() - 0.5f) * 1.2f,
                        ((float)_random.NextDouble() - 0.5f) * 1.2f
                    );
                    
                    var detailSize = new Vector3(
                        0.3f + (float)_random.NextDouble() * 0.4f,
                        0.3f + (float)_random.NextDouble() * 0.4f,
                        0.3f + (float)_random.NextDouble() * 0.4f
                    );
                    
                    var detail = new VoxelBlock(
                        baseBlock.Position + direction * 0.6f + clusterOffset,
                        detailSize,
                        baseBlock.MaterialType,
                        BlockType.Hull
                    );
                    blocks.Add(detail);
                }
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
