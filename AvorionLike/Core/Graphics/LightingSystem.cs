using System.Numerics;

namespace AvorionLike.Core.Graphics;

/// <summary>
/// Lighting system for space environments
/// </summary>
public class LightingSystem
{
    private List<Light> _lights = new();
    private Vector3 _ambientColor = new Vector3(0.1f, 0.1f, 0.15f); // Dark blue ambient
    
    /// <summary>
    /// Add a light source
    /// </summary>
    public void AddLight(Light light)
    {
        _lights.Add(light);
    }
    
    /// <summary>
    /// Remove a light source
    /// </summary>
    public void RemoveLight(Light light)
    {
        _lights.Remove(light);
    }
    
    /// <summary>
    /// Get all active lights
    /// </summary>
    public IEnumerable<Light> GetLights()
    {
        return _lights.Where(l => l.IsActive);
    }
    
    /// <summary>
    /// Calculate lighting for a point in space
    /// </summary>
    public Vector3 CalculateLighting(Vector3 position, Vector3 normal)
    {
        Vector3 color = _ambientColor;
        
        foreach (var light in _lights.Where(l => l.IsActive))
        {
            float distance = Vector3.Distance(position, light.Position);
            
            // Check if within range
            if (distance > light.Range)
                continue;
            
            // Calculate attenuation
            float attenuation = CalculateAttenuation(distance, light);
            
            // Calculate diffuse lighting
            Vector3 lightDir = Vector3.Normalize(light.Position - position);
            float diffuse = Math.Max(Vector3.Dot(normal, lightDir), 0.0f);
            
            // Add light contribution
            color += light.Color * light.Intensity * diffuse * attenuation;
        }
        
        return Vector3.Clamp(color, Vector3.Zero, Vector3.One);
    }
    
    /// <summary>
    /// Calculate light attenuation based on distance
    /// </summary>
    private float CalculateAttenuation(float distance, Light light)
    {
        return 1.0f / (light.ConstantAttenuation +
                       light.LinearAttenuation * distance +
                       light.QuadraticAttenuation * distance * distance);
    }
    
    /// <summary>
    /// Set ambient lighting color
    /// </summary>
    public void SetAmbientColor(Vector3 color)
    {
        _ambientColor = color;
    }
    
    /// <summary>
    /// Create a star light (bright point light)
    /// </summary>
    public static Light CreateStarLight(Vector3 position, Vector3 color, float intensity = 1.0f)
    {
        return new Light
        {
            Type = LightType.Point,
            Position = position,
            Color = color,
            Intensity = intensity,
            Range = 5000f,
            ConstantAttenuation = 1.0f,
            LinearAttenuation = 0.0001f,
            QuadraticAttenuation = 0.000001f
        };
    }
    
    /// <summary>
    /// Create a nebula light (soft area light)
    /// </summary>
    public static Light CreateNebulaLight(Vector3 position, Vector3 color, float intensity = 0.5f)
    {
        return new Light
        {
            Type = LightType.Area,
            Position = position,
            Color = color,
            Intensity = intensity,
            Range = 3000f,
            ConstantAttenuation = 1.0f,
            LinearAttenuation = 0.0005f,
            QuadraticAttenuation = 0.00001f
        };
    }
}

/// <summary>
/// Light source
/// </summary>
public class Light
{
    public LightType Type { get; set; } = LightType.Point;
    public Vector3 Position { get; set; }
    public Vector3 Color { get; set; } = Vector3.One;
    public float Intensity { get; set; } = 1.0f;
    public float Range { get; set; } = 100f;
    public bool IsActive { get; set; } = true;
    
    // Attenuation parameters
    public float ConstantAttenuation { get; set; } = 1.0f;
    public float LinearAttenuation { get; set; } = 0.01f;
    public float QuadraticAttenuation { get; set; } = 0.001f;
}

/// <summary>
/// Type of light source
/// </summary>
public enum LightType
{
    Point,
    Directional,
    Spot,
    Area
}

/// <summary>
/// Visual effects system for mining beams, explosions, etc.
/// </summary>
public class VisualEffectsSystem
{
    private List<VisualEffect> _activeEffects = new();
    
    /// <summary>
    /// Create a mining beam effect
    /// </summary>
    public VisualEffect CreateMiningBeam(Vector3 start, Vector3 end, Vector3 color)
    {
        var effect = new MiningBeamEffect
        {
            StartPosition = start,
            EndPosition = end,
            Color = color,
            Intensity = 1.0f,
            Duration = float.MaxValue, // Continuous until stopped
            IsActive = true
        };
        
        _activeEffects.Add(effect);
        return effect;
    }
    
    /// <summary>
    /// Create an explosion effect
    /// </summary>
    public VisualEffect CreateExplosion(Vector3 position, float radius, Vector3 color)
    {
        var effect = new ExplosionEffect
        {
            Position = position,
            Radius = radius,
            Color = color,
            Duration = 2.0f,
            IsActive = true
        };
        
        _activeEffects.Add(effect);
        return effect;
    }
    
    /// <summary>
    /// Create a resource glow effect
    /// </summary>
    public VisualEffect CreateResourceGlow(Vector3 position, Vector3 color, float intensity)
    {
        var effect = new ResourceGlowEffect
        {
            Position = position,
            Color = color,
            Intensity = intensity,
            Radius = 5.0f,
            Duration = float.MaxValue,
            IsActive = true
        };
        
        _activeEffects.Add(effect);
        return effect;
    }
    
    /// <summary>
    /// Update all active effects
    /// </summary>
    public void Update(float deltaTime)
    {
        foreach (var effect in _activeEffects.ToList())
        {
            effect.ElapsedTime += deltaTime;
            
            // Remove expired effects
            if (effect.ElapsedTime >= effect.Duration)
            {
                effect.IsActive = false;
                _activeEffects.Remove(effect);
            }
        }
    }
    
    /// <summary>
    /// Get all active effects
    /// </summary>
    public IEnumerable<VisualEffect> GetActiveEffects()
    {
        return _activeEffects.Where(e => e.IsActive);
    }
    
    /// <summary>
    /// Stop an effect
    /// </summary>
    public void StopEffect(VisualEffect effect)
    {
        effect.IsActive = false;
        _activeEffects.Remove(effect);
    }
}

/// <summary>
/// Base class for visual effects
/// </summary>
public abstract class VisualEffect
{
    public bool IsActive { get; set; }
    public float Duration { get; set; }
    public float ElapsedTime { get; set; }
    public Vector3 Color { get; set; }
}

/// <summary>
/// Mining beam effect
/// </summary>
public class MiningBeamEffect : VisualEffect
{
    public Vector3 StartPosition { get; set; }
    public Vector3 EndPosition { get; set; }
    public float Intensity { get; set; }
}

/// <summary>
/// Explosion effect
/// </summary>
public class ExplosionEffect : VisualEffect
{
    public Vector3 Position { get; set; }
    public float Radius { get; set; }
    public float MaxRadius { get; set; }
    
    public float CurrentRadius => Radius * (ElapsedTime / Duration);
    public float CurrentIntensity => 1.0f - (ElapsedTime / Duration);
}

/// <summary>
/// Resource glow effect (for resource-rich areas)
/// </summary>
public class ResourceGlowEffect : VisualEffect
{
    public Vector3 Position { get; set; }
    public float Intensity { get; set; }
    public float Radius { get; set; }
    public float PulseSpeed { get; set; } = 2.0f;
    
    public float CurrentIntensity => Intensity * (0.7f + 0.3f * (float)Math.Sin(ElapsedTime * PulseSpeed));
}
