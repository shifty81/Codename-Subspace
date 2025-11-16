using System.Numerics;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Procedural;
using AvorionLike.Core.Navigation;

namespace AvorionLike.Core.Progression;

/// <summary>
/// Manages galaxy-wide progression from rim to center
/// Handles difficulty scaling, material availability, and zone progression
/// </summary>
public class GalaxyProgressionSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private const int GalaxyCenterX = 0;
    private const int GalaxyCenterY = 0;
    private const int GalaxyCenterZ = 0;
    
    public GalaxyProgressionSystem(EntityManager entityManager) : base("GalaxyProgressionSystem")
    {
        _entityManager = entityManager;
    }
    
    public override void Update(float deltaTime)
    {
        // Update player progression based on current location
        var players = _entityManager.GetAllComponents<PlayerProgressionComponent>();
        
        foreach (var player in players)
        {
            UpdatePlayerProgression(player);
        }
    }
    
    /// <summary>
    /// Calculate distance from galactic center
    /// </summary>
    public static int GetDistanceFromCenter(SectorCoordinate sector)
    {
        int dx = sector.X - GalaxyCenterX;
        int dy = sector.Y - GalaxyCenterY;
        int dz = sector.Z - GalaxyCenterZ;
        return (int)Math.Sqrt(dx * dx + dy * dy + dz * dz);
    }
    
    /// <summary>
    /// Get the highest material tier available at this distance from center
    /// </summary>
    public static MaterialTier GetAvailableMaterialTier(int distanceFromCenter)
    {
        if (distanceFromCenter < 25) return MaterialTier.Avorion;
        if (distanceFromCenter < 50) return MaterialTier.Ogonite;
        if (distanceFromCenter < 75) return MaterialTier.Xanion;
        if (distanceFromCenter < 150) return MaterialTier.Trinium;
        if (distanceFromCenter < 250) return MaterialTier.Naonite;
        if (distanceFromCenter < 350) return MaterialTier.Titanium;
        return MaterialTier.Iron;
    }
    
    /// <summary>
    /// Get difficulty multiplier based on distance from center
    /// </summary>
    public static float GetDifficultyMultiplier(int distanceFromCenter)
    {
        // Closer to center = harder enemies
        if (distanceFromCenter < 25) return 10.0f;  // Endgame difficulty
        if (distanceFromCenter < 50) return 6.0f;
        if (distanceFromCenter < 75) return 4.0f;
        if (distanceFromCenter < 150) return 2.5f;
        if (distanceFromCenter < 250) return 1.8f;
        if (distanceFromCenter < 350) return 1.3f;
        return 1.0f; // Starting difficulty at rim
    }
    
    /// <summary>
    /// Get loot quality multiplier based on distance from center
    /// </summary>
    public static float GetLootQualityMultiplier(int distanceFromCenter)
    {
        // Closer to center = better loot
        if (distanceFromCenter < 25) return 5.0f;
        if (distanceFromCenter < 50) return 3.5f;
        if (distanceFromCenter < 75) return 2.5f;
        if (distanceFromCenter < 150) return 2.0f;
        if (distanceFromCenter < 250) return 1.5f;
        if (distanceFromCenter < 350) return 1.2f;
        return 1.0f;
    }
    
    /// <summary>
    /// Get zone name for player location
    /// </summary>
    public static string GetZoneName(int distanceFromCenter)
    {
        if (distanceFromCenter < 25) return "Galactic Core (Avorion Zone)";
        if (distanceFromCenter < 50) return "Inner Core (Ogonite Zone)";
        if (distanceFromCenter < 75) return "Core Sectors (Xanion Zone)";
        if (distanceFromCenter < 150) return "Mid-Galaxy (Trinium Zone)";
        if (distanceFromCenter < 250) return "Outer Regions (Naonite Zone)";
        if (distanceFromCenter < 350) return "Frontier (Titanium Zone)";
        return "Galaxy Rim (Iron Zone)";
    }
    
    /// <summary>
    /// Check if player can access a zone (based on their highest material tier)
    /// </summary>
    public static bool CanAccessZone(MaterialTier playerHighestTier, int targetDistanceFromCenter)
    {
        var zoneTier = GetAvailableMaterialTier(targetDistanceFromCenter);
        
        // Can only go one tier deeper than your best material
        // This creates progression gates
        return (int)playerHighestTier >= (int)zoneTier - 1;
    }
    
    /// <summary>
    /// Get enemy spawn rate multiplier for zone
    /// </summary>
    public static float GetEnemySpawnRate(int distanceFromCenter)
    {
        // More enemies closer to center
        if (distanceFromCenter < 25) return 3.0f;
        if (distanceFromCenter < 50) return 2.5f;
        if (distanceFromCenter < 75) return 2.0f;
        if (distanceFromCenter < 150) return 1.5f;
        if (distanceFromCenter < 250) return 1.2f;
        return 1.0f;
    }
    
    /// <summary>
    /// Get recommended ship value (total materials worth) for zone
    /// </summary>
    public static int GetRecommendedShipValue(int distanceFromCenter)
    {
        if (distanceFromCenter < 25) return 10000000;  // 10M credits worth
        if (distanceFromCenter < 50) return 5000000;
        if (distanceFromCenter < 75) return 2000000;
        if (distanceFromCenter < 150) return 500000;
        if (distanceFromCenter < 250) return 100000;
        if (distanceFromCenter < 350) return 50000;
        return 10000; // Starter ship
    }
    
    /// <summary>
    /// Update player progression data
    /// </summary>
    private void UpdatePlayerProgression(PlayerProgressionComponent player)
    {
        // Get player's current sector
        var locationComp = _entityManager.GetComponent<SectorLocationComponent>(player.EntityId);
        if (locationComp == null) return;
        
        int distance = GetDistanceFromCenter(locationComp.CurrentSector);
        
        // Update furthest progress
        if (distance < player.ClosestDistanceToCenter)
        {
            player.ClosestDistanceToCenter = distance;
            player.FurthestZoneReached = GetZoneName(distance);
            
            // Award achievement/milestone
            Console.WriteLine($"🎉 New Zone Reached: {player.FurthestZoneReached}!");
        }
        
        // Update current zone info
        player.CurrentZone = GetZoneName(distance);
        player.CurrentZoneDifficulty = GetDifficultyMultiplier(distance);
        player.AvailableMaterialTier = GetAvailableMaterialTier(distance);
    }
}

/// <summary>
/// Component tracking player's progression through the galaxy
/// </summary>
public class PlayerProgressionComponent : IComponent
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// Closest distance player has reached to galactic center
    /// </summary>
    public int ClosestDistanceToCenter { get; set; } = int.MaxValue;
    
    /// <summary>
    /// Name of furthest zone player has reached
    /// </summary>
    public string FurthestZoneReached { get; set; } = "Galaxy Rim (Iron Zone)";
    
    /// <summary>
    /// Current zone player is in
    /// </summary>
    public string CurrentZone { get; set; } = "Galaxy Rim (Iron Zone)";
    
    /// <summary>
    /// Current zone difficulty multiplier
    /// </summary>
    public float CurrentZoneDifficulty { get; set; } = 1.0f;
    
    /// <summary>
    /// Highest material tier available in current zone
    /// </summary>
    public MaterialTier AvailableMaterialTier { get; set; } = MaterialTier.Iron;
    
    /// <summary>
    /// Highest material tier player has ever acquired
    /// </summary>
    public MaterialTier HighestMaterialTierAcquired { get; set; } = MaterialTier.Iron;
    
    /// <summary>
    /// Total sectors explored
    /// </summary>
    public int SectorsExplored { get; set; } = 0;
    
    /// <summary>
    /// Milestones achieved
    /// </summary>
    public HashSet<string> Milestones { get; set; } = new();
    
    /// <summary>
    /// Check if player has reached a specific zone
    /// </summary>
    public bool HasReachedZone(MaterialTier tier)
    {
        return (int)HighestMaterialTierAcquired >= (int)tier;
    }
}
