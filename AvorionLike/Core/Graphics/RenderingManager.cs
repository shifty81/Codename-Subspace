using System.Collections.Concurrent;
using AvorionLike.Core.Procedural;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Integrates chunk management with mesh building and rendering
/// </summary>
public class RenderingManager
{
    private readonly ChunkManager _chunkManager;
    private readonly ThreadedMeshBuilder _meshBuilder;
    private readonly ConcurrentDictionary<VoxelChunk, OptimizedMesh> _chunkMeshes = new();
    private bool _useGreedyMeshing = true;
    
    public RenderingManager(ChunkManager chunkManager, int meshThreadCount = 0)
    {
        _chunkManager = chunkManager;
        _meshBuilder = new ThreadedMeshBuilder(meshThreadCount);
        _meshBuilder.Start();
    }
    
    /// <summary>
    /// Update mesh building for dirty chunks
    /// </summary>
    public void Update()
    {
        // Request mesh builds for dirty chunks
        var dirtyChunks = _chunkManager.GetDirtyChunks().ToList();
        foreach (var chunk in dirtyChunks)
        {
            _meshBuilder.RequestMeshBuild(chunk, _useGreedyMeshing);
            // Don't mark clean yet - wait for successful mesh build
        }
        
        // Process completed mesh builds
        var results = _meshBuilder.ProcessResults();
        foreach (var result in results)
        {
            if (result.Success && result.Mesh != null)
            {
                _chunkMeshes[result.Chunk] = result.Mesh;
                _chunkManager.MarkChunkClean(result.Chunk); // Mark clean after success
            }
            // If failed, chunk remains dirty and will be retried
        }
    }
    
    /// <summary>
    /// Get mesh for a chunk
    /// </summary>
    public OptimizedMesh? GetChunkMesh(VoxelChunk chunk)
    {
        return _chunkMeshes.TryGetValue(chunk, out var mesh) ? mesh : null;
    }
    
    /// <summary>
    /// Get all renderable meshes
    /// </summary>
    public IEnumerable<(VoxelChunk Chunk, OptimizedMesh Mesh)> GetRenderableMeshes()
    {
        return _chunkMeshes
            .Where(kvp => kvp.Key.IsLoaded)
            .Select(kvp => (kvp.Key, kvp.Value));
    }
    
    /// <summary>
    /// Enable or disable greedy meshing optimization
    /// </summary>
    public void SetGreedyMeshing(bool enabled)
    {
        _useGreedyMeshing = enabled;
        
        // Mark all chunks as dirty to rebuild with new setting
        foreach (var chunk in _chunkManager.GetLoadedChunks())
        {
            chunk.IsDirty = true;
        }
    }
    
    /// <summary>
    /// Get rendering statistics
    /// </summary>
    public RenderingStats GetStats()
    {
        int totalVertices = 0;
        int totalIndices = 0;
        
        foreach (var mesh in _chunkMeshes.Values)
        {
            totalVertices += mesh.VertexCount;
            totalIndices += mesh.IndexCount;
        }
        
        return new RenderingStats
        {
            ChunksWithMeshes = _chunkMeshes.Count,
            TotalVertices = totalVertices,
            TotalIndices = totalIndices,
            TotalFaces = totalIndices / 3,
            PendingMeshBuilds = _meshBuilder.GetPendingTaskCount(),
            PendingMeshResults = _meshBuilder.GetPendingResultCount(),
            GreedyMeshingEnabled = _useGreedyMeshing
        };
    }
    
    /// <summary>
    /// Shutdown the rendering manager
    /// </summary>
    public void Shutdown()
    {
        _meshBuilder.Stop();
    }
}

/// <summary>
/// Rendering statistics
/// </summary>
public class RenderingStats
{
    public int ChunksWithMeshes { get; set; }
    public int TotalVertices { get; set; }
    public int TotalIndices { get; set; }
    public int TotalFaces { get; set; }
    public int PendingMeshBuilds { get; set; }
    public int PendingMeshResults { get; set; }
    public bool GreedyMeshingEnabled { get; set; }
}
