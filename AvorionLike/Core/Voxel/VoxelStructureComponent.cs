using System.Numerics;
using System.Text.Json;
using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Component that contains voxel-based structure data for ships and stations
/// </summary>
public class VoxelStructureComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    public List<VoxelBlock> Blocks { get; set; } = new();
    public Vector3 CenterOfMass { get; private set; }
    public float TotalMass { get; private set; }
    public float MomentOfInertia { get; private set; }
    
    // Thrust capabilities
    public float TotalThrust { get; private set; }
    public float TotalTorque { get; private set; }
    
    // Power and shields
    public float PowerGeneration { get; private set; }
    public float ShieldCapacity { get; private set; }
    
    // Structure integrity
    public float StructuralIntegrity { get; private set; } = 100f;

    /// <summary>
    /// Add a voxel block to the structure
    /// </summary>
    public void AddBlock(VoxelBlock block)
    {
        Blocks.Add(block);
        RecalculateProperties();
    }

    /// <summary>
    /// Remove a voxel block from the structure
    /// </summary>
    public bool RemoveBlock(VoxelBlock block)
    {
        bool removed = Blocks.Remove(block);
        if (removed)
        {
            RecalculateProperties();
        }
        return removed;
    }

    /// <summary>
    /// Damage a specific block at a position
    /// </summary>
    public List<VoxelBlock> DamageAtPosition(Vector3 position, float radius, float damage)
    {
        var destroyedBlocks = new List<VoxelBlock>();
        
        foreach (var block in Blocks)
        {
            float distance = Vector3.Distance(block.Position, position);
            if (distance <= radius)
            {
                // Apply damage with falloff
                float actualDamage = damage * (1f - distance / radius);
                block.TakeDamage(actualDamage);
                
                if (block.IsDestroyed)
                {
                    destroyedBlocks.Add(block);
                }
            }
        }
        
        // Remove destroyed blocks
        foreach (var block in destroyedBlocks)
        {
            Blocks.Remove(block);
        }
        
        if (destroyedBlocks.Count > 0)
        {
            RecalculateProperties();
        }
        
        return destroyedBlocks;
    }

    /// <summary>
    /// Recalculate center of mass and all ship properties
    /// </summary>
    private void RecalculateProperties()
    {
        if (Blocks.Count == 0)
        {
            CenterOfMass = Vector3.Zero;
            TotalMass = 0f;
            MomentOfInertia = 0f;
            TotalThrust = 0f;
            TotalTorque = 0f;
            PowerGeneration = 0f;
            ShieldCapacity = 0f;
            StructuralIntegrity = 0f;
            return;
        }

        float totalMass = 0f;
        Vector3 weightedPosition = Vector3.Zero;
        float totalThrust = 0f;
        float totalTorque = 0f;
        float powerGen = 0f;
        float shieldCap = 0f;
        float totalDurability = 0f;
        float maxDurability = 0f;

        // First pass: calculate center of mass
        foreach (var block in Blocks)
        {
            totalMass += block.Mass;
            weightedPosition += block.Position * block.Mass;
        }

        TotalMass = totalMass;
        CenterOfMass = weightedPosition / totalMass;

        // Second pass: calculate moment of inertia and other properties
        float momentOfInertia = 0f;
        foreach (var block in Blocks)
        {
            // Moment of inertia relative to center of mass
            Vector3 r = block.Position - CenterOfMass;
            momentOfInertia += block.Mass * r.LengthSquared();
            
            // Accumulate functional properties
            if (block.BlockType == BlockType.Engine || block.BlockType == BlockType.Thruster)
            {
                totalThrust += block.ThrustPower;
            }
            else if (block.BlockType == BlockType.GyroArray)
            {
                totalTorque += block.ThrustPower; // Torque for gyros
            }
            else if (block.BlockType == BlockType.Generator)
            {
                powerGen += block.PowerGeneration;
            }
            else if (block.BlockType == BlockType.ShieldGenerator)
            {
                shieldCap += block.ShieldCapacity;
            }
            
            totalDurability += block.Durability;
            maxDurability += block.MaxDurability;
        }

        MomentOfInertia = momentOfInertia;
        TotalThrust = totalThrust;
        TotalTorque = totalTorque;
        PowerGeneration = powerGen;
        ShieldCapacity = shieldCap;
        StructuralIntegrity = maxDurability > 0 ? (totalDurability / maxDurability) * 100f : 0f;
    }

    /// <summary>
    /// Get blocks at a specific position
    /// </summary>
    public IEnumerable<VoxelBlock> GetBlocksAt(Vector3 position, float tolerance = 0.1f)
    {
        return Blocks.Where(b => Vector3.Distance(b.Position, position) < tolerance);
    }
    
    /// <summary>
    /// Get blocks of a specific type
    /// </summary>
    public IEnumerable<VoxelBlock> GetBlocksByType(BlockType type)
    {
        return Blocks.Where(b => b.BlockType == type);
    }

    /// <summary>
    /// Serialize the component to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        var blocksData = new List<Dictionary<string, object>>();
        foreach (var block in Blocks)
        {
            blocksData.Add(block.Serialize());
        }

        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["Blocks"] = blocksData,
            ["StructuralIntegrity"] = StructuralIntegrity
        };
    }

    /// <summary>
    /// Deserialize the component from a dictionary
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));
        StructuralIntegrity = SerializationHelper.GetValue(data, "StructuralIntegrity", 100f);
        
        Blocks.Clear();
        
        if (data.ContainsKey("Blocks"))
        {
            var blocksData = data["Blocks"];
            
            if (blocksData is JsonElement jsonElement && jsonElement.ValueKind == JsonValueKind.Array)
            {
                foreach (var blockElement in jsonElement.EnumerateArray())
                {
                    var blockDict = JsonSerializer.Deserialize<Dictionary<string, object>>(blockElement.GetRawText());
                    if (blockDict != null)
                    {
                        var block = VoxelBlock.Deserialize(blockDict);
                        Blocks.Add(block);
                    }
                }
            }
            else if (blocksData is List<object> blocksList)
            {
                foreach (var blockObj in blocksList)
                {
                    if (blockObj is Dictionary<string, object> blockDict)
                    {
                        var block = VoxelBlock.Deserialize(blockDict);
                        Blocks.Add(block);
                    }
                }
            }
        }
        
        // Recalculate all derived properties
        RecalculateProperties();
    }
}
