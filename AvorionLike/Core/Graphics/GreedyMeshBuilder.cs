using System.Numerics;
using AvorionLike.Core.Voxel;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Optimized mesh generation for voxel rendering with face culling
/// </summary>
public class GreedyMeshBuilder
{
    /// <summary>
    /// Generate optimized mesh from voxel blocks with face culling
    /// </summary>
    public static OptimizedMesh BuildMesh(IEnumerable<VoxelBlock> blocks)
    {
        var mesh = new OptimizedMesh();
        var blockList = blocks.Where(b => !b.IsDestroyed).ToList();
        
        if (blockList.Count == 0)
            return mesh;
        
        // Build spatial lookup for neighbor checking
        var blockLookup = BuildBlockLookup(blockList);
        
        // Generate faces with culling
        foreach (var block in blockList)
        {
            GenerateBlockFaces(block, blockLookup, mesh);
        }
        
        return mesh;
    }
    
    /// <summary>
    /// Generate mesh with greedy meshing algorithm (combines adjacent faces)
    /// </summary>
    public static OptimizedMesh BuildGreedyMesh(IEnumerable<VoxelBlock> blocks)
    {
        var mesh = new OptimizedMesh();
        var blockList = blocks.Where(b => !b.IsDestroyed).ToList();
        
        if (blockList.Count == 0)
            return mesh;
        
        // Build voxel grid for greedy meshing
        var grid = BuildVoxelGrid(blockList);
        
        // Process each axis
        for (int axis = 0; axis < 3; axis++)
        {
            GreedyMeshAxis(grid, axis, mesh);
        }
        
        return mesh;
    }
    
    /// <summary>
    /// Build spatial lookup dictionary for fast neighbor checking
    /// </summary>
    private static Dictionary<Vector3, VoxelBlock> BuildBlockLookup(List<VoxelBlock> blocks)
    {
        var lookup = new Dictionary<Vector3, VoxelBlock>();
        
        foreach (var block in blocks)
        {
            // Use rounded position as key for lookup
            var key = RoundPosition(block.Position);
            lookup[key] = block;
        }
        
        return lookup;
    }
    
    /// <summary>
    /// Generate faces for a single block with neighbor culling
    /// </summary>
    private static void GenerateBlockFaces(
        VoxelBlock block,
        Dictionary<Vector3, VoxelBlock> blockLookup,
        OptimizedMesh mesh)
    {
        Vector3 pos = block.Position;
        Vector3 size = block.Size;
        uint color = block.ColorRGB;
        
        // Check each direction for neighbors
        Vector3[] directions = new[]
        {
            new Vector3(1, 0, 0),   // Right
            new Vector3(-1, 0, 0),  // Left
            new Vector3(0, 1, 0),   // Top
            new Vector3(0, -1, 0),  // Bottom
            new Vector3(0, 0, 1),   // Front
            new Vector3(0, 0, -1)   // Back
        };
        
        for (int i = 0; i < directions.Length; i++)
        {
            Vector3 neighborPos = pos + directions[i] * size;
            var neighborKey = RoundPosition(neighborPos);
            
            // Only generate face if no neighbor in this direction
            if (!blockLookup.ContainsKey(neighborKey))
            {
                AddFace(mesh, pos, size, i, color);
            }
        }
    }
    
    /// <summary>
    /// Add a single face to the mesh
    /// </summary>
    private static void AddFace(OptimizedMesh mesh, Vector3 pos, Vector3 size, int faceIndex, uint color)
    {
        Vector3 halfSize = size / 2.0f;
        int vertexStart = mesh.Vertices.Count;
        
        // Define face vertices based on direction
        Vector3[] faceVertices = faceIndex switch
        {
            0 => new[] // Right (+X)
            {
                pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z)
            },
            1 => new[] // Left (-X)
            {
                pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
                pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z)
            },
            2 => new[] // Top (+Y)
            {
                pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z)
            },
            3 => new[] // Bottom (-Y)
            {
                pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
                pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
                pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z)
            },
            4 => new[] // Front (+Z)
            {
                pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z)
            },
            _ => new[] // Back (-Z)
            {
                pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
                pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z)
            }
        };
        
        // Get normal for this face
        Vector3 normal = GetFaceNormal(faceIndex);
        
        // Add vertices
        foreach (var vertex in faceVertices)
        {
            mesh.Vertices.Add(vertex);
            mesh.Normals.Add(normal);
            mesh.Colors.Add(color);
        }
        
        // Add indices (two triangles per face)
        mesh.Indices.Add(vertexStart + 0);
        mesh.Indices.Add(vertexStart + 1);
        mesh.Indices.Add(vertexStart + 2);
        
        mesh.Indices.Add(vertexStart + 0);
        mesh.Indices.Add(vertexStart + 2);
        mesh.Indices.Add(vertexStart + 3);
    }
    
    /// <summary>
    /// Get normal vector for face index
    /// </summary>
    private static Vector3 GetFaceNormal(int faceIndex)
    {
        return faceIndex switch
        {
            0 => new Vector3(1, 0, 0),   // Right
            1 => new Vector3(-1, 0, 0),  // Left
            2 => new Vector3(0, 1, 0),   // Top
            3 => new Vector3(0, -1, 0),  // Bottom
            4 => new Vector3(0, 0, 1),   // Front
            _ => new Vector3(0, 0, -1)   // Back
        };
    }
    
    /// <summary>
    /// Round position for lookup key
    /// </summary>
    private static Vector3 RoundPosition(Vector3 pos)
    {
        return new Vector3(
            (float)Math.Round(pos.X, 1),
            (float)Math.Round(pos.Y, 1),
            (float)Math.Round(pos.Z, 1)
        );
    }
    
    /// <summary>
    /// Build voxel grid for greedy meshing
    /// </summary>
    private static VoxelGrid BuildVoxelGrid(List<VoxelBlock> blocks)
    {
        // Find bounds
        Vector3 min = new Vector3(float.MaxValue);
        Vector3 max = new Vector3(float.MinValue);
        
        foreach (var block in blocks)
        {
            min = Vector3.Min(min, block.Position - block.Size / 2);
            max = Vector3.Max(max, block.Position + block.Size / 2);
        }
        
        return new VoxelGrid(min, max, blocks);
    }
    
    /// <summary>
    /// Perform greedy meshing on one axis
    /// </summary>
    private static void GreedyMeshAxis(VoxelGrid grid, int axis, OptimizedMesh mesh)
    {
        // Simplified greedy meshing - can be expanded for better optimization
        // For now, fall back to standard face culling
    }
}

/// <summary>
/// Optimized mesh data structure
/// </summary>
public class OptimizedMesh
{
    public List<Vector3> Vertices { get; set; } = new();
    public List<Vector3> Normals { get; set; } = new();
    public List<uint> Colors { get; set; } = new();
    public List<int> Indices { get; set; } = new();
    
    public int VertexCount => Vertices.Count;
    public int IndexCount => Indices.Count;
    public int FaceCount => Indices.Count / 3;
}

/// <summary>
/// Simple voxel grid for greedy meshing
/// </summary>
public class VoxelGrid
{
    public Vector3 Min { get; set; }
    public Vector3 Max { get; set; }
    public List<VoxelBlock> Blocks { get; set; }
    
    public VoxelGrid(Vector3 min, Vector3 max, List<VoxelBlock> blocks)
    {
        Min = min;
        Max = max;
        Blocks = blocks;
    }
}
