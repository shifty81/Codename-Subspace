using System.Numerics;

namespace AvorionLike.Core.Procedural;

/// <summary>
/// Noise generation utilities for procedural content
/// </summary>
public static class NoiseGenerator
{
    // Permutation table for Perlin noise
    private static readonly int[] _permutation = new int[512];
    
    static NoiseGenerator()
    {
        // Initialize permutation table (standard Perlin noise permutation)
        int[] p = new int[256];
        for (int i = 0; i < 256; i++)
            p[i] = i;
        
        // Shuffle with fixed seed for consistency
        var rand = new Random(42);
        for (int i = 255; i > 0; i--)
        {
            int j = rand.Next(i + 1);
            (p[i], p[j]) = (p[j], p[i]);
        }
        
        // Duplicate for overflow handling
        for (int i = 0; i < 256; i++)
            _permutation[i] = _permutation[i + 256] = p[i];
    }
    
    /// <summary>
    /// Generate Perlin noise value at given 3D coordinates
    /// </summary>
    public static float PerlinNoise3D(float x, float y, float z)
    {
        // Find unit cube containing point
        int X = (int)Math.Floor(x) & 255;
        int Y = (int)Math.Floor(y) & 255;
        int Z = (int)Math.Floor(z) & 255;
        
        // Find relative position in cube
        x -= (float)Math.Floor(x);
        y -= (float)Math.Floor(y);
        z -= (float)Math.Floor(z);
        
        // Compute fade curves
        float u = Fade(x);
        float v = Fade(y);
        float w = Fade(z);
        
        // Hash coordinates of 8 cube corners
        int A = _permutation[X] + Y;
        int AA = _permutation[A] + Z;
        int AB = _permutation[A + 1] + Z;
        int B = _permutation[X + 1] + Y;
        int BA = _permutation[B] + Z;
        int BB = _permutation[B + 1] + Z;
        
        // Blend results from 8 corners
        float res = Lerp(w,
            Lerp(v,
                Lerp(u, Grad(_permutation[AA], x, y, z),
                       Grad(_permutation[BA], x - 1, y, z)),
                Lerp(u, Grad(_permutation[AB], x, y - 1, z),
                       Grad(_permutation[BB], x - 1, y - 1, z))),
            Lerp(v,
                Lerp(u, Grad(_permutation[AA + 1], x, y, z - 1),
                       Grad(_permutation[BA + 1], x - 1, y, z - 1)),
                Lerp(u, Grad(_permutation[AB + 1], x, y - 1, z - 1),
                       Grad(_permutation[BB + 1], x - 1, y - 1, z - 1))));
        
        return (res + 1.0f) / 2.0f; // Normalize to 0-1
    }
    
    /// <summary>
    /// Generate fractal/octave Perlin noise (multiple frequencies)
    /// </summary>
    public static float FractalNoise3D(float x, float y, float z, int octaves = 4, float persistence = 0.5f)
    {
        float total = 0f;
        float frequency = 1f;
        float amplitude = 1f;
        float maxValue = 0f;
        
        for (int i = 0; i < octaves; i++)
        {
            total += PerlinNoise3D(x * frequency, y * frequency, z * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2f;
        }
        
        return total / maxValue;
    }
    
    /// <summary>
    /// Generate simplex-like noise (simplified implementation)
    /// </summary>
    public static float SimplexNoise3D(float x, float y, float z)
    {
        // Simplified simplex noise - use Perlin as base
        float n0 = PerlinNoise3D(x, y, z);
        float n1 = PerlinNoise3D(x + 1.7f, y + 2.3f, z + 3.1f);
        float n2 = PerlinNoise3D(x + 4.1f, y + 5.7f, z + 6.3f);
        
        return (n0 + n1 + n2) / 3.0f;
    }
    
    /// <summary>
    /// Generate turbulence (absolute value of noise)
    /// </summary>
    public static float Turbulence3D(float x, float y, float z, int octaves = 4)
    {
        float total = 0f;
        float frequency = 1f;
        float amplitude = 1f;
        
        for (int i = 0; i < octaves; i++)
        {
            float noise = PerlinNoise3D(x * frequency, y * frequency, z * frequency);
            total += Math.Abs(noise - 0.5f) * 2.0f * amplitude;
            amplitude *= 0.5f;
            frequency *= 2f;
        }
        
        return total;
    }
    
    /// <summary>
    /// Signed distance field for sphere
    /// </summary>
    public static float SDF_Sphere(Vector3 point, Vector3 center, float radius)
    {
        return Vector3.Distance(point, center) - radius;
    }
    
    /// <summary>
    /// Signed distance field for box
    /// </summary>
    public static float SDF_Box(Vector3 point, Vector3 center, Vector3 size)
    {
        Vector3 d = Vector3.Abs(point - center) - size / 2.0f;
        float outsideDist = Vector3.Max(d, Vector3.Zero).Length();
        float insideDist = Math.Min(Math.Max(d.X, Math.Max(d.Y, d.Z)), 0.0f);
        return outsideDist + insideDist;
    }
    
    /// <summary>
    /// Combine two SDFs with smooth union
    /// </summary>
    public static float SDF_SmoothUnion(float d1, float d2, float k)
    {
        float h = Math.Clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
        return Lerp(d2, d1, h) - k * h * (1.0f - h);
    }
    
    // Helper functions
    private static float Fade(float t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }
    
    private static float Lerp(float t, float a, float b)
    {
        return a + t * (b - a);
    }
    
    private static float Grad(int hash, float x, float y, float z)
    {
        // Convert low 4 bits of hash into gradient direction
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
}
