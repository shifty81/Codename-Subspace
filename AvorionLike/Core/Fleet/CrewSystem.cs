using System.Text.Json;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.Fleet;

/// <summary>
/// Represents a pilot that can fly ships
/// </summary>
public class Pilot
{
    public Guid Id { get; set; }
    public string Name { get; set; }
    public int Level { get; set; }
    public int Experience { get; set; }
    
    // Pilot skills affect ship performance
    public float CombatSkill { get; set; }      // 0-1, affects weapon accuracy
    public float NavigationSkill { get; set; }  // 0-1, affects maneuverability
    public float EngineeringSkill { get; set; } // 0-1, affects power efficiency
    
    // Specialization
    public ShipClassType? Specialization { get; set; }
    
    // Hiring cost
    public int HiringCost { get; set; }
    public int DailySalary { get; set; }
    
    // Assignment
    public Guid? AssignedShipId { get; set; }
    public bool IsAssigned => AssignedShipId.HasValue;
    
    public Pilot(string name, int level = 1)
    {
        Id = Guid.NewGuid();
        Name = name;
        Level = level;
        Experience = 0;
        
        // Random initial skills based on level
        CombatSkill = 0.3f + (Random.Shared.NextSingle() * 0.3f) + (level * 0.05f);
        NavigationSkill = 0.3f + (Random.Shared.NextSingle() * 0.3f) + (level * 0.05f);
        EngineeringSkill = 0.3f + (Random.Shared.NextSingle() * 0.3f) + (level * 0.05f);
        
        // Clamp skills to 0-1
        CombatSkill = Math.Clamp(CombatSkill, 0f, 1f);
        NavigationSkill = Math.Clamp(NavigationSkill, 0f, 1f);
        EngineeringSkill = Math.Clamp(EngineeringSkill, 0f, 1f);
        
        // Maybe specialize
        if (Random.Shared.NextDouble() < 0.3) // 30% chance
        {
            var classes = Enum.GetValues<ShipClassType>().Where(c => c != ShipClassType.Undefined).ToArray();
            Specialization = classes[Random.Shared.Next(classes.Length)];
        }
        
        // Calculate costs
        HiringCost = 1000 * level + Random.Shared.Next(500, 2000);
        DailySalary = 100 * level + Random.Shared.Next(50, 200);
    }
    
    /// <summary>
    /// Generate a random pilot name
    /// </summary>
    public static string GenerateRandomName()
    {
        string[] firstNames = new[]
        {
            "Alex", "Jordan", "Taylor", "Morgan", "Casey", "Riley", "Avery", "Quinn",
            "Blake", "Cameron", "Dakota", "Elliot", "Finley", "Harper", "Jamie", "Kai",
            "Logan", "Mika", "Nova", "Pax", "Raven", "Sage", "Sam", "Skylar"
        };
        
        string[] lastNames = new[]
        {
            "Chen", "Garcia", "Patel", "Kim", "Martinez", "Singh", "Rodriguez", "Nguyen",
            "Johnson", "Smith", "Williams", "Brown", "Jones", "Miller", "Davis", "Wilson",
            "Anderson", "Taylor", "Thomas", "Moore", "Jackson", "Martin", "Lee", "Walker"
        };
        
        return $"{firstNames[Random.Shared.Next(firstNames.Length)]} {lastNames[Random.Shared.Next(lastNames.Length)]}";
    }
    
    /// <summary>
    /// Get the overall skill rating
    /// </summary>
    public float GetOverallSkill()
    {
        return (CombatSkill + NavigationSkill + EngineeringSkill) / 3f;
    }
    
    /// <summary>
    /// Add experience and level up if threshold reached
    /// </summary>
    public bool AddExperience(int xp)
    {
        Experience += xp;
        int xpNeeded = Level * 500;
        
        if (Experience >= xpNeeded)
        {
            Experience -= xpNeeded;
            Level++;
            
            // Improve random skill
            float improvement = 0.02f;
            int skillToImprove = Random.Shared.Next(3);
            
            switch (skillToImprove)
            {
                case 0:
                    CombatSkill = Math.Min(1f, CombatSkill + improvement);
                    break;
                case 1:
                    NavigationSkill = Math.Min(1f, NavigationSkill + improvement);
                    break;
                case 2:
                    EngineeringSkill = Math.Min(1f, EngineeringSkill + improvement);
                    break;
            }
            
            return true;
        }
        
        return false;
    }
    
    /// <summary>
    /// Serialize the pilot
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["Id"] = Id.ToString(),
            ["Name"] = Name,
            ["Level"] = Level,
            ["Experience"] = Experience,
            ["CombatSkill"] = CombatSkill,
            ["NavigationSkill"] = NavigationSkill,
            ["EngineeringSkill"] = EngineeringSkill,
            ["Specialization"] = Specialization?.ToString() ?? "",
            ["HiringCost"] = HiringCost,
            ["DailySalary"] = DailySalary,
            ["AssignedShipId"] = AssignedShipId?.ToString() ?? ""
        };
    }
    
    /// <summary>
    /// Deserialize a pilot
    /// </summary>
    public static Pilot Deserialize(Dictionary<string, object> data)
    {
        var pilot = new Pilot(data["Name"].ToString()!, 1)
        {
            Id = Guid.Parse(data["Id"].ToString()!),
            Level = Convert.ToInt32(data["Level"]),
            Experience = Convert.ToInt32(data["Experience"]),
            CombatSkill = Convert.ToSingle(data["CombatSkill"]),
            NavigationSkill = Convert.ToSingle(data["NavigationSkill"]),
            EngineeringSkill = Convert.ToSingle(data["EngineeringSkill"]),
            HiringCost = Convert.ToInt32(data["HiringCost"]),
            DailySalary = Convert.ToInt32(data["DailySalary"])
        };
        
        var specializationStr = data["Specialization"].ToString();
        if (!string.IsNullOrEmpty(specializationStr))
        {
            pilot.Specialization = Enum.Parse<ShipClassType>(specializationStr);
        }
        
        var shipIdStr = data["AssignedShipId"].ToString();
        if (!string.IsNullOrEmpty(shipIdStr))
        {
            pilot.AssignedShipId = Guid.Parse(shipIdStr);
        }
        
        return pilot;
    }
}

/// <summary>
/// Component for managing crew and pilot requirements on ships
/// </summary>
public class CrewComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    // Crew requirements
    public int MinimumCrew { get; set; }
    public int CurrentCrew { get; set; }
    public int MaxCrew { get; set; }
    
    // Crew quarters capacity (from CrewQuarters blocks)
    public int CrewQuartersCapacity { get; set; }
    
    // Pilot
    public Pilot? AssignedPilot { get; set; }
    
    // Crew efficiency (affected by crew level vs requirement)
    public float CrewEfficiency { get; set; } = 1.0f;
    
    public CrewComponent()
    {
        MinimumCrew = 1;
        CurrentCrew = 0;
        MaxCrew = 10;
        CrewQuartersCapacity = 10;
    }
    
    /// <summary>
    /// Calculate minimum crew based on ship configuration
    /// </summary>
    public void CalculateMinimumCrew(Voxel.VoxelStructureComponent voxelStructure)
    {
        if (voxelStructure == null)
        {
            MinimumCrew = 1;
            return;
        }
        
        int crewNeeded = 0;
        
        // Base crew for ship size
        crewNeeded += Math.Max(1, voxelStructure.Blocks.Count / 100);
        
        // Crew for functional blocks
        foreach (var block in voxelStructure.Blocks)
        {
            switch (block.BlockType)
            {
                case Voxel.BlockType.Engine:
                case Voxel.BlockType.Thruster:
                    crewNeeded += 1;
                    break;
                case Voxel.BlockType.Generator:
                    crewNeeded += 2;
                    break;
                case Voxel.BlockType.ShieldGenerator:
                    crewNeeded += 2;
                    break;
                case Voxel.BlockType.TurretMount:
                    crewNeeded += 3;
                    break;
                case Voxel.BlockType.HyperdriveCore:
                    crewNeeded += 5;
                    break;
            }
        }
        
        MinimumCrew = Math.Max(1, crewNeeded);
        
        // Calculate crew quarters capacity
        CrewQuartersCapacity = voxelStructure.GetBlocksByType(Voxel.BlockType.CrewQuarters).Count() * 10;
        MaxCrew = Math.Max(MinimumCrew, CrewQuartersCapacity);
    }
    
    /// <summary>
    /// Check if ship has sufficient crew
    /// </summary>
    public bool HasSufficientCrew()
    {
        return CurrentCrew >= MinimumCrew;
    }
    
    /// <summary>
    /// Check if ship has a pilot
    /// </summary>
    public bool HasPilot()
    {
        return AssignedPilot != null;
    }
    
    /// <summary>
    /// Check if ship is operational (has pilot and crew)
    /// </summary>
    public bool IsOperational()
    {
        return HasPilot() && HasSufficientCrew();
    }
    
    /// <summary>
    /// Check if ship can be controlled by player pod (overrides pilot requirement)
    /// </summary>
    public bool CanBeControlledByPod(Guid podEntityId, EntityManager entityManager)
    {
        var docking = entityManager.GetComponent<RPG.DockingComponent>(EntityId);
        if (docking == null) return false;
        
        return docking.DockedPodId == podEntityId;
    }
    
    /// <summary>
    /// Assign a pilot to this ship
    /// </summary>
    public bool AssignPilot(Pilot pilot)
    {
        if (pilot.IsAssigned)
            return false;
        
        AssignedPilot = pilot;
        pilot.AssignedShipId = EntityId;
        return true;
    }
    
    /// <summary>
    /// Remove pilot from this ship
    /// </summary>
    public Pilot? RemovePilot()
    {
        if (AssignedPilot == null)
            return null;
        
        var pilot = AssignedPilot;
        pilot.AssignedShipId = null;
        AssignedPilot = null;
        return pilot;
    }
    
    /// <summary>
    /// Add crew members
    /// </summary>
    public bool AddCrew(int count)
    {
        if (CurrentCrew + count > MaxCrew)
            return false;
        
        CurrentCrew += count;
        UpdateCrewEfficiency();
        return true;
    }
    
    /// <summary>
    /// Remove crew members
    /// </summary>
    public bool RemoveCrew(int count)
    {
        if (CurrentCrew - count < 0)
            return false;
        
        CurrentCrew -= count;
        UpdateCrewEfficiency();
        return true;
    }
    
    /// <summary>
    /// Update crew efficiency based on crew levels
    /// </summary>
    private void UpdateCrewEfficiency()
    {
        if (CurrentCrew < MinimumCrew)
        {
            // Undermanned - reduced efficiency
            CrewEfficiency = (float)CurrentCrew / MinimumCrew;
        }
        else if (CurrentCrew > MinimumCrew)
        {
            // Overmanned - slight bonus up to 20%
            float bonus = Math.Min(0.2f, (CurrentCrew - MinimumCrew) * 0.02f);
            CrewEfficiency = 1.0f + bonus;
        }
        else
        {
            // Exactly at minimum
            CrewEfficiency = 1.0f;
        }
    }
    
    /// <summary>
    /// Get crew efficiency (0-1.2)
    /// </summary>
    public float GetCrewEfficiency()
    {
        return CrewEfficiency;
    }
    
    /// <summary>
    /// Serialize the component
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["MinimumCrew"] = MinimumCrew,
            ["CurrentCrew"] = CurrentCrew,
            ["MaxCrew"] = MaxCrew,
            ["CrewQuartersCapacity"] = CrewQuartersCapacity,
            ["AssignedPilot"] = (object?)AssignedPilot?.Serialize() ?? DBNull.Value,
            ["CrewEfficiency"] = CrewEfficiency
        };
    }
    
    /// <summary>
    /// Deserialize the component
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(data["EntityId"].ToString() ?? string.Empty);
        MinimumCrew = Convert.ToInt32(data["MinimumCrew"]);
        CurrentCrew = Convert.ToInt32(data["CurrentCrew"]);
        MaxCrew = Convert.ToInt32(data["MaxCrew"]);
        CrewQuartersCapacity = Convert.ToInt32(data["CrewQuartersCapacity"]);
        CrewEfficiency = Convert.ToSingle(data["CrewEfficiency"]);
        
        if (data.ContainsKey("AssignedPilot") && data["AssignedPilot"] != null)
        {
            var pilotData = data["AssignedPilot"];
            
            if (pilotData is JsonElement jsonElement && jsonElement.ValueKind != JsonValueKind.Null)
            {
                var pilotDict = JsonSerializer.Deserialize<Dictionary<string, object>>(jsonElement.GetRawText());
                if (pilotDict != null)
                {
                    AssignedPilot = Pilot.Deserialize(pilotDict);
                }
            }
            else if (pilotData is Dictionary<string, object> pilotDict)
            {
                AssignedPilot = Pilot.Deserialize(pilotDict);
            }
        }
    }
}

/// <summary>
/// System for managing pilot hiring and crew management
/// </summary>
public class CrewManagementSystem
{
    private readonly EntityManager _entityManager;
    
    // Available pilots for hire at stations
    private readonly Dictionary<Guid, List<Pilot>> _stationPilots = new();
    
    // Player's unemployed pilots
    private readonly List<Pilot> _availablePilots = new();
    
    public IReadOnlyList<Pilot> AvailablePilots => _availablePilots.AsReadOnly();
    
    public CrewManagementSystem(EntityManager entityManager)
    {
        _entityManager = entityManager;
    }
    
    /// <summary>
    /// Generate pilots available at a station
    /// </summary>
    public List<Pilot> GenerateStationPilots(Guid stationId, int count)
    {
        var pilots = new List<Pilot>();
        
        for (int i = 0; i < count; i++)
        {
            int level = Random.Shared.Next(1, 11);
            var pilot = new Pilot(Pilot.GenerateRandomName(), level);
            pilots.Add(pilot);
        }
        
        _stationPilots[stationId] = pilots;
        return pilots;
    }
    
    /// <summary>
    /// Get pilots available at a station
    /// </summary>
    public List<Pilot> GetStationPilots(Guid stationId)
    {
        if (_stationPilots.TryGetValue(stationId, out var pilots))
        {
            return pilots;
        }
        
        // Generate default pilots if none exist
        return GenerateStationPilots(stationId, Random.Shared.Next(3, 8));
    }
    
    /// <summary>
    /// Hire a pilot from a station
    /// </summary>
    public bool HirePilot(Guid stationId, Pilot pilot, Resources.Inventory playerInventory)
    {
        if (!_stationPilots.TryGetValue(stationId, out var pilots))
            return false;
        
        if (!pilots.Contains(pilot))
            return false;
        
        // Check if player has enough credits
        if (!playerInventory.HasResource(Resources.ResourceType.Credits, pilot.HiringCost))
            return false;
        
        // Deduct cost
        if (!playerInventory.RemoveResource(Resources.ResourceType.Credits, pilot.HiringCost))
            return false;
        
        // Remove from station and add to available pilots
        pilots.Remove(pilot);
        _availablePilots.Add(pilot);
        
        return true;
    }
    
    /// <summary>
    /// Assign a pilot to a ship
    /// </summary>
    public bool AssignPilotToShip(Pilot pilot, Guid shipId)
    {
        var crew = _entityManager.GetComponent<CrewComponent>(shipId);
        if (crew == null)
            return false;
        
        if (!crew.AssignPilot(pilot))
            return false;
        
        _availablePilots.Remove(pilot);
        return true;
    }
    
    /// <summary>
    /// Remove pilot from ship and return to available pool
    /// </summary>
    public bool RemovePilotFromShip(Guid shipId)
    {
        var crew = _entityManager.GetComponent<CrewComponent>(shipId);
        if (crew == null)
            return false;
        
        var pilot = crew.RemovePilot();
        if (pilot == null)
            return false;
        
        _availablePilots.Add(pilot);
        return true;
    }
    
    /// <summary>
    /// Hire crew for a ship
    /// </summary>
    public bool HireCrew(Guid shipId, int count, Resources.Inventory playerInventory)
    {
        var crew = _entityManager.GetComponent<CrewComponent>(shipId);
        if (crew == null)
            return false;
        
        // Cost per crew member
        int costPerCrew = 500;
        int totalCost = costPerCrew * count;
        
        if (!playerInventory.HasResource(Resources.ResourceType.Credits, totalCost))
            return false;
        
        if (!playerInventory.RemoveResource(Resources.ResourceType.Credits, totalCost))
            return false;
        
        return crew.AddCrew(count);
    }
    
    /// <summary>
    /// Pay daily salaries for all pilots
    /// </summary>
    public int PayDailySalaries(Resources.Inventory playerInventory)
    {
        int totalSalaries = 0;
        
        // Pay assigned pilots
        var entities = _entityManager.GetAllEntities();
        foreach (var entity in entities)
        {
            var crew = _entityManager.GetComponent<CrewComponent>(entity.Id);
            if (crew?.AssignedPilot != null)
            {
                totalSalaries += crew.AssignedPilot.DailySalary;
            }
        }
        
        // Pay available pilots (reduced salary)
        foreach (var pilot in _availablePilots)
        {
            totalSalaries += pilot.DailySalary / 2; // Half salary when not assigned
        }
        
        if (playerInventory.HasResource(Resources.ResourceType.Credits, totalSalaries))
        {
            playerInventory.RemoveResource(Resources.ResourceType.Credits, totalSalaries);
        }
        
        return totalSalaries;
    }
    
    /// <summary>
    /// Check if a ship can operate (has crew, pilot, or is controlled by player pod)
    /// </summary>
    public bool CanShipOperate(Guid shipId, Guid? playerPodId = null)
    {
        var crew = _entityManager.GetComponent<CrewComponent>(shipId);
        if (crew == null)
            return false;
        
        // Player pod overrides pilot requirement
        if (playerPodId.HasValue && crew.CanBeControlledByPod(playerPodId.Value, _entityManager))
        {
            return crew.HasSufficientCrew();
        }
        
        // Otherwise needs pilot and crew
        return crew.IsOperational();
    }
}
