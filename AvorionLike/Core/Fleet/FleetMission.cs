using System.Numerics;
using System.Text.Json;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;
using AvorionLike.Core.RPG;

namespace AvorionLike.Core.Fleet;

/// <summary>
/// Types of missions that can be undertaken
/// </summary>
public enum MissionType
{
    Explore,        // Scout and reveal areas
    Mine,           // Gather resources
    Salvage,        // Break down wrecks
    Combat,         // Engage hostiles
    Trade,          // Trade goods
    Escort,         // Protect other ships
    Reconnaissance  // Covert operations
}

/// <summary>
/// Mission difficulty affects rewards and requirements
/// </summary>
public enum MissionDifficulty
{
    Easy = 1,
    Normal = 2,
    Hard = 3,
    VeryHard = 4,
    Extreme = 5
}

/// <summary>
/// Mission status
/// </summary>
public enum MissionStatus
{
    Pending,        // Not yet started
    InProgress,     // Ships are on mission
    Completed,      // Successfully completed
    Failed,         // Mission failed
    Aborted         // Manually cancelled
}

/// <summary>
/// Represents a mission that ships can be sent on
/// </summary>
public class FleetMission
{
    public Guid Id { get; set; }
    public string Name { get; set; }
    public string Description { get; set; }
    public MissionType Type { get; set; }
    public MissionDifficulty Difficulty { get; set; }
    public MissionStatus Status { get; set; }
    
    // Location
    public Vector3 GalaxyPosition { get; set; }
    public string SectorName { get; set; }
    
    // Ships assigned to mission
    public List<Guid> AssignedShipIds { get; set; } = new();
    public int MinShips { get; set; } = 1;
    public int MaxShips { get; set; } = 4;
    
    // Time
    public float Duration { get; set; } // In game hours
    public DateTime StartTime { get; set; }
    public DateTime? CompletionTime { get; set; }
    
    // Requirements
    public ShipClassType? PreferredClass { get; set; }
    public int MinimumCombatRating { get; set; }
    public int MinimumCargoCapacity { get; set; }
    
    // Rewards
    public Dictionary<string, int> GuaranteedResources { get; set; } = new();
    public List<SubsystemUpgrade> RewardSubsystems { get; set; } = new();
    public List<string> RewardBlueprints { get; set; } = new();
    public int CreditsReward { get; set; }
    public int ExperienceReward { get; set; }
    
    // Mission results
    public bool WasSuccessful { get; set; }
    public float SuccessRate { get; set; } // 0-1, calculated based on ship stats
    public string ResultMessage { get; set; } = "";
    
    public FleetMission(MissionType type, Vector3 position, string sectorName)
    {
        Id = Guid.NewGuid();
        Type = type;
        GalaxyPosition = position;
        SectorName = sectorName;
        Status = MissionStatus.Pending;
        
        Name = GenerateMissionName(type);
        Description = GenerateMissionDescription(type);
        
        // Set defaults based on type
        InitializeMissionDefaults();
    }
    
    /// <summary>
    /// Initialize default values based on mission type
    /// </summary>
    private void InitializeMissionDefaults()
    {
        switch (Type)
        {
            case MissionType.Explore:
                Duration = 2.0f;
                PreferredClass = ShipClassType.Exploration;
                MinimumCombatRating = 100;
                MinimumCargoCapacity = 0;
                break;
                
            case MissionType.Mine:
                Duration = 4.0f;
                PreferredClass = ShipClassType.Industrial;
                MinimumCombatRating = 50;
                MinimumCargoCapacity = 1000;
                break;
                
            case MissionType.Salvage:
                Duration = 3.0f;
                PreferredClass = ShipClassType.Salvaging;
                MinimumCombatRating = 75;
                MinimumCargoCapacity = 800;
                break;
                
            case MissionType.Combat:
                Duration = 2.5f;
                PreferredClass = ShipClassType.Combat;
                MinimumCombatRating = 300;
                MinimumCargoCapacity = 0;
                MinShips = 2;
                break;
                
            case MissionType.Reconnaissance:
                Duration = 3.0f;
                PreferredClass = ShipClassType.Covert;
                MinimumCombatRating = 150;
                MinimumCargoCapacity = 0;
                MaxShips = 1; // Solo missions for covert ops
                break;
                
            case MissionType.Trade:
                Duration = 3.5f;
                PreferredClass = null; // Any class can trade
                MinimumCombatRating = 50;
                MinimumCargoCapacity = 500;
                break;
                
            case MissionType.Escort:
                Duration = 4.0f;
                PreferredClass = ShipClassType.Combat;
                MinimumCombatRating = 200;
                MinimumCargoCapacity = 0;
                MinShips = 2; // Need escort ships
                break;
                
            default:
                Duration = 2.0f;
                MinimumCombatRating = 100;
                MinimumCargoCapacity = 0;
                break;
        }
    }
    
    /// <summary>
    /// Calculate success rate based on assigned ships
    /// </summary>
    public float CalculateSuccessRate(EntityManager entityManager)
    {
        if (AssignedShipIds.Count == 0)
            return 0f;
        
        float totalRating = 0f;
        int shipCount = 0;
        
        foreach (var shipId in AssignedShipIds)
        {
            var shipClass = entityManager.GetComponent<ShipClassComponent>(shipId);
            var subsystems = entityManager.GetComponent<ShipSubsystemComponent>(shipId);
            var voxel = entityManager.GetComponent<Voxel.VoxelStructureComponent>(shipId);
            
            if (shipClass == null) continue;
            
            shipCount++;
            float shipRating = CalculateShipRatingForMission(shipClass, subsystems, voxel);
            totalRating += shipRating;
        }
        
        if (shipCount == 0)
            return 0f;
        
        // Average rating across all ships
        float avgRating = totalRating / shipCount;
        
        // Base success rate from ship ratings
        float baseSuccess = Math.Min(0.95f, avgRating / 100f);
        
        // Difficulty modifier
        float difficultyPenalty = (int)Difficulty * 0.1f;
        
        // Ship count bonus (more ships = better odds for some missions)
        float countBonus = Type == MissionType.Combat ? (shipCount - 1) * 0.1f : 0f;
        
        SuccessRate = Math.Clamp(baseSuccess - difficultyPenalty + countBonus, 0.1f, 0.95f);
        return SuccessRate;
    }
    
    /// <summary>
    /// Calculate how well a ship is suited for this mission
    /// </summary>
    private float CalculateShipRatingForMission(
        ShipClassComponent shipClass, 
        ShipSubsystemComponent? subsystems,
        Voxel.VoxelStructureComponent? voxel)
    {
        float rating = 50f; // Base rating
        
        // Class match bonus
        if (PreferredClass.HasValue && shipClass.ShipClass == PreferredClass.Value)
        {
            rating += 30f * shipClass.ClassBonusMultiplier;
        }
        
        // Specialization level bonus
        rating += shipClass.SpecializationLevel * 5f;
        
        // Readiness penalty
        rating *= (shipClass.MissionReadiness / 100f);
        
        // Subsystem bonuses
        if (subsystems != null)
        {
            var equippedSubsystems = subsystems.GetEquippedSubsystems();
            rating += equippedSubsystems.Count * 5f;
            
            // Count relevant subsystems for mission type
            int relevantCount = 0;
            foreach (var sub in equippedSubsystems)
            {
                if (IsSubsystemRelevantForMission(sub.Type))
                {
                    relevantCount++;
                    rating += sub.GetTotalBonus() * 20f;
                }
            }
        }
        
        // Ship power and capabilities
        if (voxel != null)
        {
            rating += voxel.TotalThrust * 0.01f;
            rating += voxel.PowerGeneration * 0.05f;
            rating += voxel.ShieldCapacity * 0.02f;
        }
        
        return rating;
    }
    
    /// <summary>
    /// Check if a subsystem type is relevant for this mission
    /// </summary>
    private bool IsSubsystemRelevantForMission(SubsystemType type)
    {
        return Type switch
        {
            MissionType.Explore => type == SubsystemType.ScannerArray || 
                                  type == SubsystemType.JumpDriveEnhancer ||
                                  type == SubsystemType.ThrustAmplifier,
                                  
            MissionType.Mine => type == SubsystemType.CargoExpansion || 
                               type == SubsystemType.PowerAmplifier,
                               
            MissionType.Salvage => type == SubsystemType.CargoExpansion || 
                                  type == SubsystemType.PowerAmplifier,
                                  
            MissionType.Combat => type == SubsystemType.WeaponAmplifier || 
                                 type == SubsystemType.ShieldBooster ||
                                 type == SubsystemType.ArmorPlating ||
                                 type == SubsystemType.TargetingComputer,
                                 
            MissionType.Reconnaissance => type == SubsystemType.ScannerArray || 
                                         type == SubsystemType.ThrustAmplifier ||
                                         type == SubsystemType.PowerEfficiency,
                                         
            _ => false
        };
    }
    
    /// <summary>
    /// Generate mission name based on type
    /// </summary>
    private static string GenerateMissionName(MissionType type)
    {
        return type switch
        {
            MissionType.Explore => "Exploration Mission",
            MissionType.Mine => "Mining Operation",
            MissionType.Salvage => "Salvage Operation",
            MissionType.Combat => "Combat Patrol",
            MissionType.Trade => "Trade Run",
            MissionType.Escort => "Escort Duty",
            MissionType.Reconnaissance => "Reconnaissance Mission",
            _ => "Unknown Mission"
        };
    }
    
    /// <summary>
    /// Generate mission description
    /// </summary>
    private static string GenerateMissionDescription(MissionType type)
    {
        return type switch
        {
            MissionType.Explore => "Scout the designated sector and report findings.",
            MissionType.Mine => "Extract valuable resources from asteroid fields.",
            MissionType.Salvage => "Salvage materials and technology from debris fields.",
            MissionType.Combat => "Engage hostile forces in the sector.",
            MissionType.Trade => "Transport goods and establish trade connections.",
            MissionType.Escort => "Protect allied vessels during their operations.",
            MissionType.Reconnaissance => "Gather intelligence on enemy positions without being detected.",
            _ => "Complete the assigned objectives."
        };
    }
    
    /// <summary>
    /// Generate rewards based on mission success
    /// </summary>
    public void GenerateRewards(bool successful, EntityManager entityManager)
    {
        WasSuccessful = successful;
        
        if (!successful)
        {
            // Failed missions give reduced rewards
            CreditsReward = (int)(CreditsReward * 0.3f);
            ExperienceReward = (int)(ExperienceReward * 0.5f);
            ResultMessage = "Mission failed. Minimal rewards recovered.";
            return;
        }
        
        // Base rewards by difficulty
        int baseCreditReward = (int)Difficulty * 1000;
        int baseXPReward = (int)Difficulty * 500;
        
        CreditsReward = (int)(baseCreditReward * SuccessRate);
        ExperienceReward = (int)(baseXPReward * SuccessRate);
        
        // Type-specific rewards
        switch (Type)
        {
            case MissionType.Mine:
                GenerateMiningRewards();
                break;
            case MissionType.Salvage:
                GenerateSalvageRewards();
                break;
            case MissionType.Combat:
                GenerateCombatRewards();
                break;
            case MissionType.Explore:
                GenerateExplorationRewards();
                break;
            case MissionType.Reconnaissance:
                GenerateReconnaissanceRewards();
                break;
            case MissionType.Trade:
                GenerateTradeRewards();
                break;
            case MissionType.Escort:
                GenerateEscortRewards();
                break;
        }
        
        ResultMessage = "Mission completed successfully!";
    }
    
    /// <summary>
    /// Generate mining rewards
    /// </summary>
    private void GenerateMiningRewards()
    {
        var materials = new[] { "Iron", "Titanium", "Naonite", "Trinium", "Xanion" };
        int materialIndex = Math.Min((int)Difficulty - 1, materials.Length - 1);
        
        GuaranteedResources[materials[materialIndex]] = 500 * (int)Difficulty;
        
        // Chance for blueprint
        if (Random.Shared.NextDouble() < 0.2)
        {
            RewardBlueprints.Add("Industrial Hull Enhancement");
        }
    }
    
    /// <summary>
    /// Generate salvage rewards
    /// </summary>
    private void GenerateSalvageRewards()
    {
        // Salvage gives mixed materials
        GuaranteedResources["Iron"] = 200 * (int)Difficulty;
        GuaranteedResources["Titanium"] = 150 * (int)Difficulty;
        
        // High chance for subsystems
        if (Random.Shared.NextDouble() < 0.6)
        {
            var rarity = RollSubsystemRarity();
            var subsystem = ClassSpecificSubsystemGenerator.GenerateForClass(
                ShipClassType.Salvaging, rarity);
            RewardSubsystems.Add(subsystem);
        }
        
        // Chance for blueprint
        if (Random.Shared.NextDouble() < 0.3)
        {
            RewardBlueprints.Add("Salvage Beam Enhancement");
        }
    }
    
    /// <summary>
    /// Generate combat rewards
    /// </summary>
    private void GenerateCombatRewards()
    {
        // Combat gives credits and subsystems
        CreditsReward = (int)(CreditsReward * 1.5f);
        
        // Good chance for combat subsystems
        if (Random.Shared.NextDouble() < 0.7)
        {
            var rarity = RollSubsystemRarity();
            var subsystem = ClassSpecificSubsystemGenerator.GenerateForClass(
                ShipClassType.Combat, rarity);
            RewardSubsystems.Add(subsystem);
        }
        
        // Chance for weapon blueprint
        if (Random.Shared.NextDouble() < 0.4)
        {
            RewardBlueprints.Add("Advanced Weapon System");
        }
    }
    
    /// <summary>
    /// Generate exploration rewards
    /// </summary>
    private void GenerateExplorationRewards()
    {
        // Exploration gives XP bonus and discoveries
        ExperienceReward = (int)(ExperienceReward * 1.5f);
        
        // Chance for subsystem
        if (Random.Shared.NextDouble() < 0.5)
        {
            var rarity = RollSubsystemRarity();
            var subsystem = ClassSpecificSubsystemGenerator.GenerateForClass(
                ShipClassType.Exploration, rarity);
            RewardSubsystems.Add(subsystem);
        }
        
        // Good chance for blueprints
        if (Random.Shared.NextDouble() < 0.5)
        {
            RewardBlueprints.Add("Advanced Scanner Array");
        }
    }
    
    /// <summary>
    /// Generate reconnaissance rewards
    /// </summary>
    private void GenerateReconnaissanceRewards()
    {
        // Covert missions give high credits and intel
        CreditsReward = (int)(CreditsReward * 2.0f);
        
        // Chance for rare covert subsystems
        if (Random.Shared.NextDouble() < 0.6)
        {
            var rarity = RollSubsystemRarity();
            var subsystem = ClassSpecificSubsystemGenerator.GenerateForClass(
                ShipClassType.Covert, rarity);
            RewardSubsystems.Add(subsystem);
        }
        
        // Chance for cloaking blueprint
        if (Random.Shared.NextDouble() < 0.3)
        {
            RewardBlueprints.Add("Cloaking Device Prototype");
        }
    }
    
    /// <summary>
    /// Generate trade rewards
    /// </summary>
    private void GenerateTradeRewards()
    {
        // Trade missions give high credits
        CreditsReward = (int)(CreditsReward * 1.8f);
        ExperienceReward = (int)(ExperienceReward * 1.2f);
        
        // Chance for trade-related blueprints
        if (Random.Shared.NextDouble() < 0.3)
        {
            RewardBlueprints.Add("Trade Hub Upgrade");
        }
    }
    
    /// <summary>
    /// Generate escort rewards
    /// </summary>
    private void GenerateEscortRewards()
    {
        // Escort missions give steady credits and combat XP
        CreditsReward = (int)(CreditsReward * 1.3f);
        ExperienceReward = (int)(ExperienceReward * 1.4f);
        
        // Chance for defensive subsystems
        if (Random.Shared.NextDouble() < 0.5)
        {
            var rarity = RollSubsystemRarity();
            var subsystem = ClassSpecificSubsystemGenerator.GenerateForClass(
                ShipClassType.Combat, rarity);
            RewardSubsystems.Add(subsystem);
        }
    }
    
    /// <summary>
    /// Roll for subsystem rarity based on mission difficulty
    /// </summary>
    private SubsystemRarity RollSubsystemRarity()
    {
        float roll = Random.Shared.NextSingle();
        float difficultyBonus = (int)Difficulty * 0.05f;
        
        // Higher difficulty = better drop rates
        if (roll < 0.01f + difficultyBonus) return SubsystemRarity.Legendary;
        if (roll < 0.05f + difficultyBonus) return SubsystemRarity.Epic;
        if (roll < 0.15f + difficultyBonus) return SubsystemRarity.Rare;
        if (roll < 0.40f + difficultyBonus) return SubsystemRarity.Uncommon;
        return SubsystemRarity.Common;
    }
    
    /// <summary>
    /// Serialize the mission
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        var rewardSubsystemsData = RewardSubsystems.Select(s => s.Serialize()).ToList();
        
        return new Dictionary<string, object>
        {
            ["Id"] = Id.ToString(),
            ["Name"] = Name,
            ["Description"] = Description,
            ["Type"] = Type.ToString(),
            ["Difficulty"] = Difficulty.ToString(),
            ["Status"] = Status.ToString(),
            ["GalaxyPosition"] = new Dictionary<string, object>
            {
                ["X"] = GalaxyPosition.X,
                ["Y"] = GalaxyPosition.Y,
                ["Z"] = GalaxyPosition.Z
            },
            ["SectorName"] = SectorName,
            ["AssignedShipIds"] = AssignedShipIds.Select(id => id.ToString()).ToList(),
            ["MinShips"] = MinShips,
            ["MaxShips"] = MaxShips,
            ["Duration"] = Duration,
            ["StartTime"] = StartTime.ToString("O"),
            ["CompletionTime"] = CompletionTime?.ToString("O") ?? "",
            ["PreferredClass"] = PreferredClass?.ToString() ?? "",
            ["MinimumCombatRating"] = MinimumCombatRating,
            ["MinimumCargoCapacity"] = MinimumCargoCapacity,
            ["GuaranteedResources"] = GuaranteedResources,
            ["RewardSubsystems"] = rewardSubsystemsData,
            ["RewardBlueprints"] = RewardBlueprints,
            ["CreditsReward"] = CreditsReward,
            ["ExperienceReward"] = ExperienceReward,
            ["WasSuccessful"] = WasSuccessful,
            ["SuccessRate"] = SuccessRate,
            ["ResultMessage"] = ResultMessage
        };
    }
}
