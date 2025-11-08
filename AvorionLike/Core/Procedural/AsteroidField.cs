using System.Numerics;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Manages asteroid field generation with instancing, LOD, and sparse data structures
/// Optimized for large-scale asteroid fields without generating all asteroids upfront
/// </summary>
public class AsteroidField
{
    public string FieldId { get; private set; }
    public AsteroidBeltData BeltData { get; private set; }
    
    // Spatial partitioning for efficient queries
    private readonly Dictionary<Vector3Int, List<AsteroidInstance>> _spatialHash = new();
    private readonly int _cellSize = 5000; // Size of each spatial hash cell
    
    // Instance management
    private readonly List<AsteroidInstance> _activeInstances = new();
    private readonly int _seed;
    private readonly Random _random;
    
    // LOD thresholds
    public float HighDetailDistance { get; set; } = 1000f;
    public float MediumDetailDistance { get; set; } = 5000f;
    public float LowDetailDistance { get; set; } = 15000f;
    public float CullDistance { get; set; } = 25000f;
    
    public AsteroidField(string fieldId, AsteroidBeltData beltData, int seed)
    {
        FieldId = fieldId;
        BeltData = beltData;
        _seed = seed;
        _random = new Random(seed);
    }
    
    /// <summary>
    /// Get or generate asteroids in a region around the viewer position
    /// Uses lazy generation - only creates asteroids when needed
    /// </summary>
    public List<AsteroidInstance> GetAsteroidsInRegion(Vector3 position, float radius)
    {
        var result = new List<AsteroidInstance>();
        
        // Determine which spatial cells we need to check
        var cells = GetRelevantCells(position, radius);
        
        foreach (var cell in cells)
        {
            if (!_spatialHash.TryGetValue(cell, out var asteroids))
            {
                // Generate asteroids for this cell
                asteroids = GenerateAsteroidsForCell(cell);
                _spatialHash[cell] = asteroids;
            }
            
            // Filter by actual distance
            foreach (var asteroid in asteroids)
            {
                float distance = Vector3.Distance(position, asteroid.Position);
                if (distance <= radius)
                {
                    result.Add(asteroid);
                }
            }
        }
        
        return result;
    }
    
    /// <summary>
    /// Generate asteroids for a specific spatial cell
    /// Deterministic based on cell coordinates and field seed
    /// </summary>
    private List<AsteroidInstance> GenerateAsteroidsForCell(Vector3Int cell)
    {
        var asteroids = new List<AsteroidInstance>();
        
        // Calculate cell center in world space
        Vector3 cellCenter = new Vector3(
            cell.X * _cellSize,
            cell.Y * _cellSize,
            cell.Z * _cellSize
        );
        
        // Check if this cell intersects with the asteroid belt
        if (!BeltData.Contains(cellCenter))
        {
            // Cell might partially overlap, check corners
            bool anyInBelt = false;
            float halfCell = _cellSize / 2f;
            
            for (int x = -1; x <= 1; x += 2)
            {
                for (int y = -1; y <= 1; y += 2)
                {
                    for (int z = -1; z <= 1; z += 2)
                    {
                        var corner = cellCenter + new Vector3(x * halfCell, y * halfCell, z * halfCell);
                        if (BeltData.Contains(corner))
                        {
                            anyInBelt = true;
                            break;
                        }
                    }
                    if (anyInBelt) break;
                }
                if (anyInBelt) break;
            }
            
            if (!anyInBelt)
                return asteroids; // Cell completely outside belt
        }
        
        // Create deterministic random for this cell
        int cellSeed = HashCell(cell, _seed);
        var cellRandom = new Random(cellSeed);
        
        // Calculate expected number of asteroids based on density
        float cellVolume = _cellSize * _cellSize * _cellSize;
        int asteroidCount = (int)(cellVolume * BeltData.Density);
        
        // Add some variation
        asteroidCount += cellRandom.Next(-asteroidCount / 4, asteroidCount / 4);
        asteroidCount = Math.Max(0, asteroidCount);
        
        // Generate asteroids
        for (int i = 0; i < asteroidCount; i++)
        {
            // Random position within cell
            Vector3 position = cellCenter + new Vector3(
                ((float)cellRandom.NextDouble() - 0.5f) * _cellSize,
                ((float)cellRandom.NextDouble() - 0.5f) * _cellSize,
                ((float)cellRandom.NextDouble() - 0.5f) * _cellSize
            );
            
            // Only add if in belt
            if (!BeltData.Contains(position))
                continue;
            
            // Generate asteroid properties
            var asteroid = new AsteroidInstance
            {
                InstanceId = $"{FieldId}-{cell.X}-{cell.Y}-{cell.Z}-{i}",
                Position = position,
                Size = 10f + (float)cellRandom.NextDouble() * 40f,
                Rotation = new Vector3(
                    (float)cellRandom.NextDouble() * MathF.PI * 2,
                    (float)cellRandom.NextDouble() * MathF.PI * 2,
                    (float)cellRandom.NextDouble() * MathF.PI * 2
                ),
                ResourceType = BeltData.PrimaryResource,
                MeshVariant = cellRandom.Next(0, 5), // 5 different mesh variants
                IsGenerated = false
            };
            
            asteroids.Add(asteroid);
        }
        
        return asteroids;
    }
    
    /// <summary>
    /// Update LOD levels for asteroids based on viewer position
    /// </summary>
    public void UpdateLOD(Vector3 viewerPosition, List<AsteroidInstance> asteroids)
    {
        foreach (var asteroid in asteroids)
        {
            float distance = Vector3.Distance(viewerPosition, asteroid.Position);
            
            if (distance < HighDetailDistance)
            {
                asteroid.LODLevel = LODLevel.High;
                asteroid.IsVisible = true;
            }
            else if (distance < MediumDetailDistance)
            {
                asteroid.LODLevel = LODLevel.Medium;
                asteroid.IsVisible = true;
            }
            else if (distance < LowDetailDistance)
            {
                asteroid.LODLevel = LODLevel.Low;
                asteroid.IsVisible = true;
            }
            else if (distance < CullDistance)
            {
                asteroid.LODLevel = LODLevel.Billboard; // Very distant, use billboard
                asteroid.IsVisible = true;
            }
            else
            {
                asteroid.IsVisible = false; // Culled
            }
        }
    }
    
    /// <summary>
    /// Get relevant spatial cells for a position and radius
    /// </summary>
    private List<Vector3Int> GetRelevantCells(Vector3 position, float radius)
    {
        var cells = new List<Vector3Int>();
        
        // Calculate cell range
        int minX = (int)Math.Floor((position.X - radius) / _cellSize);
        int maxX = (int)Math.Ceiling((position.X + radius) / _cellSize);
        int minY = (int)Math.Floor((position.Y - radius) / _cellSize);
        int maxY = (int)Math.Ceiling((position.Y + radius) / _cellSize);
        int minZ = (int)Math.Floor((position.Z - radius) / _cellSize);
        int maxZ = (int)Math.Ceiling((position.Z + radius) / _cellSize);
        
        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                for (int z = minZ; z <= maxZ; z++)
                {
                    cells.Add(new Vector3Int(x, y, z));
                }
            }
        }
        
        return cells;
    }
    
    /// <summary>
    /// Hash cell coordinates for deterministic generation
    /// </summary>
    private int HashCell(Vector3Int cell, int seed)
    {
        unchecked
        {
            int hash = seed;
            hash = hash * 397 ^ cell.X;
            hash = hash * 397 ^ cell.Y;
            hash = hash * 397 ^ cell.Z;
            return hash;
        }
    }
    
    /// <summary>
    /// Get statistics about the field
    /// </summary>
    public AsteroidFieldStats GetStats()
    {
        int totalGenerated = _spatialHash.Values.Sum(list => list.Count);
        int visibleCount = _activeInstances.Count(a => a.IsVisible);
        
        return new AsteroidFieldStats
        {
            FieldId = FieldId,
            TotalGeneratedCells = _spatialHash.Count,
            TotalAsteroids = totalGenerated,
            VisibleAsteroids = visibleCount,
            FieldVolume = CalculateFieldVolume()
        };
    }
    
    /// <summary>
    /// Calculate approximate volume of the asteroid field
    /// </summary>
    private float CalculateFieldVolume()
    {
        float width = BeltData.OuterRadius - BeltData.InnerRadius;
        float avgRadius = (BeltData.InnerRadius + BeltData.OuterRadius) / 2f;
        float circumference = 2 * MathF.PI * avgRadius;
        
        return width * BeltData.Height * circumference;
    }
    
    /// <summary>
    /// Clear generated data (for memory management)
    /// </summary>
    public void ClearDistantCells(Vector3 position, float keepRadius)
    {
        var cellsToRemove = new List<Vector3Int>();
        
        foreach (var kvp in _spatialHash)
        {
            Vector3 cellCenter = new Vector3(
                kvp.Key.X * _cellSize,
                kvp.Key.Y * _cellSize,
                kvp.Key.Z * _cellSize
            );
            
            if (Vector3.Distance(position, cellCenter) > keepRadius)
            {
                cellsToRemove.Add(kvp.Key);
            }
        }
        
        foreach (var cell in cellsToRemove)
        {
            _spatialHash.Remove(cell);
        }
    }
}

/// <summary>
/// Represents a single asteroid instance
/// Used for instanced rendering
/// </summary>
public class AsteroidInstance
{
    public string InstanceId { get; set; } = "";
    public Vector3 Position { get; set; }
    public Vector3 Rotation { get; set; }
    public float Size { get; set; }
    public string ResourceType { get; set; } = "Iron";
    public int MeshVariant { get; set; } // Which pre-made mesh to use
    public LODLevel LODLevel { get; set; } = LODLevel.Medium;
    public bool IsVisible { get; set; } = true;
    public bool IsGenerated { get; set; } = false; // Has voxel data been generated?
    
    // Optional: Full voxel data (only generated when player gets close or wants to mine)
    public List<Vector3>? VoxelPositions { get; set; }
}

/// <summary>
/// Level of detail for asteroid rendering
/// </summary>
public enum LODLevel
{
    High,       // Full detail voxel mesh
    Medium,     // Simplified mesh
    Low,        // Very simple mesh
    Billboard   // 2D sprite
}

/// <summary>
/// Statistics about an asteroid field
/// </summary>
public class AsteroidFieldStats
{
    public string FieldId { get; set; } = "";
    public int TotalGeneratedCells { get; set; }
    public int TotalAsteroids { get; set; }
    public int VisibleAsteroids { get; set; }
    public float FieldVolume { get; set; }
}
