using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Physics;

namespace AvorionLike.Core.Navigation;

/// <summary>
/// System for scanning mechanics - directional scanner and probe scanning
/// Implements EVE Online-style exploration mechanics
/// </summary>
public class ScanningSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private const float DirectionalScannerCooldownTime = 5f; // seconds
    private const float ProbeScanCooldownTime = 10f; // seconds
    
    public ScanningSystem(EntityManager entityManager) : base("ScanningSystem")
    {
        _entityManager = entityManager;
    }
    
    public override void Update(float deltaTime)
    {
        var scanners = _entityManager.GetAllComponents<ScanningComponent>();
        
        foreach (var scanner in scanners)
        {
            // Update cooldowns
            if (scanner.DirectionalScannerCooldown > 0)
                scanner.DirectionalScannerCooldown -= deltaTime;
                
            if (scanner.ProbeScanCooldown > 0)
                scanner.ProbeScanCooldown -= deltaTime;
        }
    }
    
    /// <summary>
    /// Perform a directional scan from the scanner's position
    /// </summary>
    public List<ScannedSignature> PerformDirectionalScan(Guid scannerEntityId, Vector3 direction, float angle = 360f)
    {
        var scanner = _entityManager.GetComponent<ScanningComponent>(scannerEntityId);
        if (scanner == null)
            return new List<ScannedSignature>();
            
        if (scanner.DirectionalScannerCooldown > 0)
        {
            Logger.Instance.Warning("ScanningSystem", "Directional scanner on cooldown");
            return new List<ScannedSignature>();
        }
        
        var physics = _entityManager.GetComponent<PhysicsComponent>(scannerEntityId);
        if (physics == null)
            return new List<ScannedSignature>();
            
        Vector3 scannerPosition = physics.Position;
        var detectedSignatures = new List<ScannedSignature>();
        
        // Scan for wormholes
        var wormholes = _entityManager.GetAllComponents<WormholeComponent>();
        foreach (var wormhole in wormholes)
        {
            if (!wormhole.IsActive)
                continue;
                
            float distance = Vector3.Distance(scannerPosition, wormhole.Position);
            if (distance > scanner.DirectionalScannerRange)
                continue;
                
            // Check if within scan angle
            if (angle < 360f)
            {
                Vector3 toTarget = Vector3.Normalize(wormhole.Position - scannerPosition);
                float dot = Vector3.Dot(direction, toTarget);
                float angleToTarget = MathF.Acos(dot) * (180f / MathF.PI);
                
                if (angleToTarget > angle / 2f)
                    continue;
            }
            
            // Detection based on signature strength and scan resolution
            float detectionChance = wormhole.SignatureStrength * scanner.ScanResolution;
            
            detectedSignatures.Add(new ScannedSignature
            {
                SignatureId = wormhole.EntityId,
                Type = SignatureType.Wormhole,
                Position = wormhole.Position,
                SignatureStrength = wormhole.SignatureStrength,
                ScanProgress = detectionChance,
                Name = $"Wormhole {wormhole.Designation}"
            });
        }
        
        // Scan for other ships
        var allPhysics = _entityManager.GetAllComponents<PhysicsComponent>();
        foreach (var target in allPhysics)
        {
            if (target.EntityId == scannerEntityId)
                continue;
                
            float distance = Vector3.Distance(scannerPosition, target.Position);
            if (distance > scanner.DirectionalScannerRange)
                continue;
                
            // Check angle
            if (angle < 360f)
            {
                Vector3 toTarget = Vector3.Normalize(target.Position - scannerPosition);
                float dot = Vector3.Dot(direction, toTarget);
                float angleToTarget = MathF.Acos(dot) * (180f / MathF.PI);
                
                if (angleToTarget > angle / 2f)
                    continue;
            }
            
            detectedSignatures.Add(new ScannedSignature
            {
                SignatureId = target.EntityId,
                Type = SignatureType.Ship,
                Position = target.Position,
                SignatureStrength = 1.0f,
                ScanProgress = 1.0f,
                Name = "Ship"
            });
        }
        
        scanner.ScannedSignatures = detectedSignatures;
        scanner.DirectionalScannerCooldown = DirectionalScannerCooldownTime;
        
        Logger.Instance.Info("ScanningSystem", $"Directional scan detected {detectedSignatures.Count} signatures");
        
        return detectedSignatures;
    }
    
    /// <summary>
    /// Deploy scanning probes at specified positions
    /// </summary>
    public bool DeployProbes(Guid scannerEntityId, List<Vector3> positions)
    {
        var scanner = _entityManager.GetComponent<ScanningComponent>(scannerEntityId);
        if (scanner == null)
            return false;
            
        if (positions.Count > scanner.AvailableProbes)
        {
            Logger.Instance.Warning("ScanningSystem", "Not enough probes available");
            return false;
        }
        
        // Deploy probes
        foreach (var position in positions)
        {
            var probe = new ProbeData
            {
                Position = position,
                ScanRadius = 2000f,
                Strength = scanner.ProbeStrength
            };
            
            scanner.DeployedProbes.Add(probe);
            scanner.AvailableProbes--;
        }
        
        Logger.Instance.Info("ScanningSystem", $"Deployed {positions.Count} probes");
        return true;
    }
    
    /// <summary>
    /// Recall all deployed probes
    /// </summary>
    public void RecallProbes(Guid scannerEntityId)
    {
        var scanner = _entityManager.GetComponent<ScanningComponent>(scannerEntityId);
        if (scanner == null)
            return;
            
        scanner.AvailableProbes += scanner.DeployedProbes.Count;
        scanner.DeployedProbes.Clear();
        
        Logger.Instance.Info("ScanningSystem", "Recalled all probes");
    }
    
    /// <summary>
    /// Perform a probe scan with deployed probes
    /// </summary>
    public List<ScannedSignature> PerformProbeScan(Guid scannerEntityId)
    {
        var scanner = _entityManager.GetComponent<ScanningComponent>(scannerEntityId);
        if (scanner == null)
            return new List<ScannedSignature>();
            
        if (scanner.ProbeScanCooldown > 0)
        {
            Logger.Instance.Warning("ScanningSystem", "Probe scanner on cooldown");
            return new List<ScannedSignature>();
        }
        
        if (scanner.DeployedProbes.Count == 0)
        {
            Logger.Instance.Warning("ScanningSystem", "No probes deployed");
            return new List<ScannedSignature>();
        }
        
        var detectedSignatures = new List<ScannedSignature>();
        
        // Scan for wormholes using probe triangulation
        var wormholes = _entityManager.GetAllComponents<WormholeComponent>();
        foreach (var wormhole in wormholes)
        {
            if (!wormhole.IsActive)
                continue;
            
            // Check if wormhole is in range of any probe
            float bestScanStrength = 0f;
            int probesInRange = 0;
            
            foreach (var probe in scanner.DeployedProbes)
            {
                float distance = Vector3.Distance(probe.Position, wormhole.Position);
                if (distance <= probe.ScanRadius)
                {
                    probesInRange++;
                    float scanStrength = (1f - (distance / probe.ScanRadius)) * probe.Strength;
                    bestScanStrength = MathF.Max(bestScanStrength, scanStrength);
                }
            }
            
            if (probesInRange > 0)
            {
                // Multiple probes improve scan quality
                float probeBonus = 1f + (probesInRange - 1) * 0.25f; // 25% bonus per additional probe
                float totalScanStrength = bestScanStrength * probeBonus * scanner.ScanResolution;
                
                var signature = scanner.ScannedSignatures.FirstOrDefault(s => s.SignatureId == wormhole.EntityId);
                if (signature == null)
                {
                    signature = new ScannedSignature
                    {
                        SignatureId = wormhole.EntityId,
                        Type = SignatureType.Wormhole,
                        Position = wormhole.Position,
                        SignatureStrength = wormhole.SignatureStrength,
                        ScanProgress = 0f,
                        Name = $"Wormhole {wormhole.Designation}"
                    };
                    scanner.ScannedSignatures.Add(signature);
                }
                
                // Improve scan progress
                signature.ScanProgress = MathF.Min(1f, signature.ScanProgress + totalScanStrength * 0.25f);
                detectedSignatures.Add(signature);
            }
        }
        
        scanner.ProbeScanCooldown = ProbeScanCooldownTime;
        
        Logger.Instance.Info("ScanningSystem", $"Probe scan detected {detectedSignatures.Count} signatures");
        
        return detectedSignatures;
    }
    
    /// <summary>
    /// Check if a signature is fully scanned and can be warped to
    /// </summary>
    public bool IsSignatureFullyScanned(ScannedSignature signature)
    {
        return signature.ScanProgress >= 1.0f;
    }
    
    /// <summary>
    /// Get the approximate position of a partially scanned signature
    /// </summary>
    public Vector3? GetSignatureApproximatePosition(ScannedSignature signature)
    {
        if (signature.ScanProgress < 0.25f)
            return null;
            
        // Add noise based on scan progress (use signature ID as seed for consistency)
        float noise = (1f - signature.ScanProgress) * 5000f;
        var random = new Random(signature.SignatureId.GetHashCode());
        
        return signature.Position + new Vector3(
            (float)(random.NextDouble() * noise - noise / 2),
            (float)(random.NextDouble() * noise - noise / 2),
            (float)(random.NextDouble() * noise - noise / 2)
        );
    }
}
