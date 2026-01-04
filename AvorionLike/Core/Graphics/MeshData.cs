using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Represents mesh data loaded from a 3D model file.
/// Contains vertex positions, normals, texture coordinates, and indices.
/// </summary>
public class MeshData
{
    /// <summary>
    /// Vertex positions (X, Y, Z coordinates)
    /// </summary>
    public Vector3[] Vertices { get; set; } = Array.Empty<Vector3>();
    
    /// <summary>
    /// Vertex normals for lighting calculations
    /// </summary>
    public Vector3[] Normals { get; set; } = Array.Empty<Vector3>();
    
    /// <summary>
    /// Texture coordinates (UV mapping)
    /// </summary>
    public Vector2[] TexCoords { get; set; } = Array.Empty<Vector2>();
    
    /// <summary>
    /// Indices defining triangles (3 indices per triangle)
    /// </summary>
    public uint[] Indices { get; set; } = Array.Empty<uint>();
    
    /// <summary>
    /// Optional vertex colors
    /// </summary>
    public Vector4[]? Colors { get; set; }
    
    /// <summary>
    /// Material name or identifier
    /// </summary>
    public string MaterialName { get; set; } = string.Empty;
    
    /// <summary>
    /// Name of this mesh
    /// </summary>
    public string Name { get; set; } = "Unnamed Mesh";
    
    /// <summary>
    /// Gets the number of vertices in this mesh
    /// </summary>
    public int VertexCount => Vertices.Length;
    
    /// <summary>
    /// Gets the number of triangles in this mesh
    /// </summary>
    public int TriangleCount => Indices.Length / 3;
    
    /// <summary>
    /// Calculates the bounding box of this mesh
    /// </summary>
    public (Vector3 Min, Vector3 Max) GetBounds()
    {
        if (Vertices.Length == 0)
            return (Vector3.Zero, Vector3.Zero);
        
        var min = new Vector3(float.MaxValue);
        var max = new Vector3(float.MinValue);
        
        foreach (var vertex in Vertices)
        {
            min = Vector3.Min(min, vertex);
            max = Vector3.Max(max, vertex);
        }
        
        return (min, max);
    }
    
    /// <summary>
    /// Calculates the center point of the mesh
    /// </summary>
    public Vector3 GetCenter()
    {
        var (min, max) = GetBounds();
        return (min + max) * 0.5f;
    }
    
    /// <summary>
    /// Gets the size of the mesh bounding box
    /// </summary>
    public Vector3 GetSize()
    {
        var (min, max) = GetBounds();
        return max - min;
    }
    
    /// <summary>
    /// Validates that the mesh data is complete and consistent
    /// </summary>
    public bool IsValid(out string errorMessage)
    {
        if (Vertices.Length == 0)
        {
            errorMessage = "Mesh has no vertices";
            return false;
        }
        
        if (Indices.Length == 0)
        {
            errorMessage = "Mesh has no indices";
            return false;
        }
        
        if (Indices.Length % 3 != 0)
        {
            errorMessage = "Index count must be a multiple of 3";
            return false;
        }
        
        if (Normals.Length > 0 && Normals.Length != Vertices.Length)
        {
            errorMessage = "Normal count must match vertex count";
            return false;
        }
        
        if (TexCoords.Length > 0 && TexCoords.Length != Vertices.Length)
        {
            errorMessage = "TexCoord count must match vertex count";
            return false;
        }
        
        if (Colors != null && Colors.Length != Vertices.Length)
        {
            errorMessage = "Color count must match vertex count";
            return false;
        }
        
        // Check that all indices are within bounds
        foreach (var index in Indices)
        {
            if (index >= Vertices.Length)
            {
                errorMessage = $"Index {index} is out of bounds (vertex count: {Vertices.Length})";
                return false;
            }
        }
        
        errorMessage = string.Empty;
        return true;
    }
}
