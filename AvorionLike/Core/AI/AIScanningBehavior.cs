using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Navigation;
using AvorionLike.Core.Logging;
using AvorionLike.Core.Physics;

namespace AvorionLike.Core.AI;

/// <summary>
/// Extension to AI system for NPC exploration and scanning behavior
/// NPCs can scan for wormholes, explore new systems, and map anomalies
/// </summary>
public class AIScanningBehavior
{
    private readonly EntityManager _entityManager;
    private readonly ScanningSystem _scanningSystem;
    private readonly Random _random;
    
    public AIScanningBehavior(EntityManager entityManager, ScanningSystem scanningSystem, int seed = 0)
    {
        _entityManager = entityManager;
        _scanningSystem = scanningSystem;
        _random = seed == 0 ? new Random() : new Random(seed);
    }
    
    /// <summary>
    /// Update scanning behavior for AI entities
    /// </summary>
    public void UpdateScanningBehavior(AIComponent ai, float deltaTime)
    {
        if (ai.CurrentState != AIState.Scanning && ai.CurrentState != AIState.Exploring)
            return;
            
        var scanner = _entityManager.GetComponent<ScanningComponent>(ai.EntityId);
        if (scanner == null)
        {
            // No scanning equipment - return to idle
            ai.CurrentState = AIState.Idle;
            return;
        }
        
        // Perform periodic directional scans
        if (scanner.DirectionalScannerCooldown <= 0)
        {
            PerformNPCDirectionalScan(ai, scanner);
        }
        
        // Explorer personality deploys probes to find wormholes
        if (ai.Personality == AIPersonality.Explorer && scanner.DeployedProbes.Count == 0)
        {
            DeployExplorerProbes(ai, scanner);
        }
        
        // Perform probe scan if probes are deployed
        if (scanner.DeployedProbes.Count > 0 && scanner.ProbeScanCooldown <= 0)
        {
            PerformNPCProbeScan(ai, scanner);
        }
    }
    
    /// <summary>
    /// NPC performs a directional scan
    /// </summary>
    private void PerformNPCDirectionalScan(AIComponent ai, ScanningComponent scanner)
    {
        var physics = _entityManager.GetComponent<PhysicsComponent>(ai.EntityId);
        if (physics == null)
            return;
            
        // Scan in a random direction or forward direction
        Vector3 scanDirection = physics.Velocity.LengthSquared() > 0 
            ? Vector3.Normalize(physics.Velocity)
            : new Vector3((float)_random.NextDouble() - 0.5f, 
                         (float)_random.NextDouble() - 0.5f, 
                         (float)_random.NextDouble() - 0.5f);
                         
        scanDirection = Vector3.Normalize(scanDirection);
        
        var signatures = _scanningSystem.PerformDirectionalScan(ai.EntityId, scanDirection, 360f);
        
        if (signatures.Any(s => s.Type == SignatureType.Wormhole))
        {
            Logger.Instance.Info("AIScanningBehavior", 
                $"NPC {ai.EntityId} detected {signatures.Count(s => s.Type == SignatureType.Wormhole)} wormhole(s)");
                
            // Explorer NPCs will investigate wormholes
            if (ai.Personality == AIPersonality.Explorer)
            {
                InvestigateWormhole(ai, signatures.First(s => s.Type == SignatureType.Wormhole));
            }
        }
    }
    
    /// <summary>
    /// NPC deploys probes for systematic scanning
    /// </summary>
    private void DeployExplorerProbes(AIComponent ai, ScanningComponent scanner)
    {
        if (scanner.AvailableProbes < 4)
            return;
            
        var physics = _entityManager.GetComponent<PhysicsComponent>(ai.EntityId);
        if (physics == null)
            return;
            
        // Deploy probes in a tetrahedral pattern for good coverage
        var probePositions = new List<Vector3>
        {
            physics.Position + new Vector3(1000, 0, 0),
            physics.Position + new Vector3(-500, 866, 0),
            physics.Position + new Vector3(-500, -866, 0),
            physics.Position + new Vector3(0, 0, 1000)
        };
        
        _scanningSystem.DeployProbes(ai.EntityId, probePositions);
        
        Logger.Instance.Info("AIScanningBehavior", $"NPC Explorer deployed scanning probes");
    }
    
    /// <summary>
    /// NPC performs a probe scan
    /// </summary>
    private void PerformNPCProbeScan(AIComponent ai, ScanningComponent scanner)
    {
        var signatures = _scanningSystem.PerformProbeScan(ai.EntityId);
        
        // Check for fully scanned wormholes
        foreach (var sig in signatures.Where(s => s.Type == SignatureType.Wormhole && s.ScanProgress >= 1.0f))
        {
            Logger.Instance.Info("AIScanningBehavior", 
                $"NPC {ai.EntityId} fully scanned wormhole: {sig.Name}");
                
            // Record discovery
            RecordWormholeDiscovery(ai, sig);
        }
        
        // If all signatures are scanned, recall probes and move on
        if (signatures.All(s => s.ScanProgress >= 1.0f) && signatures.Any())
        {
            _scanningSystem.RecallProbes(ai.EntityId);
            ai.CurrentState = AIState.Exploring; // Move to exploration
        }
    }
    
    /// <summary>
    /// NPC investigates a detected wormhole
    /// </summary>
    private void InvestigateWormhole(AIComponent ai, ScannedSignature wormhole)
    {
        // Set wormhole as navigation target
        if (wormhole.ScanProgress >= 1.0f)
        {
            Logger.Instance.Info("AIScanningBehavior", 
                $"NPC Explorer navigating to wormhole {wormhole.Name}");
                
            // Would integrate with navigation system to move to wormhole
            ai.CurrentState = AIState.Exploring;
        }
    }
    
    /// <summary>
    /// Record a wormhole discovery for the AI
    /// </summary>
    private void RecordWormholeDiscovery(AIComponent ai, ScannedSignature wormhole)
    {
        // In a full implementation, this would update an AI knowledge base
        // For now, just log the discovery
        Logger.Instance.Info("AIScanningBehavior", 
            $"NPC {ai.EntityId} recorded discovery of {wormhole.Name}");
    }
    
    /// <summary>
    /// Decide if an AI should start scanning based on personality
    /// </summary>
    public bool ShouldStartScanning(AIComponent ai)
    {
        // Explorers frequently scan
        if (ai.Personality == AIPersonality.Explorer)
            return _random.NextDouble() < 0.3; // 30% chance
            
        // Traders occasionally scan for safe routes
        if (ai.Personality == AIPersonality.Trader)
            return _random.NextDouble() < 0.05; // 5% chance
            
        return false;
    }
    
    /// <summary>
    /// Check if AI should enter wormhole
    /// </summary>
    public bool ShouldEnterWormhole(AIComponent ai, WormholeComponent wormhole)
    {
        // Check personality
        if (ai.Personality == AIPersonality.Coward)
            return false; // Cowards avoid unknown space
            
        if (ai.Personality == AIPersonality.Explorer)
            return true; // Explorers love wormholes
            
        // Check wormhole class danger
        if (wormhole.Class >= WormholeClass.Class5)
        {
            // High-class wormholes are dangerous
            if (ai.Personality == AIPersonality.Defensive)
                return false;
                
            // Others have 50% chance
            return _random.NextDouble() < 0.5;
        }
        
        // Low-class wormholes are safer
        return _random.NextDouble() < 0.7;
    }
}
