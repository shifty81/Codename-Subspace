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
    /// Significantly reduces face count by merging adjacent faces of the same material
    /// </summary>
    public static OptimizedMesh BuildGreedyMesh(IEnumerable<VoxelBlock> blocks)
    {
        var mesh = new OptimizedMesh();
        var blockList = blocks.Where(b => !b.IsDestroyed).ToList();
        
        if (blockList.Count == 0)
            return mesh;
        
        // Build voxel grid for greedy meshing
        var grid = BuildVoxelGrid(blockList);
        
        // Process each axis (X, Y, Z) and both directions (positive and negative)
        for (int axis = 0; axis < 3; axis++)
        {
            for (int direction = -1; direction <= 1; direction += 2)
            {
                GreedyMeshAxis(grid, axis, direction, mesh);
            }
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
    /// Handles different block shapes (cube, wedge, corner, etc.)
    /// </summary>
    private static void GenerateBlockFaces(
        VoxelBlock block,
        Dictionary<Vector3, VoxelBlock> blockLookup,
        OptimizedMesh mesh)
    {
        Vector3 pos = block.Position;
        Vector3 size = block.Size;
        uint color = block.ColorRGB;
        float blockType = (float)block.BlockType; // Convert BlockType enum to float
        
        // For shaped blocks, generate special geometry
        if (block.Shape != BlockShape.Cube)
        {
            GenerateShapedBlockFaces(block, blockLookup, mesh);
            return;
        }
        
        // Standard cube face generation with neighbor culling
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
            // Calculate the expected neighbor position based on this block's size
            // For a block at position P with size S, a neighbor in direction D
            // should be at position P + D * S (center to center distance)
            Vector3 dir = directions[i];
            
            // Calculate per-axis offset based on direction
            float offsetX = dir.X != 0 ? size.X : 0;
            float offsetY = dir.Y != 0 ? size.Y : 0;
            float offsetZ = dir.Z != 0 ? size.Z : 0;
            
            Vector3 neighborPos = pos + new Vector3(dir.X * offsetX, dir.Y * offsetY, dir.Z * offsetZ);
            var neighborKey = RoundPosition(neighborPos);
            
            // Check if there's a block at the expected position
            bool hasNeighbor = blockLookup.ContainsKey(neighborKey);
            
            // Only generate face if no neighbor found in this direction
            if (!hasNeighbor)
            {
                AddFace(mesh, pos, size, i, color, blockType);
            }
        }
    }
    
    /// <summary>
    /// Generate faces for shaped blocks (wedges, corners, etc.)
    /// </summary>
    private static void GenerateShapedBlockFaces(
        VoxelBlock block,
        Dictionary<Vector3, VoxelBlock> blockLookup,
        OptimizedMesh mesh)
    {
        Vector3 pos = block.Position;
        Vector3 size = block.Size;
        uint color = block.ColorRGB;
        float blockType = (float)block.BlockType;
        
        switch (block.Shape)
        {
            case BlockShape.Wedge:
                AddWedgeFaces(mesh, pos, size, block.Orientation, color, blockType);
                break;
            case BlockShape.Corner:
                AddCornerFaces(mesh, pos, size, block.Orientation, color, blockType);
                break;
            case BlockShape.InnerCorner:
                AddInnerCornerFaces(mesh, pos, size, block.Orientation, color, blockType);
                break;
            case BlockShape.Tetrahedron:
                AddTetrahedronFaces(mesh, pos, size, block.Orientation, color, blockType);
                break;
            case BlockShape.HalfBlock:
                AddHalfBlockFaces(mesh, pos, size, block.Orientation, color, blockType);
                break;
            default:
                // Fallback to cube
                for (int i = 0; i < 6; i++)
                {
                    AddFace(mesh, pos, size, i, color, blockType);
                }
                break;
        }
    }
    
    /// <summary>
    /// Add wedge block faces - a diagonal slope from one edge to opposite edge
    /// The orientation determines which direction the slope faces
    /// </summary>
    private static void AddWedgeFaces(OptimizedMesh mesh, Vector3 pos, Vector3 size, BlockOrientation orientation, uint color, float blockType)
    {
        Vector3 halfSize = size / 2.0f;
        
        // Wedge vertices depend on orientation
        // For a wedge facing +Z (slope goes up toward +Z):
        // - The back (-Z) has the full height
        // - The front (+Z) is at the bottom
        
        Vector3[] vertices;
        Vector3 normal;
        
        switch (orientation)
        {
            case BlockOrientation.PosZ: // Slope rises toward +Z
                // Add bottom face (full quad)
                AddFace(mesh, pos, size, 3, color, blockType); // Bottom
                
                // Add back face (-Z) - full height
                AddFace(mesh, pos, size, 5, color, blockType); // Back
                
                // Add left triangle face - CCW when viewed from -X toward +X
                // When viewing from -X: +Z is right, -Z is left, +Y is up
                // For PosZ wedge: full height at back (-Z), tapers at front (+Z)
                // CCW: back-bottom -> front-bottom -> back-top
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),  // back-bottom
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),   // front-bottom
                    pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z)    // back-top
                };
                normal = new Vector3(-1, 0, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Add right triangle face - CCW when viewed from +X toward -X
                // When viewing from +X: -Z is right, +Z is left, +Y is up
                // CCW: back-bottom -> front-bottom -> back-top
                vertices = new[]
                {
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),  // back-bottom
                    pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),   // front-bottom
                    pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z)    // back-top
                };
                normal = new Vector3(1, 0, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Add sloped top face (from top-back to bottom-front)
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z)
                };
                normal = Vector3.Normalize(new Vector3(0, size.Z, size.Y));
                AddQuad(mesh, vertices, normal, color, blockType);
                break;
                
            case BlockOrientation.NegZ: // Slope rises toward -Z
                // Add bottom face
                AddFace(mesh, pos, size, 3, color, blockType);
                
                // Add front face (+Z) - full height
                AddFace(mesh, pos, size, 4, color, blockType);
                
                // Add left triangle - CCW when viewed from -X toward +X
                // When viewing from -X: +Z is right, -Z is left, +Y is up
                // For NegZ wedge: full height at front (+Z), tapers at back (-Z)
                // CCW: front-bottom -> back-bottom -> front-top
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),   // front-bottom
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),  // back-bottom
                    pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z)     // front-top
                };
                normal = new Vector3(-1, 0, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Add right triangle - CCW when viewed from +X toward -X
                // When viewing from +X: -Z is right, +Z is left, +Y is up
                // For NegZ wedge: full height at front (+Z), tapers at back (-Z)
                // CCW: front-bottom -> back-bottom -> front-top
                vertices = new[]
                {
                    pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),   // front-bottom
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),  // back-bottom
                    pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z)     // front-top
                };
                normal = new Vector3(1, 0, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Add sloped top
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z),
                    pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z)
                };
                normal = Vector3.Normalize(new Vector3(0, size.Z, -size.Y));
                AddQuad(mesh, vertices, normal, color, blockType);
                break;
                
            case BlockOrientation.PosX: // Slope rises toward +X
                AddFace(mesh, pos, size, 3, color, blockType); // Bottom
                AddFace(mesh, pos, size, 1, color, blockType); // Left (-X) full height
                
                // Front triangle - CCW when viewed from +Z toward -Z
                // Fixed winding order
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
                    pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z)
                };
                normal = new Vector3(0, 0, 1);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Back triangle - CCW when viewed from -Z toward +Z
                // Fixed winding order
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z)
                };
                normal = new Vector3(0, 0, -1);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Sloped face
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z)
                };
                normal = Vector3.Normalize(new Vector3(size.Y, size.X, 0));
                AddQuad(mesh, vertices, normal, color, blockType);
                break;
                
            case BlockOrientation.NegX: // Slope rises toward -X
                AddFace(mesh, pos, size, 3, color, blockType); // Bottom
                AddFace(mesh, pos, size, 0, color, blockType); // Right (+X) full height
                
                // Front triangle - CCW when viewed from +Z toward -Z
                // For NegX wedge: full height at right (+X), tapers at left (-X)
                // CCW from +Z: left-bottom -> right-bottom -> right-top
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),   // left-bottom
                    pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),    // right-bottom
                    pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z)      // right-top
                };
                normal = new Vector3(0, 0, 1);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Back triangle - CCW when viewed from -Z toward +Z
                // For NegX wedge: full height at right (+X), tapers at left (-X)
                // CCW from -Z: right-bottom -> left-bottom -> right-top
                vertices = new[]
                {
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),   // right-bottom
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),  // left-bottom
                    pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z)     // right-top
                };
                normal = new Vector3(0, 0, -1);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Sloped face
                vertices = new[]
                {
                    pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                    pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z)
                };
                normal = Vector3.Normalize(new Vector3(-size.Y, size.X, 0));
                AddQuad(mesh, vertices, normal, color, blockType);
                break;
                
            case BlockOrientation.PosY: // Slope rises toward +Y (default - slope on top)
                AddFace(mesh, pos, size, 3, color, blockType); // Bottom
                AddFace(mesh, pos, size, 5, color, blockType); // Back - full height at low end
                
                // Left side triangle - CCW when viewed from -X toward +X
                // When viewing from -X: +Z is right, -Z is left, +Y is up
                // For PosY wedge: full height at front (+Z), tapers at back (-Z)
                // CCW: front-bottom -> back-bottom -> front-top
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),   // front-bottom
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),  // back-bottom
                    pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z)     // front-top
                };
                normal = new Vector3(-1, 0, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Right side triangle - CCW when viewed from +X toward -X
                // When viewing from +X: -Z is right, +Z is left, +Y is up
                // For PosY wedge: full height at front (+Z), tapers at back (-Z)
                // CCW: front-bottom -> back-bottom -> front-top
                vertices = new[]
                {
                    pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),   // front-bottom
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),  // back-bottom
                    pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z)     // front-top
                };
                normal = new Vector3(1, 0, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Sloped top face
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                    pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z)
                };
                normal = Vector3.Normalize(new Vector3(0, size.Z, size.Y));
                AddQuad(mesh, vertices, normal, color, blockType);
                break;
                
            default: // NegY - Slope rises toward -Y (upside down wedge)
                AddFace(mesh, pos, size, 2, color, blockType); // Top
                AddFace(mesh, pos, size, 5, color, blockType); // Back
                
                // Left triangle - CCW when viewed from -X toward +X
                // Fixed winding order
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
                    pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z)
                };
                normal = new Vector3(-1, 0, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Right triangle - CCW when viewed from +X toward -X
                // Fixed winding order
                vertices = new[]
                {
                    pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
                    pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z)
                };
                normal = new Vector3(1, 0, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Sloped bottom
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z)
                };
                normal = Vector3.Normalize(new Vector3(0, -size.Z, size.Y));
                AddQuad(mesh, vertices, normal, color, blockType);
                break;
        }
    }
    
    /// <summary>
    /// Add a triangle to the mesh
    /// </summary>
    private static void AddTriangle(OptimizedMesh mesh, Vector3[] vertices, Vector3 normal, uint color, float blockType)
    {
        int vertexStart = mesh.Vertices.Count;
        
        foreach (var vertex in vertices)
        {
            mesh.Vertices.Add(vertex);
            mesh.Normals.Add(normal);
            mesh.Colors.Add(color);
            mesh.BlockTypes.Add(blockType);
        }
        
        mesh.Indices.Add(vertexStart + 0);
        mesh.Indices.Add(vertexStart + 1);
        mesh.Indices.Add(vertexStart + 2);
    }
    
    /// <summary>
    /// Add a quad to the mesh
    /// </summary>
    private static void AddQuad(OptimizedMesh mesh, Vector3[] vertices, Vector3 normal, uint color, float blockType)
    {
        int vertexStart = mesh.Vertices.Count;
        
        foreach (var vertex in vertices)
        {
            mesh.Vertices.Add(vertex);
            mesh.Normals.Add(normal);
            mesh.Colors.Add(color);
            mesh.BlockTypes.Add(blockType);
        }
        
        // Two triangles for the quad
        mesh.Indices.Add(vertexStart + 0);
        mesh.Indices.Add(vertexStart + 1);
        mesh.Indices.Add(vertexStart + 2);
        
        mesh.Indices.Add(vertexStart + 0);
        mesh.Indices.Add(vertexStart + 2);
        mesh.Indices.Add(vertexStart + 3);
    }
    
    /// <summary>
    /// Add corner block faces - triangular corner piece
    /// </summary>
    private static void AddCornerFaces(OptimizedMesh mesh, Vector3 pos, Vector3 size, BlockOrientation orientation, uint color, float blockType)
    {
        Vector3 halfSize = size / 2.0f;
        Vector3[] vertices;
        Vector3 normal;
        
        // Corner piece is like a wedge but cut diagonally on two axes
        // Result is a tetrahedron with one corner at full height
        
        switch (orientation)
        {
            case BlockOrientation.PosZ: // Corner at +X, +Y, -Z
            default:
                // Bottom face (triangle) - CCW when viewed from -Y toward +Y
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z)
                };
                normal = new Vector3(0, -1, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Back face (triangle) - CCW when viewed from -Z toward +Z
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z)
                };
                normal = new Vector3(0, 0, -1);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Left face (triangle) - CCW when viewed from -X toward +X
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z)
                };
                normal = new Vector3(-1, 0, 0);
                AddTriangle(mesh, vertices, normal, color, blockType);
                
                // Sloped hypotenuse face (triangle) - CCW when viewed from outside
                vertices = new[]
                {
                    pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                    pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
                    pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z)
                };
                normal = Vector3.Normalize(new Vector3(1, 1, 1));
                AddTriangle(mesh, vertices, normal, color, blockType);
                break;
        }
    }
    
    /// <summary>
    /// Add inner corner block faces
    /// </summary>
    private static void AddInnerCornerFaces(OptimizedMesh mesh, Vector3 pos, Vector3 size, BlockOrientation orientation, uint color, float blockType)
    {
        // Inner corner is a full cube with one corner cut out
        // For simplicity, generate all 6 faces and add the angled inner face
        for (int i = 0; i < 6; i++)
        {
            AddFace(mesh, pos, size, i, color, blockType);
        }
        
        // Add the inner diagonal face (where the corner is cut)
        // The diagonal face has normal pointing toward negative X and negative Z (into the cut corner)
        // CCW winding order when viewed along the normal direction
        Vector3 halfSize = size / 2.0f;
        Vector3[] vertices = new[]
        {
            pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
            pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
            pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z)
        };
        Vector3 normal = Vector3.Normalize(new Vector3(-1, 0, -1));
        AddTriangle(mesh, vertices, normal, color, blockType);
    }
    
    /// <summary>
    /// Add tetrahedron block faces (pyramid shape)
    /// </summary>
    private static void AddTetrahedronFaces(OptimizedMesh mesh, Vector3 pos, Vector3 size, BlockOrientation orientation, uint color, float blockType)
    {
        Vector3 halfSize = size / 2.0f;
        Vector3 apex = pos + new Vector3(0, halfSize.Y, 0); // Top center point
        Vector3[] vertices;
        Vector3 normal;
        
        // Base (bottom) - square - CCW when viewed from -Y toward +Y
        Vector3[] baseVerts = new[]
        {
            pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
            pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
            pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
            pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z)
        };
        normal = new Vector3(0, -1, 0);
        AddQuad(mesh, baseVerts, normal, color, blockType);
        
        // Front face (triangle) - CCW when viewed from +Z toward -Z
        vertices = new[]
        {
            pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
            pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
            apex
        };
        normal = Vector3.Normalize(new Vector3(0, size.Z, size.Y));
        AddTriangle(mesh, vertices, normal, color, blockType);
        
        // Back face (triangle) - CCW when viewed from -Z toward +Z
        vertices = new[]
        {
            pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
            pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
            apex
        };
        normal = Vector3.Normalize(new Vector3(0, size.Z, -size.Y));
        AddTriangle(mesh, vertices, normal, color, blockType);
        
        // Left face (triangle) - CCW when viewed from -X toward +X
        vertices = new[]
        {
            pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
            pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
            apex
        };
        normal = Vector3.Normalize(new Vector3(-size.X, size.Y, 0));
        AddTriangle(mesh, vertices, normal, color, blockType);
        
        // Right face (triangle) - CCW when viewed from +X toward -X
        vertices = new[]
        {
            pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
            pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
            apex
        };
        normal = Vector3.Normalize(new Vector3(size.X, size.Y, 0));
        AddTriangle(mesh, vertices, normal, color, blockType);
    }
    
    /// <summary>
    /// Add half block faces
    /// </summary>
    private static void AddHalfBlockFaces(OptimizedMesh mesh, Vector3 pos, Vector3 size, BlockOrientation orientation, uint color, float blockType)
    {
        Vector3 halfSize = size / 2.0f;
        
        // Half block is sliced in half based on orientation
        // Generate the appropriate half
        Vector3 halfBlockSize;
        Vector3 halfBlockPos;
        
        switch (orientation)
        {
            case BlockOrientation.PosY: // Bottom half
            default:
                // Adjust size to half height
                halfBlockSize = new Vector3(size.X, size.Y / 2, size.Z);
                halfBlockPos = pos - new Vector3(0, size.Y / 4, 0);
                
                // Generate all 6 faces for the half-height block
                for (int i = 0; i < 6; i++)
                {
                    AddFace(mesh, halfBlockPos, halfBlockSize, i, color, blockType);
                }
                break;
            case BlockOrientation.NegY: // Top half
                halfBlockSize = new Vector3(size.X, size.Y / 2, size.Z);
                halfBlockPos = pos + new Vector3(0, size.Y / 4, 0);
                for (int i = 0; i < 6; i++)
                {
                    AddFace(mesh, halfBlockPos, halfBlockSize, i, color, blockType);
                }
                break;
        }
    }
    
    /// <summary>
    /// Add a single face to the mesh
    /// </summary>
    private static void AddFace(OptimizedMesh mesh, Vector3 pos, Vector3 size, int faceIndex, uint color, float blockType)
    {
        Vector3 halfSize = size / 2.0f;
        int vertexStart = mesh.Vertices.Count;
        
        // Define face vertices based on direction
        // All faces use CCW winding order when viewed from outside the cube
        Vector3[] faceVertices = faceIndex switch
        {
            0 => new[] // Right (+X) - CCW when looking from +X towards -X
            {
                pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z),
                pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z)
            },
            1 => new[] // Left (-X) - CCW when looking from -X towards +X
            {
                pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z),
                pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z)
            },
            2 => new[] // Top (+Y) - CCW when looking from +Y towards -Y (looking down)
            {
                pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, -halfSize.Z),
                pos + new Vector3(-halfSize.X, halfSize.Y, -halfSize.Z)
            },
            3 => new[] // Bottom (-Y) - CCW when looking from -Y towards +Y (looking up)
            {
                pos + new Vector3(-halfSize.X, -halfSize.Y, -halfSize.Z),
                pos + new Vector3(halfSize.X, -halfSize.Y, -halfSize.Z),
                pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
                pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z)
            },
            4 => new[] // Front (+Z) - CCW when looking from +Z towards -Z
            {
                pos + new Vector3(-halfSize.X, -halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, -halfSize.Y, halfSize.Z),
                pos + new Vector3(halfSize.X, halfSize.Y, halfSize.Z),
                pos + new Vector3(-halfSize.X, halfSize.Y, halfSize.Z)
            },
            _ => new[] // Back (-Z) - CCW when looking from -Z towards +Z
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
            mesh.BlockTypes.Add(blockType); // Store block type for each vertex
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
    private static void GreedyMeshAxis(VoxelGrid grid, int axis, int direction, OptimizedMesh mesh)
    {
        // Create a 3D voxel array for easier access
        var voxelArray = CreateVoxelArray(grid);
        if (voxelArray == null)
            return;
        
        int width = voxelArray.GetLength(0);
        int height = voxelArray.GetLength(1);
        int depth = voxelArray.GetLength(2);
        
        // Determine dimensions based on axis
        int uSize, vSize, wSize;
        
        switch (axis)
        {
            case 0: // X axis
                uSize = height; vSize = depth; wSize = width;
                break;
            case 1: // Y axis
                uSize = width; vSize = depth; wSize = height;
                break;
            default: // Z axis
                uSize = width; vSize = height; wSize = depth;
                break;
        }
        
        // Process each slice along the axis
        for (int d = 0; d < wSize; d++)
        {
            // Create a mask for this slice
            var mask = new VoxelFace?[uSize, vSize];
            
            // Fill the mask by checking which faces are exposed
            for (int i = 0; i < uSize; i++)
            {
                for (int j = 0; j < vSize; j++)
                {
                    var coords = GetCoords(axis, i, j, d);
                    var block = GetVoxel(voxelArray, coords[0], coords[1], coords[2]);
                    
                    // Check if there's a face here (block exists and neighbor doesn't exist in direction)
                    if (block != null)
                    {
                        var neighborCoords = GetCoords(axis, i, j, d + direction);
                        var neighbor = GetVoxel(voxelArray, neighborCoords[0], neighborCoords[1], neighborCoords[2]);
                        
                        if (neighbor == null)
                        {
                            mask[i, j] = new VoxelFace
                            {
                                Block = block,
                                Color = block.ColorRGB,
                                MaterialType = block.MaterialType
                            };
                        }
                    }
                }
            }
            
            // Generate mesh from mask using greedy algorithm
            for (int i = 0; i < uSize; i++)
            {
                for (int j = 0; j < vSize; j++)
                {
                    if (mask[i, j] == null)
                        continue;
                    
                    var face = mask[i, j]!.Value;
                    
                    // Find width of this quad
                    int width_quad = 1;
                    while (i + width_quad < uSize && 
                           mask[i + width_quad, j] != null &&
                           CompareFaces(mask[i + width_quad, j]!.Value, face))
                    {
                        width_quad++;
                    }
                    
                    // Find height of this quad
                    int height_quad = 1;
                    bool canExtend = true;
                    while (j + height_quad < vSize && canExtend)
                    {
                        // Check entire row
                        for (int k = i; k < i + width_quad; k++)
                        {
                            if (mask[k, j + height_quad] == null ||
                                !CompareFaces(mask[k, j + height_quad]!.Value, face))
                            {
                                canExtend = false;
                                break;
                            }
                        }
                        if (canExtend)
                            height_quad++;
                    }
                    
                    // Add merged quad to mesh
                    AddGreedyQuad(mesh, axis, direction, d, i, j, width_quad, height_quad, face, grid);
                    
                    // Clear processed cells in mask
                    for (int widthIdx = i; widthIdx < i + width_quad; widthIdx++)
                    {
                        for (int heightIdx = j; heightIdx < j + height_quad; heightIdx++)
                        {
                            mask[widthIdx, heightIdx] = null;
                        }
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// Create a 3D array representation of the voxel grid for easier indexing
    /// </summary>
    private static VoxelBlock?[,,]? CreateVoxelArray(VoxelGrid grid)
    {
        if (grid.Blocks.Count == 0)
            return null;
        
        // Find the bounds in voxel units (assuming unit size blocks for simplicity)
        var minX = (int)Math.Floor(grid.Min.X);
        var minY = (int)Math.Floor(grid.Min.Y);
        var minZ = (int)Math.Floor(grid.Min.Z);
        var maxX = (int)Math.Ceiling(grid.Max.X);
        var maxY = (int)Math.Ceiling(grid.Max.Y);
        var maxZ = (int)Math.Ceiling(grid.Max.Z);
        
        int width = maxX - minX + 1;
        int height = maxY - minY + 1;
        int depth = maxZ - minZ + 1;
        
        // Prevent excessive memory allocation
        if (width > 1000 || height > 1000 || depth > 1000)
            return null;
        
        var array = new VoxelBlock?[width, height, depth];
        
        // Fill array with blocks
        foreach (var block in grid.Blocks)
        {
            int x = (int)Math.Round(block.Position.X) - minX;
            int y = (int)Math.Round(block.Position.Y) - minY;
            int z = (int)Math.Round(block.Position.Z) - minZ;
            
            if (x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth)
            {
                array[x, y, z] = block;
            }
        }
        
        return array;
    }
    
    /// <summary>
    /// Get voxel from array with bounds checking
    /// </summary>
    private static VoxelBlock? GetVoxel(VoxelBlock?[,,] array, int x, int y, int z)
    {
        if (x < 0 || x >= array.GetLength(0) ||
            y < 0 || y >= array.GetLength(1) ||
            z < 0 || z >= array.GetLength(2))
        {
            return null;
        }
        
        return array[x, y, z];
    }
    
    /// <summary>
    /// Get coordinates for a given axis, u, v, w indexing
    /// </summary>
    private static int[] GetCoords(int axis, int u, int v, int w)
    {
        return axis switch
        {
            0 => new[] { w, u, v }, // X axis: w=x, u=y, v=z
            1 => new[] { u, w, v }, // Y axis: u=x, w=y, v=z
            _ => new[] { u, v, w }  // Z axis: u=x, v=y, w=z
        };
    }
    
    /// <summary>
    /// Compare two voxel faces for mergeability
    /// </summary>
    private static bool CompareFaces(VoxelFace a, VoxelFace b)
    {
        return a.Color == b.Color && a.MaterialType == b.MaterialType;
    }
    
    /// <summary>
    /// Add a greedy meshed quad to the mesh
    /// </summary>
    private static void AddGreedyQuad(
        OptimizedMesh mesh,
        int axis,
        int direction,
        int depth,
        int u,
        int v,
        int width,
        int height,
        VoxelFace face,
        VoxelGrid grid)
    {
        // Calculate actual world positions
        float minX = grid.Min.X;
        float minY = grid.Min.Y;
        float minZ = grid.Min.Z;
        
        // Convert grid coordinates to world coordinates
        Vector3[] vertices = new Vector3[4];
        Vector3 normal;
        
        // All faces use CCW winding order when viewed from outside the volume
        switch (axis)
        {
            case 0: // X axis
                {
                    float x = minX + depth + (direction > 0 ? 1 : 0);
                    float y1 = minY + u;
                    float y2 = minY + u + height;
                    float z1 = minZ + v;
                    float z2 = minZ + v + width;
                    
                    if (direction > 0) // Right face (+X) - CCW when looking from +X towards -X
                    {
                        vertices[0] = new Vector3(x, y1, z2);
                        vertices[1] = new Vector3(x, y2, z2);
                        vertices[2] = new Vector3(x, y2, z1);
                        vertices[3] = new Vector3(x, y1, z1);
                        normal = new Vector3(1, 0, 0);
                    }
                    else // Left face (-X) - CCW when looking from -X towards +X
                    {
                        vertices[0] = new Vector3(x, y1, z1);
                        vertices[1] = new Vector3(x, y2, z1);
                        vertices[2] = new Vector3(x, y2, z2);
                        vertices[3] = new Vector3(x, y1, z2);
                        normal = new Vector3(-1, 0, 0);
                    }
                    break;
                }
            case 1: // Y axis
                {
                    float y = minY + depth + (direction > 0 ? 1 : 0);
                    float x1 = minX + u;
                    float x2 = minX + u + width;
                    float z1 = minZ + v;
                    float z2 = minZ + v + height;
                    
                    if (direction > 0) // Top face (+Y) - CCW when looking from +Y towards -Y (looking down)
                    {
                        vertices[0] = new Vector3(x1, y, z2);
                        vertices[1] = new Vector3(x2, y, z2);
                        vertices[2] = new Vector3(x2, y, z1);
                        vertices[3] = new Vector3(x1, y, z1);
                        normal = new Vector3(0, 1, 0);
                    }
                    else // Bottom face (-Y) - CCW when looking from -Y towards +Y (looking up)
                    {
                        vertices[0] = new Vector3(x1, y, z1);
                        vertices[1] = new Vector3(x2, y, z1);
                        vertices[2] = new Vector3(x2, y, z2);
                        vertices[3] = new Vector3(x1, y, z2);
                        normal = new Vector3(0, -1, 0);
                    }
                    break;
                }
            default: // Z axis
                {
                    float z = minZ + depth + (direction > 0 ? 1 : 0);
                    float x1 = minX + u;
                    float x2 = minX + u + width;
                    float y1 = minY + v;
                    float y2 = minY + v + height;
                    
                    if (direction > 0) // Front face (+Z) - CCW when looking from +Z towards -Z
                    {
                        // Viewer at +Z, looking at face. X goes right, Y goes up.
                        // CCW from viewer: left-bottom -> right-bottom -> right-top -> left-top
                        vertices[0] = new Vector3(x1, y1, z);
                        vertices[1] = new Vector3(x2, y1, z);
                        vertices[2] = new Vector3(x2, y2, z);
                        vertices[3] = new Vector3(x1, y2, z);
                        normal = new Vector3(0, 0, 1);
                    }
                    else // Back face (-Z) - CCW when looking from -Z towards +Z
                    {
                        // Viewer at -Z, looking at face. X goes left, Y goes up.
                        // CCW from viewer: right-bottom -> left-bottom -> left-top -> right-top
                        vertices[0] = new Vector3(x2, y1, z);
                        vertices[1] = new Vector3(x1, y1, z);
                        vertices[2] = new Vector3(x1, y2, z);
                        vertices[3] = new Vector3(x2, y2, z);
                        normal = new Vector3(0, 0, -1);
                    }
                    break;
                }
        }
        
        // Add vertices to mesh
        int vertexStart = mesh.Vertices.Count;
        foreach (var vertex in vertices)
        {
            mesh.Vertices.Add(vertex);
            mesh.Normals.Add(normal);
            mesh.Colors.Add(face.Color);
        }
        
        // Add indices (two triangles)
        mesh.Indices.Add(vertexStart + 0);
        mesh.Indices.Add(vertexStart + 1);
        mesh.Indices.Add(vertexStart + 2);
        
        mesh.Indices.Add(vertexStart + 0);
        mesh.Indices.Add(vertexStart + 2);
        mesh.Indices.Add(vertexStart + 3);
    }
    
    /// <summary>
    /// Structure representing a voxel face
    /// </summary>
    private struct VoxelFace
    {
        public VoxelBlock Block;
        public uint Color;
        public string MaterialType;
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
    public List<float> BlockTypes { get; set; } = new(); // Block type for each vertex (0=Hull, 1=Armor, 2=Engine, etc.)
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
