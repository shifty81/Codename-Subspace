using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Enhanced procedural texture generator with more variety and visual interest
/// Implements panel lines, greebling, wear patterns, and faction-specific styles
/// </summary>
public class EnhancedTextureGenerator
{
    private readonly int _seed;
    private readonly Random _random;
    
    // Constants for noise and pattern generation
    private const float NoiseScale = 1000f;
    private const int HashMask = 0xFFFF;
    private const float LightSpacing = 20f;
    private const float LightSize = 0.5f;
    private const float LightBlinkThreshold = 0.7f;
    
    /// <summary>
    /// Texture style presets for different factions/themes
    /// </summary>
    public enum TextureStyle
    {
        Clean,           // Factory-fresh, minimal wear
        Military,        // Camo patterns, armor plates, warning stripes
        Industrial,      // Dirty, oily, rust stains, exposed rivets
        Sleek,           // High-tech panels, glowing accents, minimal seams
        Pirate,          // Heavy wear, patches, mismatched panels
        Ancient,         // Geometric patterns, mysterious glyphs
        Organic,         // Biomechanical, veins, pulsing patterns
        Crystalline      // Faceted surfaces, prismatic colors
    }
    
    public EnhancedTextureGenerator(int seed = 0)
    {
        _seed = seed == 0 ? Environment.TickCount : seed;
        _random = new Random(_seed);
    }
    
    /// <summary>
    /// Generate an enhanced texture color for a given world position
    /// Includes panel lines, greebles, wear, and style-specific effects
    /// </summary>
    public Vector3 GenerateEnhancedColor(Vector3 worldPosition, Vector3 baseColor, TextureStyle style, float time = 0f)
    {
        Vector3 color = baseColor;
        
        // Apply base style pattern
        color = ApplyStylePattern(worldPosition, color, style, time);
        
        // Add panel lines
        color = ApplyPanelLines(worldPosition, color, style);
        
        // Add greebling (surface detail)
        color = ApplyGreebling(worldPosition, color, style);
        
        // Add wear/weathering
        color = ApplyWear(worldPosition, color, style);
        
        // Add emissive accents for certain styles
        color = ApplyEmissiveAccents(worldPosition, color, style, time);
        
        return Vector3.Clamp(color, Vector3.Zero, new Vector3(2f)); // Allow overbright for emissive
    }
    
    /// <summary>
    /// Apply style-specific base pattern
    /// </summary>
    private Vector3 ApplyStylePattern(Vector3 pos, Vector3 color, TextureStyle style, float time)
    {
        switch (style)
        {
            case TextureStyle.Military:
                // Camo-like pattern using multiple noise octaves
                float camo1 = PerlinNoise3D(pos * 0.5f);
                float camo2 = PerlinNoise3D(pos * 1.0f + new Vector3(50, 50, 50));
                float camo3 = PerlinNoise3D(pos * 2.0f + new Vector3(100, 100, 100));
                
                float camoBlend = (camo1 + camo2 * 0.5f + camo3 * 0.25f) / 1.75f;
                
                if (camoBlend > 0.3f)
                {
                    color *= 0.85f; // Slightly darker variation
                }
                else if (camoBlend < -0.2f)
                {
                    color *= 1.1f; // Slightly lighter variation
                }
                break;
                
            case TextureStyle.Industrial:
                // Dirty/oily variation
                float dirt = PerlinNoise3D(pos * 2.0f) * 0.3f;
                color = Vector3.Lerp(color, new Vector3(0.3f, 0.25f, 0.2f), Math.Clamp(dirt + 0.1f, 0f, 0.3f));
                
                // Oil stains
                float oil = VoronoiNoise(pos * 0.5f);
                if (oil > 0.7f)
                {
                    color = Vector3.Lerp(color, new Vector3(0.1f, 0.08f, 0.05f), 0.5f);
                }
                break;
                
            case TextureStyle.Sleek:
                // Subtle gradient along surface
                float gradient = MathF.Sin(pos.Y * 0.3f + pos.Z * 0.1f) * 0.5f + 0.5f;
                color = Vector3.Lerp(color * 0.9f, color * 1.1f, gradient * 0.15f);
                
                // Subtle metallic flake
                float flake = HighFrequencyNoise(pos * 10f) * 0.05f;
                color += new Vector3(flake);
                break;
                
            case TextureStyle.Pirate:
                // Patched/mismatched panels
                float patch = VoronoiNoise(pos * 0.3f);
                if (patch > 0.6f)
                {
                    // Different colored patch
                    float hueShift = PerlinNoise3D(pos * 0.2f) * 0.3f;
                    color = new Vector3(
                        color.X + hueShift,
                        color.Y - hueShift * 0.5f,
                        color.Z - hueShift * 0.3f
                    );
                }
                break;
                
            case TextureStyle.Ancient:
                // Geometric glyph patterns
                float glyph = GeometricPattern(pos, 4f);
                if (glyph > 0.7f)
                {
                    color = Vector3.Lerp(color, new Vector3(1.0f, 0.8f, 0.3f), 0.3f); // Gold accents
                }
                break;
                
            case TextureStyle.Organic:
                // Pulsing vein patterns
                float vein = VeinPattern(pos, time);
                color = Vector3.Lerp(color, new Vector3(0.8f, 0.2f, 0.5f), vein * 0.4f);
                break;
                
            case TextureStyle.Crystalline:
                // Prismatic color shifting
                float prism = CrystallinePattern(pos);
                Vector3 prismaticColor = new Vector3(
                    MathF.Sin(prism * 6.28f) * 0.5f + 0.5f,
                    MathF.Sin(prism * 6.28f + 2.09f) * 0.5f + 0.5f,
                    MathF.Sin(prism * 6.28f + 4.19f) * 0.5f + 0.5f
                );
                color = Vector3.Lerp(color, prismaticColor, 0.2f);
                break;
        }
        
        return color;
    }
    
    /// <summary>
    /// Apply panel lines - thin dark lines that separate hull sections
    /// </summary>
    private Vector3 ApplyPanelLines(Vector3 pos, Vector3 color, TextureStyle style)
    {
        float panelScale = style switch
        {
            TextureStyle.Sleek => 6f,        // Larger, cleaner panels
            TextureStyle.Industrial => 3f,   // Smaller, busy panels
            TextureStyle.Military => 4f,     // Medium panels
            TextureStyle.Pirate => 2.5f,     // Irregular small panels
            _ => 4f
        };
        
        float lineThickness = style == TextureStyle.Sleek ? 0.04f : 0.08f;
        
        // Grid panel lines
        float gridX = MathF.Abs((pos.X % panelScale) / panelScale - 0.5f);
        float gridY = MathF.Abs((pos.Y % panelScale) / panelScale - 0.5f);
        float gridZ = MathF.Abs((pos.Z % panelScale) / panelScale - 0.5f);
        
        // Determine if we're on a panel line
        bool onLine = gridX < lineThickness || gridY < lineThickness || gridZ < lineThickness;
        
        if (onLine)
        {
            // Darken for panel lines
            float darken = style == TextureStyle.Sleek ? 0.15f : 0.25f;
            color *= (1f - darken);
            
            // Industrial style adds rivets at intersections
            if (style == TextureStyle.Industrial)
            {
                if (gridX < lineThickness * 2 && gridY < lineThickness * 2)
                {
                    // Rivet/bolt
                    color *= 0.7f;
                }
            }
        }
        
        return color;
    }
    
    /// <summary>
    /// Apply greebling - small surface details that add visual complexity
    /// </summary>
    private Vector3 ApplyGreebling(Vector3 pos, Vector3 color, TextureStyle style)
    {
        if (style == TextureStyle.Sleek || style == TextureStyle.Clean)
        {
            return color; // Minimal greebling for clean styles
        }
        
        float greebleIntensity = style switch
        {
            TextureStyle.Industrial => 0.3f,
            TextureStyle.Military => 0.2f,
            TextureStyle.Pirate => 0.35f,
            TextureStyle.Ancient => 0.15f,
            _ => 0.1f
        };
        
        // Multiple scales of detail
        float detail1 = HighFrequencyNoise(pos * 3f);
        float detail2 = HighFrequencyNoise(pos * 7f);
        float detail3 = HighFrequencyNoise(pos * 15f);
        
        float greeble = (detail1 * 0.5f + detail2 * 0.3f + detail3 * 0.2f) * greebleIntensity;
        
        // Raised/recessed panels
        if (greeble > 0.1f)
        {
            color *= (1f + greeble * 0.3f); // Raised = slightly brighter
        }
        else if (greeble < -0.1f)
        {
            color *= (1f + greeble * 0.5f); // Recessed = slightly darker
        }
        
        // Add small vents/grilles pattern
        if (style == TextureStyle.Industrial || style == TextureStyle.Military)
        {
            float ventPattern = VentPattern(pos);
            if (ventPattern > 0.8f)
            {
                color *= 0.5f; // Dark vent openings
            }
        }
        
        return color;
    }
    
    /// <summary>
    /// Apply wear and weathering effects
    /// </summary>
    private Vector3 ApplyWear(Vector3 pos, Vector3 color, TextureStyle style)
    {
        float wearIntensity = style switch
        {
            TextureStyle.Clean => 0.02f,
            TextureStyle.Sleek => 0.05f,
            TextureStyle.Military => 0.15f,
            TextureStyle.Industrial => 0.25f,
            TextureStyle.Pirate => 0.4f,
            TextureStyle.Ancient => 0.2f,
            _ => 0.1f
        };
        
        // Edge wear (brighter edges)
        float edgeWear = EdgeWearPattern(pos) * wearIntensity;
        if (edgeWear > 0)
        {
            color = Vector3.Lerp(color, color * 1.3f, edgeWear);
        }
        
        // Scratches
        float scratches = ScratchPattern(pos) * wearIntensity;
        if (scratches > 0.3f)
        {
            color = Vector3.Lerp(color, new Vector3(0.8f, 0.8f, 0.8f), scratches * 0.3f);
        }
        
        // Scorch marks (for combat-focused styles)
        if (style == TextureStyle.Military || style == TextureStyle.Pirate)
        {
            float scorch = ScorchPattern(pos);
            if (scorch > 0.7f)
            {
                color = Vector3.Lerp(color, new Vector3(0.1f, 0.08f, 0.05f), (scorch - 0.7f) * 2f);
            }
        }
        
        // Rust/corrosion (for industrial and pirate)
        if (style == TextureStyle.Industrial || style == TextureStyle.Pirate)
        {
            float rust = RustPattern(pos);
            if (rust > 0.6f)
            {
                Vector3 rustColor = new Vector3(0.6f, 0.3f, 0.1f);
                color = Vector3.Lerp(color, rustColor, (rust - 0.6f) * wearIntensity * 2f);
            }
        }
        
        return color;
    }
    
    /// <summary>
    /// Apply emissive accent highlights
    /// </summary>
    private Vector3 ApplyEmissiveAccents(Vector3 pos, Vector3 color, TextureStyle style, float time)
    {
        if (style == TextureStyle.Sleek || style == TextureStyle.Ancient || style == TextureStyle.Organic)
        {
            float emissivePattern = EmissiveLinePattern(pos, style);
            
            if (emissivePattern > 0.8f)
            {
                Vector3 emissiveColor = style switch
                {
                    TextureStyle.Sleek => new Vector3(0f, 0.8f, 1f),      // Cyan
                    TextureStyle.Ancient => new Vector3(1f, 0.8f, 0.2f),   // Gold
                    TextureStyle.Organic => new Vector3(0.8f, 0.2f, 0.6f), // Magenta
                    _ => new Vector3(0.5f, 0.8f, 1f)
                };
                
                // Pulsing effect
                float pulse = MathF.Sin(time * 2f + pos.Z * 0.5f) * 0.3f + 0.7f;
                
                color = Vector3.Lerp(color, emissiveColor * 1.5f * pulse, (emissivePattern - 0.8f) * 5f);
            }
        }
        
        // Running lights / status indicators
        if (style != TextureStyle.Pirate)
        {
            float lightX = pos.X % LightSpacing;
            float lightZ = pos.Z % LightSpacing;
            
            if (Math.Abs(lightX) < LightSize && Math.Abs(lightZ) < LightSize && pos.Y > 0)
            {
                // Small running light
                float blink = MathF.Sin(time * 3f + pos.X + pos.Z) > LightBlinkThreshold ? 1f : 0.3f;
                color = Vector3.Lerp(color, new Vector3(1f, 0.3f, 0.3f) * blink, 0.8f);
            }
        }
        
        return color;
    }
    
    // ========== Pattern Generation Functions ==========
    
    private float GeometricPattern(Vector3 pos, float scale)
    {
        float angle = MathF.Atan2(pos.Y, pos.X);
        float radius = MathF.Sqrt(pos.X * pos.X + pos.Y * pos.Y);
        
        // Create angular segments
        float segments = 8f;
        float segmentAngle = (angle + MathF.PI) / (2f * MathF.PI) * segments;
        float segmentFrac = MathF.Abs(segmentAngle % 1f - 0.5f);
        
        // Radial rings
        float rings = (radius * scale) % 1f;
        
        return (segmentFrac < 0.1f || rings < 0.1f) ? 1f : 0f;
    }
    
    private float VeinPattern(Vector3 pos, float time)
    {
        // Organic vein-like patterns
        float vein1 = PerlinNoise3D(pos * 2f + new Vector3(time * 0.1f, 0, 0));
        float vein2 = PerlinNoise3D(pos * 3f - new Vector3(0, time * 0.15f, 0));
        
        float veins = MathF.Abs(vein1 * vein2);
        
        // Threshold to create vein lines
        return veins < 0.05f ? 1f - veins * 20f : 0f;
    }
    
    private float CrystallinePattern(Vector3 pos)
    {
        // Voronoi-based crystal facets
        return VoronoiNoise(pos * 0.5f);
    }
    
    private float VentPattern(Vector3 pos)
    {
        // Parallel slits pattern
        float slitSpacing = 0.3f;
        float slitX = (pos.X + pos.Z) % slitSpacing;
        
        return MathF.Abs(slitX) < slitSpacing * 0.15f ? 1f : 0f;
    }
    
    private float EdgeWearPattern(Vector3 pos)
    {
        // Higher values at edges/corners
        float edge = PerlinNoise3D(pos * 4f);
        return MathF.Max(0, edge - 0.3f) * 2f;
    }
    
    private float ScratchPattern(Vector3 pos)
    {
        // Long, thin scratches
        float scratch1 = PerlinNoise3D(pos * new Vector3(0.5f, 10f, 2f));
        float scratch2 = PerlinNoise3D(pos * new Vector3(2f, 0.5f, 10f));
        
        return MathF.Max(
            MathF.Abs(scratch1) < 0.05f ? 1f : 0f,
            MathF.Abs(scratch2) < 0.05f ? 1f : 0f
        );
    }
    
    private float ScorchPattern(Vector3 pos)
    {
        return PerlinNoise3D(pos * 0.3f) * 0.5f + 0.5f;
    }
    
    private float RustPattern(Vector3 pos)
    {
        // Rust tends to form in corners and edges
        float rust = PerlinNoise3D(pos * 1.5f);
        float edge = VoronoiNoise(pos * 0.8f);
        
        return (rust + edge) * 0.5f;
    }
    
    private float EmissiveLinePattern(Vector3 pos, TextureStyle style)
    {
        // Thin glowing lines
        float lineSpacing = style == TextureStyle.Sleek ? 8f : 4f;
        
        float lineY = MathF.Abs((pos.Y % lineSpacing) / lineSpacing - 0.5f);
        float lineZ = MathF.Abs((pos.Z % lineSpacing) / lineSpacing - 0.5f);
        
        float line = MathF.Min(lineY, lineZ);
        return 1f - MathF.Min(line * 20f, 1f);
    }
    
    private float HighFrequencyNoise(Vector3 pos)
    {
        // Fast, cheap noise for fine detail
        int xi = (int)(pos.X * NoiseScale) & HashMask;
        int yi = (int)(pos.Y * NoiseScale) & HashMask;
        int zi = (int)(pos.Z * NoiseScale) & HashMask;
        
        int hash = xi ^ (yi << 1) ^ (zi << 2);
        hash = ((hash >> 8) ^ hash) * 0x5bd1e995;
        hash = ((hash >> 8) ^ hash);
        
        return (hash & 0xFF) / 255f - 0.5f;
    }
    
    // ========== Core Noise Functions ==========
    
    private float PerlinNoise3D(Vector3 pos)
    {
        int xi = (int)MathF.Floor(pos.X) & 255;
        int yi = (int)MathF.Floor(pos.Y) & 255;
        int zi = (int)MathF.Floor(pos.Z) & 255;
        
        float xf = pos.X - MathF.Floor(pos.X);
        float yf = pos.Y - MathF.Floor(pos.Y);
        float zf = pos.Z - MathF.Floor(pos.Z);
        
        float u = Fade(xf);
        float v = Fade(yf);
        float w = Fade(zf);
        
        int a = Hash(xi) + yi;
        int aa = Hash(a) + zi;
        int ab = Hash(a + 1) + zi;
        int b = Hash(xi + 1) + yi;
        int ba = Hash(b) + zi;
        int bb = Hash(b + 1) + zi;
        
        float x1 = Lerp(Gradient(Hash(aa), xf, yf, zf), Gradient(Hash(ba), xf - 1, yf, zf), u);
        float x2 = Lerp(Gradient(Hash(ab), xf, yf - 1, zf), Gradient(Hash(bb), xf - 1, yf - 1, zf), u);
        float y1 = Lerp(x1, x2, v);
        
        x1 = Lerp(Gradient(Hash(aa + 1), xf, yf, zf - 1), Gradient(Hash(ba + 1), xf - 1, yf, zf - 1), u);
        x2 = Lerp(Gradient(Hash(ab + 1), xf, yf - 1, zf - 1), Gradient(Hash(bb + 1), xf - 1, yf - 1, zf - 1), u);
        float y2 = Lerp(x1, x2, v);
        
        return Lerp(y1, y2, w);
    }
    
    private float VoronoiNoise(Vector3 pos)
    {
        Vector3 cellPos = new Vector3(MathF.Floor(pos.X), MathF.Floor(pos.Y), MathF.Floor(pos.Z));
        float minDist = float.MaxValue;
        
        for (int x = -1; x <= 1; x++)
        {
            for (int y = -1; y <= 1; y++)
            {
                for (int z = -1; z <= 1; z++)
                {
                    Vector3 neighborCell = cellPos + new Vector3(x, y, z);
                    Vector3 point = neighborCell + RandomVector3(neighborCell);
                    float dist = Vector3.Distance(pos, point);
                    minDist = MathF.Min(minDist, dist);
                }
            }
        }
        
        return 1.0f - minDist;
    }
    
    private float Fade(float t) => t * t * t * (t * (t * 6 - 15) + 10);
    
    private float Lerp(float a, float b, float t) => a + t * (b - a);
    
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
    
    /// <summary>
    /// Calculate bump/normal value for enhanced surface detail
    /// </summary>
    public float CalculateEnhancedBump(Vector3 worldPosition, TextureStyle style)
    {
        float bump = 0f;
        
        // Panel lines create depth
        float panelScale = 4f;
        float lineThickness = 0.08f;
        float gridX = MathF.Abs((worldPosition.X % panelScale) / panelScale - 0.5f);
        float gridY = MathF.Abs((worldPosition.Y % panelScale) / panelScale - 0.5f);
        float gridZ = MathF.Abs((worldPosition.Z % panelScale) / panelScale - 0.5f);
        
        if (gridX < lineThickness || gridY < lineThickness || gridZ < lineThickness)
        {
            bump -= 0.2f; // Recessed panel lines
        }
        
        // Greebling adds height variation
        if (style != TextureStyle.Sleek)
        {
            bump += PerlinNoise3D(worldPosition * 5f) * 0.1f;
        }
        
        // Scratches create shallow grooves
        float scratches = ScratchPattern(worldPosition);
        if (scratches > 0.3f)
        {
            bump -= scratches * 0.05f;
        }
        
        return bump;
    }
}
