using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Color palette for celestial bodies
/// </summary>
public class CelestialColorPalette
{
    public string Name { get; set; } = "";
    public Vector3[] Colors { get; set; } = Array.Empty<Vector3>();
    public float[] ColorStops { get; set; } = Array.Empty<float>(); // 0-1 positions for each color
    
    /// <summary>
    /// Sample a color from the palette at position t (0-1)
    /// </summary>
    public Vector3 SampleColor(float t)
    {
        if (Colors.Length == 0) return Vector3.One;
        if (Colors.Length == 1) return Colors[0];
        
        // Clamp t to valid range
        t = Math.Clamp(t, 0f, 1f);
        
        // Find the two colors to interpolate between
        for (int i = 0; i < ColorStops.Length - 1; i++)
        {
            if (t >= ColorStops[i] && t <= ColorStops[i + 1])
            {
                float localT = (t - ColorStops[i]) / (ColorStops[i + 1] - ColorStops[i]);
                return Vector3.Lerp(Colors[i], Colors[i + 1], localT);
            }
        }
        
        return Colors[^1];
    }
}

/// <summary>
/// Specialized texture generator for celestial bodies
/// </summary>
public class CelestialTextureGenerator
{
    private readonly ProceduralTextureGenerator _textureGen;
    private readonly Dictionary<string, CelestialColorPalette> _palettes;
    
    public CelestialTextureGenerator(int seed = 0)
    {
        _textureGen = new ProceduralTextureGenerator(seed);
        _palettes = InitializePalettes();
    }
    
    private Dictionary<string, CelestialColorPalette> InitializePalettes()
    {
        var palettes = new Dictionary<string, CelestialColorPalette>();
        
        // Gas Giant - Jupiter-like
        palettes["jupiter"] = new CelestialColorPalette
        {
            Name = "Jupiter",
            Colors = new[]
            {
                new Vector3(0.9f, 0.85f, 0.7f),  // Cream
                new Vector3(0.8f, 0.6f, 0.4f),   // Light orange
                new Vector3(0.6f, 0.3f, 0.2f),   // Dark orange
                new Vector3(0.9f, 0.75f, 0.6f),  // Light brown
                new Vector3(0.5f, 0.25f, 0.15f)  // Dark brown
            },
            ColorStops = new[] { 0f, 0.25f, 0.5f, 0.75f, 1f }
        };
        
        // Gas Giant - Neptune-like
        palettes["neptune"] = new CelestialColorPalette
        {
            Name = "Neptune",
            Colors = new[]
            {
                new Vector3(0.2f, 0.4f, 0.8f),   // Deep blue
                new Vector3(0.3f, 0.5f, 0.9f),   // Medium blue
                new Vector3(0.6f, 0.75f, 0.95f), // Light blue
                new Vector3(0.5f, 0.65f, 0.85f)  // Cyan
            },
            ColorStops = new[] { 0f, 0.33f, 0.66f, 1f }
        };
        
        // Gas Giant - Saturn-like
        palettes["saturn"] = new CelestialColorPalette
        {
            Name = "Saturn",
            Colors = new[]
            {
                new Vector3(0.95f, 0.9f, 0.7f),  // Pale yellow
                new Vector3(0.9f, 0.85f, 0.65f), // Light tan
                new Vector3(0.85f, 0.75f, 0.55f) // Darker tan
            },
            ColorStops = new[] { 0f, 0.5f, 1f }
        };
        
        // Gas Giant - Toxic/Uranus-like
        palettes["toxic"] = new CelestialColorPalette
        {
            Name = "Toxic",
            Colors = new[]
            {
                new Vector3(0.6f, 0.8f, 0.3f),   // Lime green
                new Vector3(0.8f, 0.9f, 0.4f),   // Yellow-green
                new Vector3(0.4f, 0.6f, 0.2f),   // Dark green
                new Vector3(0.7f, 0.7f, 0.3f)    // Sickly yellow
            },
            ColorStops = new[] { 0f, 0.33f, 0.66f, 1f }
        };
        
        // Rocky Planet - Desert/Mars
        palettes["desert"] = new CelestialColorPalette
        {
            Name = "Desert",
            Colors = new[]
            {
                new Vector3(0.8f, 0.5f, 0.3f),   // Orange sand
                new Vector3(0.7f, 0.4f, 0.2f),   // Dark orange
                new Vector3(0.9f, 0.7f, 0.5f),   // Light sand
                new Vector3(0.6f, 0.3f, 0.15f)   // Reddish rock
            },
            ColorStops = new[] { 0f, 0.33f, 0.66f, 1f }
        };
        
        // Rocky Planet - Earth-like
        palettes["earthlike"] = new CelestialColorPalette
        {
            Name = "Earth-like",
            Colors = new[]
            {
                new Vector3(0.1f, 0.2f, 0.5f),   // Ocean blue
                new Vector3(0.2f, 0.5f, 0.2f),   // Forest green
                new Vector3(0.6f, 0.5f, 0.4f),   // Mountain brown
                new Vector3(0.9f, 0.9f, 0.95f)   // Snow white
            },
            ColorStops = new[] { 0f, 0.4f, 0.7f, 1f }
        };
        
        // Rocky Planet - Volcanic
        palettes["volcanic"] = new CelestialColorPalette
        {
            Name = "Volcanic",
            Colors = new[]
            {
                new Vector3(0.2f, 0.2f, 0.2f),   // Dark rock
                new Vector3(1.0f, 0.3f, 0.0f),   // Lava orange
                new Vector3(0.8f, 0.1f, 0.0f),   // Dark lava
                new Vector3(0.3f, 0.25f, 0.2f)   // Ash gray
            },
            ColorStops = new[] { 0f, 0.3f, 0.6f, 1f }
        };
        
        // Rocky Planet - Ice
        palettes["ice"] = new CelestialColorPalette
        {
            Name = "Ice",
            Colors = new[]
            {
                new Vector3(0.85f, 0.9f, 0.95f), // Light ice
                new Vector3(0.7f, 0.8f, 0.9f),   // Blue ice
                new Vector3(0.6f, 0.65f, 0.75f), // Dark ice
                new Vector3(0.9f, 0.95f, 1.0f)   // Snow white
            },
            ColorStops = new[] { 0f, 0.33f, 0.66f, 1f }
        };
        
        // Asteroid
        palettes["asteroid"] = new CelestialColorPalette
        {
            Name = "Asteroid",
            Colors = new[]
            {
                new Vector3(0.4f, 0.4f, 0.4f),   // Gray rock
                new Vector3(0.3f, 0.25f, 0.2f),  // Brown rock
                new Vector3(0.5f, 0.5f, 0.5f),   // Light gray
                new Vector3(0.6f, 0.5f, 0.3f)    // Ore hints (gold/copper)
            },
            ColorStops = new[] { 0f, 0.4f, 0.7f, 1f }
        };
        
        // Nebula - Pink/Purple
        palettes["nebula_pink"] = new CelestialColorPalette
        {
            Name = "Pink Nebula",
            Colors = new[]
            {
                new Vector3(1.0f, 0.3f, 0.6f),   // Hot pink
                new Vector3(0.8f, 0.2f, 0.5f),   // Deep pink
                new Vector3(0.6f, 0.4f, 0.8f),   // Purple
                new Vector3(0.9f, 0.5f, 0.7f)    // Light pink
            },
            ColorStops = new[] { 0f, 0.33f, 0.66f, 1f }
        };
        
        // Nebula - Blue/Green
        palettes["nebula_blue"] = new CelestialColorPalette
        {
            Name = "Blue Nebula",
            Colors = new[]
            {
                new Vector3(0.2f, 0.5f, 0.9f),   // Bright blue
                new Vector3(0.3f, 0.7f, 0.8f),   // Cyan
                new Vector3(0.4f, 0.8f, 0.6f),   // Blue-green
                new Vector3(0.5f, 0.6f, 0.9f)    // Light blue
            },
            ColorStops = new[] { 0f, 0.33f, 0.66f, 1f }
        };
        
        return palettes;
    }
    
    /// <summary>
    /// Generate texture for a gas giant with swirling bands
    /// </summary>
    public Vector3 GenerateGasGiantTexture(Vector3 worldPos, string paletteType, float time = 0f)
    {
        var palette = _palettes.GetValueOrDefault(paletteType, _palettes["jupiter"]);
        
        // Latitude-based banding (around Y axis)
        float latitude = worldPos.Y;
        
        // Add turbulence for storm systems
        float turbulence = GenerateTurbulence(worldPos * 0.5f, time);
        
        // Large-scale bands
        float bandPattern = MathF.Sin(latitude * 0.3f + turbulence * 2.0f);
        
        // Fine detail waves
        float waves = MathF.Sin(latitude * 2.0f + turbulence * 4.0f) * 0.2f;
        float detailWaves = MathF.Sin(worldPos.X * 5.0f + time * 0.5f) * 0.1f;
        
        // Combine for color position
        float colorPos = (bandPattern + waves + detailWaves + 1.0f) * 0.5f;
        colorPos = Math.Clamp(colorPos, 0f, 1f);
        
        // Sample from palette
        Vector3 color = palette.SampleColor(colorPos);
        
        // Add atmospheric shimmer
        float shimmer = GenerateShimmer(worldPos, time) * 0.1f;
        color += new Vector3(shimmer, shimmer, shimmer);
        
        // Add great red spot or vortex features for Jupiter-like
        if (paletteType == "jupiter")
        {
            float vortex = GenerateVortex(worldPos, new Vector3(0, 20, 0), 15f);
            if (vortex > 0)
            {
                Vector3 vortexColor = new Vector3(0.8f, 0.3f, 0.2f); // Reddish
                color = Vector3.Lerp(color, vortexColor, vortex * 0.7f);
            }
        }
        
        return Vector3.Clamp(color, Vector3.Zero, Vector3.One);
    }
    
    /// <summary>
    /// Generate texture for an asteroid with craters and ore
    /// </summary>
    public Vector3 GenerateAsteroidTexture(Vector3 worldPos, float resourceDensity = 0.3f)
    {
        var palette = _palettes["asteroid"];
        
        // Base rocky texture
        float rockNoise = GenerateMultiOctaveNoise(worldPos * 2.0f, 4);
        Vector3 baseColor = palette.SampleColor(rockNoise * 0.5f + 0.5f);
        
        // Crater detection (darker areas)
        float craterNoise = GenerateMultiOctaveNoise(worldPos * 1.0f, 3);
        if (craterNoise < -0.4f)
        {
            // Crater interior - darker and shadowed
            baseColor *= 0.6f;
        }
        
        // Ore veins (if resource-rich)
        if (resourceDensity > 0)
        {
            float oreNoise = GenerateMultiOctaveNoise(worldPos * 3.0f + new Vector3(100, 100, 100), 2);
            if (oreNoise > 0.6f * (1.0f / resourceDensity))
            {
                // Metallic ore glints
                Vector3 oreColor = new Vector3(0.7f, 0.6f, 0.3f); // Gold/copper
                float oreBlend = (oreNoise - 0.6f) * resourceDensity;
                baseColor = Vector3.Lerp(baseColor, oreColor, oreBlend * 0.5f);
            }
        }
        
        return Vector3.Clamp(baseColor, Vector3.Zero, Vector3.One);
    }
    
    /// <summary>
    /// Generate texture for a rocky planet with biomes
    /// </summary>
    public Vector3 GenerateRockyPlanetTexture(Vector3 worldPos, string paletteType, float altitude, float temperature, float moisture)
    {
        var palette = _palettes.GetValueOrDefault(paletteType, _palettes["earthlike"]);
        
        // Base terrain color from altitude
        float normalizedAltitude = (altitude + 200) / 600f; // Normalize -200 to 400 range
        normalizedAltitude = Math.Clamp(normalizedAltitude, 0f, 1f);
        
        Vector3 baseColor = palette.SampleColor(normalizedAltitude);
        
        // Add terrain detail noise
        float terrainDetail = GenerateMultiOctaveNoise(worldPos * 5.0f, 3) * 0.15f;
        baseColor += new Vector3(terrainDetail, terrainDetail, terrainDetail);
        
        // Climate-based variation
        if (paletteType == "earthlike")
        {
            // Polar ice caps (high altitude or low temperature)
            if (temperature < 0.2f || altitude > 350)
            {
                Vector3 snowColor = new Vector3(0.95f, 0.95f, 1.0f);
                float snowBlend = altitude > 350 ? (altitude - 350) / 50f : (0.2f - temperature) / 0.2f;
                baseColor = Vector3.Lerp(baseColor, snowColor, Math.Clamp(snowBlend, 0f, 1f));
            }
            
            // Vegetation (moderate temperature, high moisture)
            if (temperature > 0.4f && temperature < 0.8f && moisture > 0.5f && altitude < 100)
            {
                Vector3 vegColor = new Vector3(0.2f, 0.5f, 0.2f);
                baseColor = Vector3.Lerp(baseColor, vegColor, moisture * 0.6f);
            }
        }
        else if (paletteType == "volcanic")
        {
            // Lava flows
            float lavaFlow = GenerateMultiOctaveNoise(worldPos * 1.5f, 2);
            if (lavaFlow > 0.5f && altitude < 50)
            {
                Vector3 lavaColor = new Vector3(1.0f, 0.3f, 0.0f);
                float lavaBlend = (lavaFlow - 0.5f) * 2.0f;
                baseColor = Vector3.Lerp(baseColor, lavaColor, lavaBlend);
                // Add glow
                baseColor *= 1.5f;
            }
        }
        
        return Vector3.Clamp(baseColor, Vector3.Zero, Vector3.One * 2.0f); // Allow overbright for lava
    }
    
    /// <summary>
    /// Generate texture for nebula clouds
    /// </summary>
    public Vector3 GenerateNebulaTexture(Vector3 worldPos, string paletteType, float time = 0f)
    {
        var palette = _palettes.GetValueOrDefault(paletteType, _palettes["nebula_pink"]);
        
        // Animated swirling clouds
        Vector3 animatedPos = worldPos + new Vector3(
            MathF.Sin(time * 0.1f) * 5f,
            MathF.Cos(time * 0.15f) * 5f,
            time * 2f
        );
        
        // Multiple layers of turbulence
        float turbulence1 = GenerateTurbulence(animatedPos * 0.3f, time * 0.5f);
        float turbulence2 = GenerateTurbulence(animatedPos * 0.6f + new Vector3(50, 50, 50), time * 0.3f);
        
        // Combine turbulence
        float density = (turbulence1 + turbulence2) * 0.5f;
        
        // Sample color based on density
        float colorPos = (density + 1.0f) * 0.5f;
        Vector3 color = palette.SampleColor(colorPos);
        
        // Add shimmer/glow
        float shimmer = GenerateShimmer(worldPos, time);
        color += color * shimmer * 0.3f; // Self-illumination
        
        // Vary opacity based on density
        float opacity = Math.Clamp((density + 1.0f) * 0.4f, 0.1f, 0.8f);
        
        return color;
    }
    
    /// <summary>
    /// Generate texture for station/ship hulls
    /// </summary>
    public Vector3 GenerateStationTexture(Vector3 worldPos, Vector3 baseColor, bool addWeathering = true)
    {
        // Hull panel grid
        float panelSize = 4.0f;
        float gridX = MathF.Abs(worldPos.X % panelSize - panelSize / 2);
        float gridY = MathF.Abs(worldPos.Y % panelSize - panelSize / 2);
        
        Vector3 color = baseColor;
        
        // Panel lines (darker)
        if (gridX < 0.1f || gridY < 0.1f)
        {
            color *= 0.7f;
        }
        
        // Rivets at panel corners
        if (gridX < 0.2f && gridY < 0.2f)
        {
            color *= 0.8f;
        }
        
        // Add subtle surface variation
        float surfaceNoise = GenerateMultiOctaveNoise(worldPos * 3.0f, 2) * 0.05f;
        color += new Vector3(surfaceNoise, surfaceNoise, surfaceNoise);
        
        // Weathering effects
        if (addWeathering)
        {
            // Scorch marks near damage
            float scorch = GenerateMultiOctaveNoise(worldPos * 0.8f, 2);
            if (scorch > 0.6f)
            {
                Vector3 scorchColor = new Vector3(0.1f, 0.1f, 0.1f);
                float scorchBlend = (scorch - 0.6f) / 0.4f;
                color = Vector3.Lerp(color, scorchColor, scorchBlend * 0.4f);
            }
            
            // Rust/oxidation
            float rust = GenerateMultiOctaveNoise(worldPos * 1.5f + new Vector3(100, 100, 100), 2);
            if (rust > 0.5f)
            {
                Vector3 rustColor = new Vector3(0.6f, 0.3f, 0.1f);
                float rustBlend = (rust - 0.5f) / 0.5f;
                color = Vector3.Lerp(color, rustColor, rustBlend * 0.2f);
            }
        }
        
        return Vector3.Clamp(color, Vector3.Zero, Vector3.One);
    }
    
    // Helper noise functions
    
    private float GenerateTurbulence(Vector3 pos, float time)
    {
        // Domain warping for turbulent flow
        float qx = GenerateMultiOctaveNoise(pos, 2);
        float qy = GenerateMultiOctaveNoise(pos + new Vector3(5.2f, 1.3f, 0), 2);
        
        Vector3 warpedPos = pos + new Vector3(qx, qy, 0) * 3.0f + new Vector3(time * 0.5f, 0, 0);
        return GenerateMultiOctaveNoise(warpedPos, 3);
    }
    
    private float GenerateVortex(Vector3 pos, Vector3 center, float radius)
    {
        Vector3 toCenter = pos - center;
        float dist = toCenter.Length();
        
        if (dist > radius) return 0f;
        
        // Spiral pattern
        float angle = MathF.Atan2(toCenter.Z, toCenter.X);
        float spiral = MathF.Sin(angle * 3.0f + dist * 0.5f);
        
        // Falloff from center
        float falloff = 1.0f - (dist / radius);
        
        return falloff * (spiral * 0.5f + 0.5f);
    }
    
    private float GenerateShimmer(Vector3 pos, float time)
    {
        return MathF.Sin(pos.X * 10.0f + time * 2.0f) * 
               MathF.Sin(pos.Y * 12.0f + time * 1.5f) * 
               MathF.Sin(pos.Z * 8.0f + time * 2.5f);
    }
    
    private float GenerateMultiOctaveNoise(Vector3 pos, int octaves)
    {
        // Use the texture generator's Perlin noise through material
        var tempMaterial = new TextureMaterial
        {
            NoiseScale = 1.0f,
            NoiseStrength = 1.0f
        };
        
        float noise = 0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float maxValue = 0f;
        
        for (int i = 0; i < octaves; i++)
        {
            Vector3 color = _textureGen.GenerateTextureColor(pos * frequency, tempMaterial);
            float octaveNoise = (color.X + color.Y + color.Z) / 3.0f;
            noise += (octaveNoise - 0.5f) * 2.0f * amplitude; // Convert 0-1 to -1 to 1
            
            maxValue += amplitude;
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }
        
        return noise / maxValue;
    }
    
    /// <summary>
    /// Get available palette types
    /// </summary>
    public IEnumerable<string> GetAvailablePalettes()
    {
        return _palettes.Keys;
    }
}
