using System.Numerics;
using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Aggregates and calculates all ship/station properties from voxel blocks
/// This is the main object that the AI uses to understand ship performance
/// </summary>
public class ShipAggregate
{
    private readonly VoxelStructureComponent _structure;
    
    // Structural properties
    public float TotalMass { get; private set; }
    public float TotalHitPoints { get; private set; }
    public float CurrentHitPoints { get; private set; }
    public Vector3 CenterOfMass { get; private set; }
    public float MomentOfInertia { get; private set; }
    public float StructuralIntegrity { get; private set; }
    
    // Power system
    public float TotalPowerGeneration { get; private set; }
    public float TotalPowerConsumption { get; private set; }
    public float AvailablePower => TotalPowerGeneration - TotalPowerConsumption;
    public float PowerEfficiency => TotalPowerGeneration > 0 ? AvailablePower / TotalPowerGeneration : 0f;
    
    // Propulsion
    public float TotalThrust { get; private set; }
    public float TotalTorque { get; private set; }
    public float MaxSpeed { get; private set; } // Based on thrust-to-mass ratio
    public float MaxRotationSpeed { get; private set; } // Based on torque-to-inertia ratio
    public float Acceleration { get; private set; }
    
    // Defense
    public float TotalShieldCapacity { get; private set; }
    public float TotalArmorPoints { get; private set; }
    
    // Utility
    public float TotalCargoCapacity { get; private set; }
    public float AvailableCargoSpace { get; private set; }
    public int TotalCrewCapacity { get; private set; }
    public int CurrentCrew { get; private set; }
    
    // Weapon systems
    public int WeaponMountCount { get; private set; }
    
    // Special systems
    public bool HasHyperdrive { get; private set; }
    public bool HasPodDocking { get; private set; }
    
    // Block counts by type
    public Dictionary<BlockType, int> BlockCounts { get; private set; } = new();
    
    // Maneuverability rating (0-100, higher is better)
    public float ManeuverabilityRating { get; private set; }
    
    // Combat effectiveness rating (0-100, higher is better)
    public float CombatEffectivenessRating { get; private set; }
    
    // Cargo efficiency rating (0-100, higher is better)
    public float CargoEfficiencyRating { get; private set; }
    
    public ShipAggregate(VoxelStructureComponent structure)
    {
        _structure = structure;
        Recalculate();
    }
    
    /// <summary>
    /// Recalculate all aggregate properties from blocks
    /// Call this whenever blocks are added/removed or damaged
    /// </summary>
    public void Recalculate()
    {
        ResetProperties();
        
        if (_structure.Blocks.Count == 0)
        {
            return;
        }
        
        // First pass: accumulate basic properties and calculate center of mass
        Vector3 weightedPosition = Vector3.Zero;
        foreach (var block in _structure.Blocks)
        {
            var definition = BlockDefinitionDatabase.GetDefinition(block.BlockType);
            float volume = block.Size.X * block.Size.Y * block.Size.Z;
            
            // Mass
            float blockMass = volume * definition.MassPerUnitVolume;
            TotalMass += blockMass;
            weightedPosition += block.Position * blockMass;
            
            // Hit points
            TotalHitPoints += block.MaxDurability;
            CurrentHitPoints += block.Durability;
            
            // Count blocks by type
            if (!BlockCounts.ContainsKey(block.BlockType))
            {
                BlockCounts[block.BlockType] = 0;
            }
            BlockCounts[block.BlockType]++;
        }
        
        CenterOfMass = TotalMass > 0 ? weightedPosition / TotalMass : Vector3.Zero;
        StructuralIntegrity = TotalHitPoints > 0 ? (CurrentHitPoints / TotalHitPoints) * 100f : 0f;
        
        // Second pass: calculate functional properties and moment of inertia
        foreach (var block in _structure.Blocks)
        {
            var definition = BlockDefinitionDatabase.GetDefinition(block.BlockType);
            float volume = block.Size.X * block.Size.Y * block.Size.Z;
            float blockMass = volume * definition.MassPerUnitVolume;
            
            // Moment of inertia relative to center of mass
            Vector3 r = block.Position - CenterOfMass;
            MomentOfInertia += blockMass * r.LengthSquared();
            
            // Power generation and consumption
            TotalPowerGeneration += block.PowerGeneration;
            TotalPowerConsumption += definition.PowerConsumptionPerVolume * volume;
            
            // Propulsion
            if (block.BlockType == BlockType.Engine || block.BlockType == BlockType.Thruster)
            {
                TotalThrust += block.ThrustPower;
            }
            else if (block.BlockType == BlockType.GyroArray)
            {
                TotalTorque += block.ThrustPower;
            }
            
            // Defense
            TotalShieldCapacity += block.ShieldCapacity;
            if (block.BlockType == BlockType.Armor)
            {
                TotalArmorPoints += block.Durability;
            }
            
            // Utility
            if (block.BlockType == BlockType.Cargo)
            {
                TotalCargoCapacity += definition.CargoCapacityPerVolume * volume;
            }
            if (block.BlockType == BlockType.CrewQuarters)
            {
                TotalCrewCapacity += (int)(definition.CrewCapacityPerVolume * volume);
            }
            
            // Weapons
            if (block.BlockType == BlockType.TurretMount)
            {
                WeaponMountCount++;
            }
            
            // Special systems
            if (block.BlockType == BlockType.HyperdriveCore)
            {
                HasHyperdrive = true;
            }
            if (block.BlockType == BlockType.PodDocking)
            {
                HasPodDocking = true;
            }
        }
        
        // Calculate derived properties
        CalculatePerformanceMetrics();
        CalculateRatings();
    }
    
    /// <summary>
    /// Calculate performance metrics like max speed and acceleration
    /// </summary>
    private void CalculatePerformanceMetrics()
    {
        // Max speed calculation (simplified physics)
        // Higher thrust-to-mass ratio = higher max speed
        if (TotalMass > 0)
        {
            Acceleration = TotalThrust / TotalMass;
            MaxSpeed = Acceleration * 10f; // Simplified: assuming 10 second acceleration
        }
        
        // Max rotation speed (simplified)
        if (MomentOfInertia > 0)
        {
            MaxRotationSpeed = TotalTorque / MomentOfInertia;
        }
        
        // Available cargo space (assume some is used)
        AvailableCargoSpace = TotalCargoCapacity * 0.9f; // 90% available
    }
    
    /// <summary>
    /// Calculate various effectiveness ratings
    /// </summary>
    private void CalculateRatings()
    {
        // Maneuverability: based on thrust-to-mass and torque-to-inertia
        float thrustToMass = TotalMass > 0 ? TotalThrust / TotalMass : 0f;
        float torqueToInertia = MomentOfInertia > 0 ? TotalTorque / MomentOfInertia : 0f;
        ManeuverabilityRating = Math.Min(100f, (thrustToMass * 2f + torqueToInertia * 50f));
        
        // Combat effectiveness: weapons, shields, armor, and power
        float weaponScore = WeaponMountCount * 10f;
        float defenseScore = (TotalShieldCapacity / 100f) + (TotalArmorPoints / 100f);
        float powerScore = PowerEfficiency * 20f;
        CombatEffectivenessRating = Math.Min(100f, weaponScore + defenseScore + powerScore);
        
        // Cargo efficiency: cargo capacity relative to mass
        float cargoToMass = TotalMass > 0 ? TotalCargoCapacity / TotalMass : 0f;
        CargoEfficiencyRating = Math.Min(100f, cargoToMass * 10f);
    }
    
    /// <summary>
    /// Reset all properties to zero
    /// </summary>
    private void ResetProperties()
    {
        TotalMass = 0f;
        TotalHitPoints = 0f;
        CurrentHitPoints = 0f;
        CenterOfMass = Vector3.Zero;
        MomentOfInertia = 0f;
        StructuralIntegrity = 0f;
        TotalPowerGeneration = 0f;
        TotalPowerConsumption = 0f;
        TotalThrust = 0f;
        TotalTorque = 0f;
        MaxSpeed = 0f;
        MaxRotationSpeed = 0f;
        Acceleration = 0f;
        TotalShieldCapacity = 0f;
        TotalArmorPoints = 0f;
        TotalCargoCapacity = 0f;
        AvailableCargoSpace = 0f;
        TotalCrewCapacity = 0;
        CurrentCrew = 0;
        WeaponMountCount = 0;
        HasHyperdrive = false;
        HasPodDocking = false;
        BlockCounts.Clear();
        ManeuverabilityRating = 0f;
        CombatEffectivenessRating = 0f;
        CargoEfficiencyRating = 0f;
    }
    
    /// <summary>
    /// Get a summary of ship statistics
    /// </summary>
    public string GetStatsSummary()
    {
        return $@"=== Ship Statistics ===
Blocks: {_structure.Blocks.Count}
Mass: {TotalMass:F1} tons
Integrity: {StructuralIntegrity:F1}%

=== Power ===
Generation: {TotalPowerGeneration:F0} MW
Consumption: {TotalPowerConsumption:F0} MW
Available: {AvailablePower:F0} MW ({PowerEfficiency * 100:F1}% efficiency)

=== Propulsion ===
Thrust: {TotalThrust:F0} kN
Max Speed: {MaxSpeed:F1} m/s
Acceleration: {Acceleration:F2} m/s²
Torque: {TotalTorque:F0} Nm
Max Rotation: {MaxRotationSpeed:F2} rad/s

=== Defense ===
Shield Capacity: {TotalShieldCapacity:F0}
Armor Points: {TotalArmorPoints:F0}
Hull Points: {CurrentHitPoints:F0} / {TotalHitPoints:F0}

=== Utility ===
Cargo Capacity: {TotalCargoCapacity:F0} m³
Crew Capacity: {TotalCrewCapacity}
Weapon Mounts: {WeaponMountCount}

=== Ratings ===
Maneuverability: {ManeuverabilityRating:F0}/100
Combat Effectiveness: {CombatEffectivenessRating:F0}/100
Cargo Efficiency: {CargoEfficiencyRating:F0}/100

=== Special Systems ===
Hyperdrive: {(HasHyperdrive ? "Yes" : "No")}
Pod Docking: {(HasPodDocking ? "Yes" : "No")}";
    }
    
    /// <summary>
    /// Check if the ship meets minimum functional requirements
    /// </summary>
    public List<string> ValidateRequirements()
    {
        var warnings = new List<string>();
        
        if (TotalPowerGeneration < TotalPowerConsumption)
        {
            warnings.Add($"Insufficient power: {TotalPowerConsumption:F0} MW needed, {TotalPowerGeneration:F0} MW available");
        }
        
        if (TotalThrust == 0)
        {
            warnings.Add("No propulsion: Ship has no engines or thrusters");
        }
        
        if (TotalTorque == 0)
        {
            warnings.Add("No rotation control: Ship has no gyro arrays");
        }
        
        if (WeaponMountCount == 0)
        {
            warnings.Add("No weapons: Ship is defenseless");
        }
        
        if (TotalShieldCapacity == 0 && TotalArmorPoints < TotalMass * 10)
        {
            warnings.Add("Weak defenses: Consider adding shields or armor");
        }
        
        if (!HasHyperdrive)
        {
            warnings.Add("No hyperdrive: Ship cannot travel between sectors");
        }
        
        if (TotalCrewCapacity == 0)
        {
            warnings.Add("No crew quarters: Ship requires crew for operation");
        }
        
        return warnings;
    }
}
