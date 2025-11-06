using AvorionLike.Core.ECS;
using AvorionLike.Core.RPG;
using AvorionLike.Core.Resources;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.Fleet;

/// <summary>
/// System for managing fleet missions and ship assignments
/// </summary>
public class FleetMissionSystem
{
    private readonly EntityManager _entityManager;
    private readonly List<FleetMission> _activeMissions = new();
    private readonly List<FleetMission> _completedMissions = new();
    private readonly List<FleetMission> _availableMissions = new();
    
    public IReadOnlyList<FleetMission> ActiveMissions => _activeMissions.AsReadOnly();
    public IReadOnlyList<FleetMission> CompletedMissions => _completedMissions.AsReadOnly();
    public IReadOnlyList<FleetMission> AvailableMissions => _availableMissions.AsReadOnly();
    
    public FleetMissionSystem(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }
    
    /// <summary>
    /// Generate available missions based on player progress
    /// </summary>
    public void GenerateMissions(int count, int playerLevel)
    {
        for (int i = 0; i < count; i++)
        {
            var missionType = (MissionType)Random.Shared.Next(0, Enum.GetValues<MissionType>().Length);
            var difficulty = DetermineDifficulty(playerLevel);
            
            var mission = new FleetMission(
                missionType,
                new System.Numerics.Vector3(
                    Random.Shared.Next(-1000, 1000),
                    Random.Shared.Next(-1000, 1000),
                    Random.Shared.Next(-1000, 1000)
                ),
                $"Sector {Random.Shared.Next(1000, 9999)}"
            );
            
            mission.Difficulty = difficulty;
            _availableMissions.Add(mission);
        }
    }
    
    /// <summary>
    /// Determine mission difficulty based on player level
    /// </summary>
    private MissionDifficulty DetermineDifficulty(int playerLevel)
    {
        float roll = Random.Shared.NextSingle();
        
        if (playerLevel < 5)
        {
            return roll < 0.7f ? MissionDifficulty.Easy : MissionDifficulty.Normal;
        }
        else if (playerLevel < 15)
        {
            if (roll < 0.3f) return MissionDifficulty.Easy;
            if (roll < 0.7f) return MissionDifficulty.Normal;
            return MissionDifficulty.Hard;
        }
        else if (playerLevel < 30)
        {
            if (roll < 0.2f) return MissionDifficulty.Normal;
            if (roll < 0.6f) return MissionDifficulty.Hard;
            return MissionDifficulty.VeryHard;
        }
        else
        {
            if (roll < 0.3f) return MissionDifficulty.Hard;
            if (roll < 0.7f) return MissionDifficulty.VeryHard;
            return MissionDifficulty.Extreme;
        }
    }
    
    /// <summary>
    /// Assign ships to a mission and start it
    /// </summary>
    public bool StartMission(FleetMission mission, List<Guid> shipIds)
    {
        if (shipIds.Count < mission.MinShips || shipIds.Count > mission.MaxShips)
            return false;
        
        // Verify all ships can accept mission
        foreach (var shipId in shipIds)
        {
            var shipClass = _entityManager.GetComponent<ShipClassComponent>(shipId);
            if (shipClass == null || !shipClass.CanAcceptMission())
                return false;
        }
        
        // Assign ships
        mission.AssignedShipIds.AddRange(shipIds);
        
        // Calculate success rate
        mission.CalculateSuccessRate(_entityManager);
        
        // Update ship states
        foreach (var shipId in shipIds)
        {
            var shipClass = _entityManager.GetComponent<ShipClassComponent>(shipId);
            if (shipClass != null)
            {
                shipClass.IsOnMission = true;
                shipClass.CurrentMissionId = mission.Id;
            }
        }
        
        // Start mission
        mission.Status = MissionStatus.InProgress;
        mission.StartTime = DateTime.UtcNow;
        
        _availableMissions.Remove(mission);
        _activeMissions.Add(mission);
        
        return true;
    }
    
    /// <summary>
    /// Update mission progress (call periodically)
    /// </summary>
    public void Update(float deltaTime)
    {
        var completedThisFrame = new List<FleetMission>();
        
        foreach (var mission in _activeMissions)
        {
            if (mission.Status != MissionStatus.InProgress)
                continue;
            
            // Check if mission duration has elapsed (simplified time check)
            var elapsed = DateTime.UtcNow - mission.StartTime;
            if (elapsed.TotalMinutes >= mission.Duration * 10) // 10 real minutes per game hour
            {
                CompleteMission(mission);
                completedThisFrame.Add(mission);
            }
        }
        
        // Move completed missions
        foreach (var mission in completedThisFrame)
        {
            _activeMissions.Remove(mission);
            _completedMissions.Add(mission);
        }
    }
    
    /// <summary>
    /// Complete a mission and distribute rewards
    /// </summary>
    private void CompleteMission(FleetMission mission)
    {
        // Roll for success based on calculated rate
        bool successful = Random.Shared.NextSingle() < mission.SuccessRate;
        
        // Generate rewards
        mission.GenerateRewards(successful, _entityManager);
        mission.Status = successful ? MissionStatus.Completed : MissionStatus.Failed;
        mission.CompletionTime = DateTime.UtcNow;
        
        // Update ship states and grant rewards
        foreach (var shipId in mission.AssignedShipIds)
        {
            var shipClass = _entityManager.GetComponent<ShipClassComponent>(shipId);
            if (shipClass != null)
            {
                shipClass.IsOnMission = false;
                shipClass.CurrentMissionId = null;
                
                // Reduce readiness
                shipClass.ReduceReadiness(20f);
                
                // Grant specialization XP if successful
                if (successful)
                {
                    int xpGain = (int)mission.Difficulty * 100;
                    shipClass.AddSpecializationXP(xpGain);
                }
            }
        }
        
        // Add rewards to player inventory (find player entity)
        DistributeRewards(mission);
    }
    
    /// <summary>
    /// Distribute mission rewards to player
    /// </summary>
    private void DistributeRewards(FleetMission mission)
    {
        // Find player pod entity (or any entity with inventory)
        var entities = _entityManager.GetAllEntities();
        Entity? playerEntity = null;
        
        foreach (var entity in entities)
        {
            var pod = _entityManager.GetComponent<PlayerPodComponent>(entity.Id);
            if (pod != null)
            {
                playerEntity = entity;
                break;
            }
        }
        
        if (playerEntity == null)
        {
            // Find any entity with inventory as fallback
            playerEntity = entities.FirstOrDefault(e => 
                _entityManager.GetComponent<InventoryComponent>(e.Id) != null);
        }
        
        if (playerEntity == null)
            return;
        
        var inventory = _entityManager.GetComponent<InventoryComponent>(playerEntity.Id);
        if (inventory == null)
            return;
        
        // Add resources
        foreach (var resource in mission.GuaranteedResources)
        {
            if (Enum.TryParse<ResourceType>(resource.Key, out var resourceType))
            {
                inventory.Inventory.AddResource(resourceType, resource.Value);
            }
        }
        
        // Add credits
        if (mission.CreditsReward > 0)
        {
            inventory.Inventory.AddResource(ResourceType.Credits, mission.CreditsReward);
        }
        
        // TODO: Store subsystems and blueprints in a dedicated storage system
        // For now, they're stored in the mission object for retrieval
    }
    
    /// <summary>
    /// Abort a mission early
    /// </summary>
    public bool AbortMission(Guid missionId)
    {
        var mission = _activeMissions.FirstOrDefault(m => m.Id == missionId);
        if (mission == null)
            return false;
        
        mission.Status = MissionStatus.Aborted;
        mission.CompletionTime = DateTime.UtcNow;
        
        // Return ships to fleet
        foreach (var shipId in mission.AssignedShipIds)
        {
            var shipClass = _entityManager.GetComponent<ShipClassComponent>(shipId);
            if (shipClass != null)
            {
                shipClass.IsOnMission = false;
                shipClass.CurrentMissionId = null;
                shipClass.ReduceReadiness(10f); // Small penalty
            }
        }
        
        _activeMissions.Remove(mission);
        return true;
    }
    
    /// <summary>
    /// Get all ships available for missions (not on mission, good readiness)
    /// </summary>
    public List<Guid> GetAvailableShips()
    {
        var availableShips = new List<Guid>();
        var entities = _entityManager.GetAllEntities();
        
        foreach (var entity in entities)
        {
            var shipClass = _entityManager.GetComponent<ShipClassComponent>(entity.Id);
            if (shipClass != null && shipClass.CanAcceptMission())
            {
                availableShips.Add(entity.Id);
            }
        }
        
        return availableShips;
    }
    
    /// <summary>
    /// Restore ship readiness over time
    /// </summary>
    public void RestoreShipReadiness(float deltaTime)
    {
        var entities = _entityManager.GetAllEntities();
        
        foreach (var entity in entities)
        {
            var shipClass = _entityManager.GetComponent<ShipClassComponent>(entity.Id);
            if (shipClass != null && !shipClass.IsOnMission)
            {
                // Restore 5% per minute
                shipClass.RestoreReadiness(deltaTime * 5f / 60f);
            }
        }
    }
}

/// <summary>
/// Component for storing subsystem inventory
/// </summary>
public class SubsystemInventoryComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    public List<RPG.SubsystemUpgrade> StoredSubsystems { get; set; } = new();
    public int MaxStorage { get; set; } = 50;
    
    /// <summary>
    /// Add a subsystem to inventory
    /// </summary>
    public bool AddSubsystem(RPG.SubsystemUpgrade subsystem)
    {
        if (StoredSubsystems.Count >= MaxStorage)
            return false;
        
        StoredSubsystems.Add(subsystem);
        return true;
    }
    
    /// <summary>
    /// Remove a subsystem from inventory
    /// </summary>
    public bool RemoveSubsystem(Guid subsystemId)
    {
        var subsystem = StoredSubsystems.FirstOrDefault(s => s.Id == subsystemId);
        if (subsystem == null)
            return false;
        
        StoredSubsystems.Remove(subsystem);
        return true;
    }
    
    /// <summary>
    /// Get subsystems by type
    /// </summary>
    public List<RPG.SubsystemUpgrade> GetSubsystemsByType(SubsystemType type)
    {
        return StoredSubsystems.Where(s => s.Type == type).ToList();
    }
    
    /// <summary>
    /// Get subsystems by rarity
    /// </summary>
    public List<RPG.SubsystemUpgrade> GetSubsystemsByRarity(SubsystemRarity rarity)
    {
        return StoredSubsystems.Where(s => s.Rarity == rarity).ToList();
    }
    
    /// <summary>
    /// Serialize the component
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        var subsystemsData = StoredSubsystems.Select(s => s.Serialize()).ToList();
        
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["StoredSubsystems"] = subsystemsData,
            ["MaxStorage"] = MaxStorage
        };
    }
    
    /// <summary>
    /// Deserialize the component
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(data["EntityId"].ToString()!);
        MaxStorage = Convert.ToInt32(data["MaxStorage"]);
        
        StoredSubsystems.Clear();
        
        if (data.ContainsKey("StoredSubsystems"))
        {
            var subsystemsData = data["StoredSubsystems"];
            
            if (subsystemsData is System.Text.Json.JsonElement jsonElement && 
                jsonElement.ValueKind == System.Text.Json.JsonValueKind.Array)
            {
                foreach (var subsystemElement in jsonElement.EnumerateArray())
                {
                    var subsystemDict = System.Text.Json.JsonSerializer.Deserialize<Dictionary<string, object>>(
                        subsystemElement.GetRawText());
                    if (subsystemDict != null)
                    {
                        StoredSubsystems.Add(RPG.SubsystemUpgrade.Deserialize(subsystemDict));
                    }
                }
            }
            else if (subsystemsData is List<object> subsystemsList)
            {
                foreach (var subsystemObj in subsystemsList)
                {
                    if (subsystemObj is Dictionary<string, object> subsystemDict)
                    {
                        StoredSubsystems.Add(RPG.SubsystemUpgrade.Deserialize(subsystemDict));
                    }
                }
            }
        }
    }
}
