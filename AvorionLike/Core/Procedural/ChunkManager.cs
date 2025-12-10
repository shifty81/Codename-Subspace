using System.Collections.Concurrent;
using System.Numerics;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Axis-aligned bounding box
/// </summary>
public class AABB
{
    public Vector3 Min { get; set; }
    public Vector3 Max { get; set; }
    
    public AABB(Vector3 min, Vector3 max)
    {
        Min = min;
        Max = max;
    }
    
    public Vector3 Center => (Min + Max) / 2.0f;
    public Vector3 Size => Max - Min;
}

/// <summary>
/// Represents a chunk of voxel data
/// </summary>
public class VoxelChunk
{
    public Vector3 ChunkPosition { get; set; }
    public int ChunkSize { get; set; }
    public List<VoxelBlock> Blocks { get; set; } = new();
    public bool IsLoaded { get; set; } = false;
    public bool IsDirty { get; set; } = true; // Needs mesh rebuild
    public DateTime LastAccessTime { get; set; } = DateTime.UtcNow;
    public AABB? BoundingBox { get; set; } = null; // Bounding box for frustum culling
    
    public VoxelChunk(Vector3 position, int size)
    {
        ChunkPosition = position;
        ChunkSize = size;
    }
    
    /// <summary>
    /// Add a voxel block to this chunk
    /// </summary>
    public void AddBlock(VoxelBlock block)
    {
        Blocks.Add(block);
        IsDirty = true;
    }
    
    /// <summary>
    /// Remove a voxel block from this chunk
    /// </summary>
    public bool RemoveBlock(VoxelBlock block)
    {
        bool removed = Blocks.Remove(block);
        if (removed)
            IsDirty = true;
        return removed;
    }
    
    /// <summary>
    /// Get all blocks in this chunk
    /// </summary>
    public IEnumerable<VoxelBlock> GetBlocks()
    {
        LastAccessTime = DateTime.UtcNow;
        return Blocks;
    }
    
    /// <summary>
    /// Update bounding box from block positions
    /// </summary>
    public void UpdateBoundingBox()
    {
        if (Blocks.Count == 0)
        {
            BoundingBox = null;
            return;
        }
        
        Vector3 min = new Vector3(float.MaxValue);
        Vector3 max = new Vector3(float.MinValue);
        
        foreach (var block in Blocks)
        {
            Vector3 halfSize = block.Size / 2.0f;
            Vector3 blockMin = block.Position - halfSize;
            Vector3 blockMax = block.Position + halfSize;
            
            min = Vector3.Min(min, blockMin);
            max = Vector3.Max(max, blockMax);
        }
        
        BoundingBox = new AABB(min, max);
    }
}

/// <summary>
/// Manages dynamic loading and unloading of voxel chunks
/// </summary>
public class ChunkManager
{
    private readonly ConcurrentDictionary<Vector3, VoxelChunk> _chunks = new();
    private readonly int _chunkSize;
    private readonly float _loadRadius;
    private readonly float _unloadRadius;
    private readonly int _maxLoadedChunks;
    
    public ChunkManager(int chunkSize = 100, float loadRadius = 500f, float unloadRadius = 750f, int maxLoadedChunks = 100)
    {
        _chunkSize = chunkSize;
        _loadRadius = loadRadius;
        _unloadRadius = unloadRadius;
        _maxLoadedChunks = maxLoadedChunks;
    }
    
    /// <summary>
    /// Update chunk loading/unloading based on player position
    /// </summary>
    public void Update(Vector3 playerPosition)
    {
        // Find chunks that should be loaded
        var chunksToLoad = GetChunksInRadius(playerPosition, _loadRadius);
        
        // Load new chunks
        foreach (var chunkPos in chunksToLoad)
        {
            if (!_chunks.ContainsKey(chunkPos))
            {
                LoadChunk(chunkPos);
            }
        }
        
        // Unload distant chunks
        var chunksToUnload = _chunks.Keys
            .Where(pos => Vector3.Distance(pos, playerPosition) > _unloadRadius)
            .ToList();
            
        foreach (var chunkPos in chunksToUnload)
        {
            UnloadChunk(chunkPos);
        }
        
        // Enforce max loaded chunks limit
        EnforceChunkLimit();
    }
    
    /// <summary>
    /// Get chunk at world position
    /// </summary>
    public VoxelChunk? GetChunkAtPosition(Vector3 worldPosition)
    {
        Vector3 chunkPos = WorldToChunkPosition(worldPosition);
        return _chunks.TryGetValue(chunkPos, out var chunk) ? chunk : null;
    }
    
    /// <summary>
    /// Add a voxel block to the appropriate chunk
    /// </summary>
    public void AddBlock(VoxelBlock block)
    {
        Vector3 chunkPos = WorldToChunkPosition(block.Position);
        
        if (!_chunks.TryGetValue(chunkPos, out var chunk))
        {
            chunk = new VoxelChunk(chunkPos, _chunkSize);
            _chunks[chunkPos] = chunk;
        }
        
        chunk.AddBlock(block);
    }
    
    /// <summary>
    /// Remove a voxel block
    /// </summary>
    public void RemoveBlock(VoxelBlock block)
    {
        Vector3 chunkPos = WorldToChunkPosition(block.Position);
        
        if (_chunks.TryGetValue(chunkPos, out var chunk))
        {
            chunk.RemoveBlock(block);
        }
    }
    
    /// <summary>
    /// Get all loaded chunks
    /// </summary>
    public IEnumerable<VoxelChunk> GetLoadedChunks()
    {
        return _chunks.Values.Where(c => c.IsLoaded);
    }
    
    /// <summary>
    /// Get all blocks in loaded chunks
    /// </summary>
    public IEnumerable<VoxelBlock> GetAllBlocks()
    {
        return _chunks.Values
            .Where(c => c.IsLoaded)
            .SelectMany(c => c.GetBlocks());
    }
    
    /// <summary>
    /// Get dirty chunks that need mesh rebuilding
    /// </summary>
    public IEnumerable<VoxelChunk> GetDirtyChunks()
    {
        return _chunks.Values.Where(c => c.IsLoaded && c.IsDirty);
    }
    
    /// <summary>
    /// Mark chunk as clean (mesh rebuilt)
    /// </summary>
    public void MarkChunkClean(VoxelChunk chunk)
    {
        chunk.IsDirty = false;
    }
    
    /// <summary>
    /// Load a chunk at the specified position
    /// </summary>
    private void LoadChunk(Vector3 chunkPosition)
    {
        if (_chunks.ContainsKey(chunkPosition))
            return;
            
        var chunk = new VoxelChunk(chunkPosition, _chunkSize);
        chunk.IsLoaded = true;
        _chunks[chunkPosition] = chunk;
    }
    
    /// <summary>
    /// Unload a chunk at the specified position
    /// </summary>
    private void UnloadChunk(Vector3 chunkPosition)
    {
        if (_chunks.TryRemove(chunkPosition, out var chunk))
        {
            chunk.IsLoaded = false;
            chunk.Blocks.Clear();
        }
    }
    
    /// <summary>
    /// Enforce maximum loaded chunks limit by unloading oldest chunks
    /// </summary>
    private void EnforceChunkLimit()
    {
        if (_chunks.Count <= _maxLoadedChunks)
            return;
            
        var oldestChunks = _chunks.Values
            .OrderBy(c => c.LastAccessTime)
            .Take(_chunks.Count - _maxLoadedChunks)
            .Select(c => c.ChunkPosition)
            .ToList();
            
        foreach (var chunkPos in oldestChunks)
        {
            UnloadChunk(chunkPos);
        }
    }
    
    /// <summary>
    /// Get chunk positions within radius of a point
    /// </summary>
    private List<Vector3> GetChunksInRadius(Vector3 center, float radius)
    {
        var chunks = new List<Vector3>();
        int chunkRadius = (int)Math.Ceiling(radius / _chunkSize);
        
        Vector3 centerChunk = WorldToChunkPosition(center);
        
        for (int x = -chunkRadius; x <= chunkRadius; x++)
        {
            for (int y = -chunkRadius; y <= chunkRadius; y++)
            {
                for (int z = -chunkRadius; z <= chunkRadius; z++)
                {
                    Vector3 chunkPos = new Vector3(
                        centerChunk.X + x * _chunkSize,
                        centerChunk.Y + y * _chunkSize,
                        centerChunk.Z + z * _chunkSize
                    );
                    
                    if (Vector3.Distance(center, chunkPos) <= radius)
                    {
                        chunks.Add(chunkPos);
                    }
                }
            }
        }
        
        return chunks;
    }
    
    /// <summary>
    /// Convert world position to chunk position
    /// </summary>
    private Vector3 WorldToChunkPosition(Vector3 worldPosition)
    {
        return new Vector3(
            (float)Math.Floor(worldPosition.X / _chunkSize) * _chunkSize,
            (float)Math.Floor(worldPosition.Y / _chunkSize) * _chunkSize,
            (float)Math.Floor(worldPosition.Z / _chunkSize) * _chunkSize
        );
    }
    
    /// <summary>
    /// Get statistics about chunk management
    /// </summary>
    public ChunkStats GetStats()
    {
        return new ChunkStats
        {
            TotalChunks = _chunks.Count,
            LoadedChunks = _chunks.Values.Count(c => c.IsLoaded),
            DirtyChunks = _chunks.Values.Count(c => c.IsDirty),
            TotalBlocks = _chunks.Values.Sum(c => c.Blocks.Count)
        };
    }
}

/// <summary>
/// Statistics about chunk management
/// </summary>
public class ChunkStats
{
    public int TotalChunks { get; set; }
    public int LoadedChunks { get; set; }
    public int DirtyChunks { get; set; }
    public int TotalBlocks { get; set; }
}
