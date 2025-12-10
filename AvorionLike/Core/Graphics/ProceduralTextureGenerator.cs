using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Procedural texture generator using noise functions
/// Generates textures at runtime for voxel surfaces
/// </summary>
public class ProceduralTextureGenerator
{
    private readonly int _seed;
    
    public ProceduralTextureGenerator(int seed = 0)
    {
        _seed = seed == 0 ? Environment.TickCount : seed;
    }
    
    /// <summary>
    /// Generate a texture color for a given world position and material
    /// </summary>
    public Vector3 GenerateTextureColor(Vector3 worldPosition, TextureMaterial material, float time = 0f)
    {
        // Base color
        Vector3 color = material.BaseColor;
        
        // Apply pattern
        color = ApplyPattern(worldPosition, color, material);
        
        // Apply noise variation
        color = ApplyNoise(worldPosition, color, material, time);
        
        // Apply wear and tear for industrial materials
        if (material.Pattern == TexturePattern.Weathered)
        {
            color = ApplyWeathering(worldPosition, color, material);
        }
        
        // Clamp to valid range
        color = Vector3.Clamp(color, Vector3.Zero, Vector3.One);
        
        return color;
    }
    
    /// <summary>
    /// Apply texture pattern to base color
    /// </summary>
    private Vector3 ApplyPattern(Vector3 worldPos, Vector3 baseColor, TextureMaterial material)
    {
        float patternValue = 0f;
        
        switch (material.Pattern)
        {
            case TexturePattern.Uniform:
                // No pattern, just base color
                return baseColor;
                
            case TexturePattern.Striped:
                // Horizontal stripes
                patternValue = MathF.Sin(worldPos.Y * material.PatternScale);
                break;
                
            case TexturePattern.Banded:
                // Concentric bands (for gas giants)
                float radius = MathF.Sqrt(worldPos.X * worldPos.X + worldPos.Z * worldPos.Z);
                patternValue = MathF.Sin(radius * material.PatternScale);
                break;
                
            case TexturePattern.Paneled:
                // Hull panels with grid - Enhanced for better visibility
                float gridX = MathF.Abs(worldPos.X % material.PatternScale - material.PatternScale / 2);
                float gridY = MathF.Abs(worldPos.Y % material.PatternScale - material.PatternScale / 2);
                float gridZ = MathF.Abs(worldPos.Z % material.PatternScale - material.PatternScale / 2);
                
                // Panel lines with highlights and shadows
                float lineThickness = 0.15f; // Slightly thicker for visibility
                float edgeHighlight = 0.05f;
                
                if (gridX < lineThickness || gridY < lineThickness || gridZ < lineThickness)
                {
                    // Panel seam - create a groove effect
                    patternValue = -0.4f; // Darker panel lines
                }
                else if (gridX < lineThickness + edgeHighlight || 
                         gridY < lineThickness + edgeHighlight || 
                         gridZ < lineThickness + edgeHighlight)
                {
                    // Edge highlight for depth
                    patternValue = 0.15f;
                }
                else
                {
                    // Panel surface with subtle variation
                    float panelNoise = PerlinNoise3D(worldPos * 2.0f) * 0.1f;
                    patternValue = 0.05f + panelNoise; // Slight variation on panels
                }
                break;
                
            case TexturePattern.Hexagonal:
                // Hexagonal pattern
                patternValue = HexagonalPattern(worldPos, material.PatternScale);
                break;
                
            case TexturePattern.Cracked:
                // Cracked/fractured pattern
                patternValue = CrackedPattern(worldPos, material.PatternScale);
                break;
                
            case TexturePattern.Crystalline:
                // Crystal structure
                patternValue = CrystallinePattern(worldPos, material.PatternScale);
                break;
                
            case TexturePattern.Swirled:
                // Turbulent swirl
                patternValue = SwirlPattern(worldPos, material.PatternScale);
                break;
                
            case TexturePattern.Spotted:
                // Random spots
                patternValue = SpottedPattern(worldPos, material.PatternScale);
                break;
                
            case TexturePattern.Weathered:
                // Weathering handled separately
                return baseColor;
        }
        
        // Blend pattern with base color
        return Vector3.Lerp(baseColor, material.SecondaryColor, (patternValue + 1) * 0.5f);
    }
    
    /// <summary>
    /// Apply procedural noise to color
    /// </summary>
    private Vector3 ApplyNoise(Vector3 worldPos, Vector3 color, TextureMaterial material, float time)
    {
        if (material.NoiseStrength <= 0) return color;
        
        // Apply time offset for animated materials
        Vector3 samplePos = worldPos * material.NoiseScale;
        if (material.Animated)
        {
            samplePos += new Vector3(time * material.AnimationSpeed, time * material.AnimationSpeed * 0.5f, 0);
        }
        
        // Multi-octave Perlin noise
        float noise = 0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float maxValue = 0f;
        
        // 3 octaves
        for (int i = 0; i < 3; i++)
        {
            noise += PerlinNoise3D(samplePos * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }
        
        noise /= maxValue;
        
        // Apply noise to color
        float noiseValue = noise * material.NoiseStrength;
        return color + new Vector3(noiseValue, noiseValue, noiseValue);
    }
    
    /// <summary>
    /// Apply weathering effects (rust, scorch marks, scratches)
    /// </summary>
    private Vector3 ApplyWeathering(Vector3 worldPos, Vector3 color, TextureMaterial material)
    {
        // Rust/scorch marks
        float scorch = PerlinNoise3D(worldPos * 0.5f);
        if (scorch > 0.7f)
        {
            // Dark scorch marks
            Vector3 scorchedColor = new Vector3(0.1f, 0.1f, 0.1f);
            float blend = (scorch - 0.7f) / 0.3f;
            color = Vector3.Lerp(color, scorchedColor, blend * 0.5f);
        }
        
        // Rust stains
        float rust = PerlinNoise3D(worldPos * 1.5f + new Vector3(100, 100, 100));
        if (rust > 0.6f)
        {
            Vector3 rustColor = new Vector3(0.6f, 0.3f, 0.1f); // Brown-orange
            float blend = (rust - 0.6f) / 0.4f;
            color = Vector3.Lerp(color, rustColor, blend * 0.3f);
        }
        
        return color;
    }
    
    /// <summary>
    /// Calculate bump/normal map value for a position
    /// </summary>
    public float CalculateBumpValue(Vector3 worldPos, TextureMaterial material)
    {
        if (material.BumpStrength <= 0) return 0f;
        
        // Use noise to create height variation
        float bump = PerlinNoise3D(worldPos * material.NoiseScale * 2.0f);
        return bump * material.BumpStrength;
    }
    
    // Pattern generation functions
    
    private float HexagonalPattern(Vector3 pos, float scale)
    {
        // Enhanced hexagonal honeycomb pattern for armor plating
        float x = pos.X * scale;
        float y = pos.Y * scale;
        float z = pos.Z * scale;
        
        // Create hexagonal grid using three sine waves at 120 degrees
        float hex1 = MathF.Sin(x);
        float hex2 = MathF.Sin(x * 0.5f + y * 0.866f);
        float hex3 = MathF.Sin(x * 0.5f - y * 0.866f);
        
        float hexPattern = (hex1 + hex2 + hex3) / 3.0f;
        
        // Add depth variation for 3D hexagon cells
        float cellNoise = PerlinNoise3D(new Vector3(x, y, z) * 0.5f);
        
        // Create cell borders (darker lines)
        float cellBorder = MathF.Abs(hexPattern);
        if (cellBorder > 0.7f)
        {
            return -0.5f; // Dark borders for cell separation
        }
        else if (cellBorder > 0.6f)
        {
            return 0.2f; // Highlight edge
        }
        else
        {
            return cellNoise * 0.15f; // Cell interior with subtle variation
        }
    }
    
    private float CrackedPattern(Vector3 pos, float scale)
    {
        // Create crack-like patterns using multiple noise octaves
        float crack1 = PerlinNoise3D(pos * scale * 2.0f);
        float crack2 = PerlinNoise3D(pos * scale * 4.0f + new Vector3(50, 50, 50));
        
        // Sharp cracks
        float cracks = MathF.Abs(crack1) * MathF.Abs(crack2);
        return cracks < 0.1f ? -0.5f : 0.1f;
    }
    
    private float CrystallinePattern(Vector3 pos, float scale)
    {
        // Faceted crystal appearance
        float facet = VoronoiNoise(pos * scale);
        return facet;
    }
    
    private float SwirlPattern(Vector3 pos, float scale)
    {
        // Turbulent swirl using domain warping
        Vector3 q = new Vector3(
            PerlinNoise3D(pos * scale),
            PerlinNoise3D(pos * scale + new Vector3(5.2f, 1.3f, 0)),
            PerlinNoise3D(pos * scale + new Vector3(0, 2.7f, 3.1f))
        );
        
        Vector3 warpedPos = pos + q * 2.0f;
        return PerlinNoise3D(warpedPos * scale);
    }
    
    private float SpottedPattern(Vector3 pos, float scale)
    {
        // Random spots using Worley/cellular noise
        return VoronoiNoise(pos * scale);
    }
    
    // Core noise functions
    
    /// <summary>
    /// 3D Perlin noise implementation
    /// </summary>
    private float PerlinNoise3D(Vector3 pos)
    {
        // Integer coordinates
        int xi = (int)MathF.Floor(pos.X) & 255;
        int yi = (int)MathF.Floor(pos.Y) & 255;
        int zi = (int)MathF.Floor(pos.Z) & 255;
        
        // Fractional coordinates
        float xf = pos.X - MathF.Floor(pos.X);
        float yf = pos.Y - MathF.Floor(pos.Y);
        float zf = pos.Z - MathF.Floor(pos.Z);
        
        // Fade curves
        float u = Fade(xf);
        float v = Fade(yf);
        float w = Fade(zf);
        
        // Hash coordinates of the 8 cube corners
        int a = Hash(xi) + yi;
        int aa = Hash(a) + zi;
        int ab = Hash(a + 1) + zi;
        int b = Hash(xi + 1) + yi;
        int ba = Hash(b) + zi;
        int bb = Hash(b + 1) + zi;
        
        // Blend results from 8 corners
        float x1 = Lerp(Gradient(Hash(aa), xf, yf, zf), Gradient(Hash(ba), xf - 1, yf, zf), u);
        float x2 = Lerp(Gradient(Hash(ab), xf, yf - 1, zf), Gradient(Hash(bb), xf - 1, yf - 1, zf), u);
        float y1 = Lerp(x1, x2, v);
        
        x1 = Lerp(Gradient(Hash(aa + 1), xf, yf, zf - 1), Gradient(Hash(ba + 1), xf - 1, yf, zf - 1), u);
        x2 = Lerp(Gradient(Hash(ab + 1), xf, yf - 1, zf - 1), Gradient(Hash(bb + 1), xf - 1, yf - 1, zf - 1), u);
        float y2 = Lerp(x1, x2, v);
        
        return Lerp(y1, y2, w);
    }
    
    /// <summary>
    /// Voronoi/Worley noise for cellular patterns
    /// </summary>
    private float VoronoiNoise(Vector3 pos)
    {
        // Find integer cell coordinates
        Vector3 cellPos = new Vector3(MathF.Floor(pos.X), MathF.Floor(pos.Y), MathF.Floor(pos.Z));
        
        float minDist = float.MaxValue;
        
        // Check 3x3x3 neighboring cells
        for (int x = -1; x <= 1; x++)
        {
            for (int y = -1; y <= 1; y++)
            {
                for (int z = -1; z <= 1; z++)
                {
                    Vector3 neighborCell = cellPos + new Vector3(x, y, z);
                    
                    // Get random point in cell
                    Vector3 point = neighborCell + RandomVector3(neighborCell);
                    
                    // Distance to point
                    float dist = Vector3.Distance(pos, point);
                    minDist = MathF.Min(minDist, dist);
                }
            }
        }
        
        return 1.0f - minDist;
    }
    
    // Helper functions
    
    private float Fade(float t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }
    
    private float Lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }
    
    private int Hash(int x)
    {
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return x;
    }
    
    private float Gradient(int hash, float x, float y, float z)
    {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
    
    private Vector3 RandomVector3(Vector3 seed)
    {
        int hash = Hash((int)seed.X + _seed) ^ Hash((int)seed.Y + _seed) ^ Hash((int)seed.Z + _seed);
        return new Vector3(
            (hash & 0xFF) / 255.0f,
            ((hash >> 8) & 0xFF) / 255.0f,
            ((hash >> 16) & 0xFF) / 255.0f
        );
    }
}

/// <summary>
/// Manages splatmaps for blending multiple textures on terrain
/// </summary>
public class SplatmapManager
{
    /// <summary>
    /// Calculate blend weights for multiple materials based on altitude and other factors
    /// </summary>
    public Dictionary<MaterialType, float> CalculateBlendWeights(Vector3 worldPos, float altitude, float temperature, float moisture)
    {
        var weights = new Dictionary<MaterialType, float>();
        
        // Altitude-based material selection (for planets)
        if (altitude < -100) // Deep water
        {
            weights[MaterialType.Water] = 1.0f;
        }
        else if (altitude < 0) // Shallow water/coast
        {
            weights[MaterialType.Water] = 0.7f;
            weights[MaterialType.Sand] = 0.3f;
        }
        else if (altitude < 50) // Low elevation
        {
            if (temperature > 0.7f && moisture < 0.3f)
            {
                // Hot and dry = desert
                weights[MaterialType.Sand] = 1.0f;
            }
            else if (moisture > 0.5f)
            {
                // Wet = grass
                weights[MaterialType.Grass] = 0.8f;
                weights[MaterialType.Sand] = 0.2f;
            }
            else
            {
                // Default = mixed
                weights[MaterialType.Grass] = 0.5f;
                weights[MaterialType.Rock] = 0.5f;
            }
        }
        else if (altitude < 200) // Mid elevation
        {
            weights[MaterialType.Rock] = 0.7f;
            weights[MaterialType.Grass] = 0.3f;
        }
        else if (altitude < 400) // High elevation
        {
            weights[MaterialType.Rock] = 0.6f;
            weights[MaterialType.Snow] = 0.4f;
        }
        else // Very high = snow peaks
        {
            weights[MaterialType.Snow] = 1.0f;
        }
        
        // Normalize weights
        float total = 0;
        foreach (var weight in weights.Values)
        {
            total += weight;
        }
        
        if (total > 0)
        {
            var normalizedWeights = new Dictionary<MaterialType, float>();
            foreach (var kvp in weights)
            {
                normalizedWeights[kvp.Key] = kvp.Value / total;
            }
            return normalizedWeights;
        }
        
        return weights;
    }
    
    /// <summary>
    /// Blend multiple material colors based on weights
    /// </summary>
    public Vector3 BlendMaterialColors(Dictionary<MaterialType, float> weights, Vector3 worldPos, ProceduralTextureGenerator generator, float time = 0f)
    {
        Vector3 finalColor = Vector3.Zero;
        
        foreach (var kvp in weights)
        {
            var material = MaterialLibrary.GetMaterial(kvp.Key);
            var color = generator.GenerateTextureColor(worldPos, material, time);
            finalColor += color * kvp.Value;
        }
        
        return finalColor;
    }
}
