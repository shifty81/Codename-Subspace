using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Navigation;

/// <summary>
/// Component for scanning capabilities - directional scanner and probes
/// Inspired by EVE Online's scanning mechanics
/// </summary>
public class ScanningComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// Maximum range of the directional scanner (km)
    /// </summary>
    public float DirectionalScannerRange { get; set; } = 14000f; // 14 AU equivalent
    
    /// <summary>
    /// Scan resolution - higher is better at finding smaller signatures
    /// </summary>
    public float ScanResolution { get; set; } = 1.0f;
    
    /// <summary>
    /// Probe strength for signature detection
    /// </summary>
    public float ProbeStrength { get; set; } = 1.0f;
    
    /// <summary>
    /// Number of probes available
    /// </summary>
    public int AvailableProbes { get; set; } = 8;
    
    /// <summary>
    /// Maximum number of probes that can be deployed
    /// </summary>
    public int MaxProbes { get; set; } = 8;
    
    /// <summary>
    /// Currently deployed probes
    /// </summary>
    public List<ProbeData> DeployedProbes { get; set; } = new();
    
    /// <summary>
    /// Recently scanned signatures
    /// </summary>
    public List<ScannedSignature> ScannedSignatures { get; set; } = new();
    
    /// <summary>
    /// Cooldown for directional scanner (seconds)
    /// </summary>
    public float DirectionalScannerCooldown { get; set; } = 0f;
    
    /// <summary>
    /// Cooldown for probe scanning (seconds)
    /// </summary>
    public float ProbeScanCooldown { get; set; } = 0f;
}

/// <summary>
/// Represents a deployed scanning probe
/// </summary>
public class ProbeData
{
    public Guid ProbeId { get; set; } = Guid.NewGuid();
    public Vector3 Position { get; set; }
    public float ScanRadius { get; set; } = 1000f;
    public float Strength { get; set; } = 1.0f;
}

/// <summary>
/// Represents a detected signature from scanning
/// </summary>
public class ScannedSignature
{
    public Guid SignatureId { get; set; }
    public SignatureType Type { get; set; }
    public Vector3 Position { get; set; }
    public float SignatureStrength { get; set; }
    public float ScanProgress { get; set; } // 0.0 to 1.0
    public string Name { get; set; } = "Unknown Signal";
}

/// <summary>
/// Types of scannable signatures
/// </summary>
public enum SignatureType
{
    Unknown,
    Wormhole,
    Ship,
    Station,
    Asteroid,
    Anomaly,
    CosmicSignature,
    CombatSite,
    DataSite,
    RelicSite
}

/// <summary>
/// Interface for entities that can be scanned
/// </summary>
public interface IScannable
{
    /// <summary>
    /// Signature strength for detection (0.0 - 1.0)
    /// </summary>
    float SignatureStrength { get; }
    
    /// <summary>
    /// Type of signature
    /// </summary>
    SignatureType SignatureType { get; }
    
    /// <summary>
    /// Position of the scannable object
    /// </summary>
    Vector3 Position { get; }
    
    /// <summary>
    /// Unique identifier
    /// </summary>
    Guid Id { get; }
}
