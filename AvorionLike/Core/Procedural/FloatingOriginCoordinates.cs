using System.Numerics;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Hierarchical coordinate system for handling vast distances
/// Prevents floating-point precision errors at large scales
/// </summary>
public struct FloatingOriginCoordinates
{
    // Sector coordinates (large-scale grid)
    public Vector3Int Sector { get; set; }
    
    // Local position within sector (relative coordinates)
    public Vector3 LocalPosition { get; set; }
    
    // Sector size in world units
    public const float SectorSize = 100000f; // 100km per sector
    
    public FloatingOriginCoordinates(Vector3Int sector, Vector3 localPosition)
    {
        Sector = sector;
        LocalPosition = localPosition;
    }
    
    /// <summary>
    /// Create from world position
    /// </summary>
    public static FloatingOriginCoordinates FromWorldPosition(Vector3 worldPosition)
    {
        var sector = new Vector3Int(
            (int)Math.Floor(worldPosition.X / SectorSize),
            (int)Math.Floor(worldPosition.Y / SectorSize),
            (int)Math.Floor(worldPosition.Z / SectorSize)
        );
        
        var localPosition = new Vector3(
            worldPosition.X - sector.X * SectorSize,
            worldPosition.Y - sector.Y * SectorSize,
            worldPosition.Z - sector.Z * SectorSize
        );
        
        return new FloatingOriginCoordinates(sector, localPosition);
    }
    
    /// <summary>
    /// Convert to world position (use with caution for large coordinates)
    /// </summary>
    public Vector3 ToWorldPosition()
    {
        return new Vector3(
            Sector.X * SectorSize + LocalPosition.X,
            Sector.Y * SectorSize + LocalPosition.Y,
            Sector.Z * SectorSize + LocalPosition.Z
        );
    }
    
    /// <summary>
    /// Calculate distance to another coordinate
    /// </summary>
    public double DistanceTo(FloatingOriginCoordinates other)
    {
        // If in same sector, use simple distance
        if (Sector.Equals(other.Sector))
        {
            return Vector3.Distance(LocalPosition, other.LocalPosition);
        }
        
        // Calculate sector difference
        var sectorDiff = new Vector3(
            (other.Sector.X - Sector.X) * SectorSize,
            (other.Sector.Y - Sector.Y) * SectorSize,
            (other.Sector.Z - Sector.Z) * SectorSize
        );
        
        // Add local position differences
        var totalDiff = sectorDiff + (other.LocalPosition - LocalPosition);
        return totalDiff.Length();
    }
    
    /// <summary>
    /// Normalize coordinates to keep local position in valid range
    /// </summary>
    public FloatingOriginCoordinates Normalized()
    {
        var coords = this;
        var sector = coords.Sector;
        var localPos = coords.LocalPosition;
        
        // Normalize X
        while (localPos.X >= SectorSize / 2)
        {
            sector = new Vector3Int(sector.X + 1, sector.Y, sector.Z);
            localPos.X -= SectorSize;
        }
        while (localPos.X < -SectorSize / 2)
        {
            sector = new Vector3Int(sector.X - 1, sector.Y, sector.Z);
            localPos.X += SectorSize;
        }
        
        // Normalize Y
        while (localPos.Y >= SectorSize / 2)
        {
            sector = new Vector3Int(sector.X, sector.Y + 1, sector.Z);
            localPos.Y -= SectorSize;
        }
        while (localPos.Y < -SectorSize / 2)
        {
            sector = new Vector3Int(sector.X, sector.Y - 1, sector.Z);
            localPos.Y += SectorSize;
        }
        
        // Normalize Z
        while (localPos.Z >= SectorSize / 2)
        {
            sector = new Vector3Int(sector.X, sector.Y, sector.Z + 1);
            localPos.Z -= SectorSize;
        }
        while (localPos.Z < -SectorSize / 2)
        {
            sector = new Vector3Int(sector.X, sector.Y, sector.Z - 1);
            localPos.Z += SectorSize;
        }
        
        return new FloatingOriginCoordinates(sector, localPos);
    }
}

/// <summary>
/// Integer 3D vector for sector coordinates
/// </summary>
public struct Vector3Int : IEquatable<Vector3Int>
{
    public int X { get; set; }
    public int Y { get; set; }
    public int Z { get; set; }
    
    public Vector3Int(int x, int y, int z)
    {
        X = x;
        Y = y;
        Z = z;
    }
    
    public static Vector3Int Zero => new(0, 0, 0);
    
    public bool Equals(Vector3Int other)
    {
        return X == other.X && Y == other.Y && Z == other.Z;
    }
    
    public override bool Equals(object? obj)
    {
        return obj is Vector3Int other && Equals(other);
    }
    
    public override int GetHashCode()
    {
        return HashCode.Combine(X, Y, Z);
    }
    
    public static bool operator ==(Vector3Int left, Vector3Int right)
    {
        return left.Equals(right);
    }
    
    public static bool operator !=(Vector3Int left, Vector3Int right)
    {
        return !left.Equals(right);
    }
}
