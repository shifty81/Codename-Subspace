using System.Numerics;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Pure-function compiler that produces a CompiledShipStats snapshot from a
/// VoxelStructureComponent's block list. This is the single authoritative
/// source for all ship stats at runtime.
///
/// Usage:
///   var stats = ShipStatsCompiler.Compile(voxelStructure);
///   // Use stats.Thrust, stats.Mass, etc. — never iterate blocks at runtime.
///
/// Call Compile() whenever blocks are added, removed, or damaged.
/// </summary>
public static class ShipStatsCompiler
{
    /// <summary>
    /// Compile ship stats from a voxel structure's blocks.
    /// Two-pass algorithm: first pass computes mass &amp; center-of-mass,
    /// second pass computes functional stats relative to center-of-mass.
    /// </summary>
    public static CompiledShipStats Compile(VoxelStructureComponent structure)
    {
        return Compile(structure.Blocks);
    }

    /// <summary>
    /// Compile ship stats from a list of blocks.
    /// This overload is useful for testing without a full component.
    /// </summary>
    public static CompiledShipStats Compile(IReadOnlyList<VoxelBlock> blocks)
    {
        if (blocks.Count == 0)
        {
            return default;
        }

        // ── Pass 1: mass, center-of-mass, hit points ──
        float mass = 0f;
        float totalHP = 0f;
        float currentHP = 0f;
        Vector3 weightedPos = Vector3.Zero;

        foreach (var block in blocks)
        {
            var def = BlockDefinitionDatabase.GetDefinition(block.BlockType);
            float volume = block.Size.X * block.Size.Y * block.Size.Z;
            float blockMass = volume * def.MassPerUnitVolume;

            mass += blockMass;
            weightedPos += block.Position * blockMass;
            totalHP += block.MaxDurability;
            currentHP += block.Durability;
        }

        Vector3 centerOfMass = mass > 0 ? weightedPos / mass : Vector3.Zero;

        // ── Pass 2: functional stats ──
        float momentOfInertia = 0f;
        float powerGen = 0f;
        float powerCon = 0f;
        float thrust = 0f;
        float torque = 0f;
        float shieldCap = 0f;
        float armorPts = 0f;
        float cargoCap = 0f;
        int crewCap = 0;
        int weaponMounts = 0;
        bool hasHyperdrive = false;
        bool hasPodDocking = false;

        foreach (var block in blocks)
        {
            var def = BlockDefinitionDatabase.GetDefinition(block.BlockType);
            float volume = block.Size.X * block.Size.Y * block.Size.Z;
            float blockMass = volume * def.MassPerUnitVolume;

            // Moment of inertia relative to center of mass
            Vector3 r = block.Position - centerOfMass;
            momentOfInertia += blockMass * r.LengthSquared();

            // Power
            powerGen += block.PowerGeneration;
            powerCon += def.PowerConsumptionPerVolume * volume;

            // Propulsion
            if (block.BlockType == BlockType.Engine || block.BlockType == BlockType.Thruster)
            {
                thrust += block.ThrustPower;

                if (block.BlockType == BlockType.Thruster)
                {
                    float dist = r.Length();
                    float leverage = 1.0f + dist * 0.1f;
                    torque += block.ThrustPower * leverage * 0.5f;
                }
            }
            else if (block.BlockType == BlockType.GyroArray)
            {
                float dist = r.Length();
                float leverage = 1.0f + dist * 0.05f;
                torque += block.ThrustPower * leverage;
            }

            // Defense
            shieldCap += block.ShieldCapacity;
            if (block.BlockType == BlockType.Armor)
            {
                armorPts += block.Durability;
            }

            // Utility
            if (block.BlockType == BlockType.Cargo)
            {
                cargoCap += def.CargoCapacityPerVolume * volume;
            }
            if (block.BlockType == BlockType.CrewQuarters)
            {
                crewCap += (int)(def.CrewCapacityPerVolume * volume);
            }

            // Weapons
            if (block.BlockType == BlockType.TurretMount)
            {
                weaponMounts++;
            }

            // Special systems
            if (block.BlockType == BlockType.HyperdriveCore)
            {
                hasHyperdrive = true;
            }
            if (block.BlockType == BlockType.PodDocking)
            {
                hasPodDocking = true;
            }
        }

        float integrity = totalHP > 0 ? (currentHP / totalHP) * 100f : 0f;

        return new CompiledShipStats
        {
            Mass = mass,
            TotalHitPoints = totalHP,
            CurrentHitPoints = currentHP,
            CenterOfMass = centerOfMass,
            MomentOfInertia = momentOfInertia,
            StructuralIntegrity = integrity,
            PowerGeneration = powerGen,
            PowerConsumption = powerCon,
            Thrust = thrust,
            Torque = torque,
            ShieldCapacity = shieldCap,
            ArmorPoints = armorPts,
            CargoCapacity = cargoCap,
            CrewCapacity = crewCap,
            WeaponMounts = weaponMounts,
            HasHyperdrive = hasHyperdrive,
            HasPodDocking = hasPodDocking,
            TotalBlocks = blocks.Count
        };
    }
}
