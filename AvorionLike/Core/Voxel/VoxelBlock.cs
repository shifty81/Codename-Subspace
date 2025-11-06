using System.Numerics;
using System.Text.Json;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Represents a voxel block with position, size, and material properties
/// </summary>
public class VoxelBlock
{
    public Vector3 Position { get; set; }
    public Vector3 Size { get; set; }
    public string MaterialType { get; set; }
    public BlockType BlockType { get; set; } = BlockType.Hull;
    public float Durability { get; set; }
    public float MaxDurability { get; set; }
    public float Mass { get; set; }
    public uint ColorRGB { get; set; } // RGB color as uint (e.g., 0xRRGGBB)
    public bool IsDestroyed { get; set; } = false;
    
    // Functional properties for engines, thrusters, etc.
    public float ThrustPower { get; set; } = 0f; // For engines/thrusters
    public float PowerGeneration { get; set; } = 0f; // For generators
    public float ShieldCapacity { get; set; } = 0f; // For shield generators

    public VoxelBlock(Vector3 position, Vector3 size, string materialType = "Iron", BlockType blockType = BlockType.Hull)
    {
        Position = position;
        Size = size;
        MaterialType = materialType;
        BlockType = blockType;
        
        var material = MaterialProperties.GetMaterial(materialType);
        float volume = size.X * size.Y * size.Z;
        
        // Calculate properties based on material and block type
        Mass = volume * material.MassMultiplier;
        MaxDurability = 100f * material.DurabilityMultiplier * volume;
        Durability = MaxDurability;
        ColorRGB = material.Color;
        
        // Apply block-specific modifiers
        switch (blockType)
        {
            case BlockType.Armor:
                // Armor is 5x more durable but 1.5x heavier than hull
                MaxDurability *= 5.0f;
                Durability = MaxDurability;
                Mass *= 1.5f;
                break;
            case BlockType.Engine:
                ThrustPower = 50f * volume * material.EnergyEfficiency;
                break;
            case BlockType.Thruster:
                ThrustPower = 30f * volume * material.EnergyEfficiency;
                break;
            case BlockType.GyroArray:
                ThrustPower = 20f * volume * material.EnergyEfficiency; // Torque
                break;
            case BlockType.Generator:
                PowerGeneration = 100f * volume * material.EnergyEfficiency;
                break;
            case BlockType.ShieldGenerator:
                ShieldCapacity = 200f * volume * material.ShieldMultiplier;
                break;
        }
    }

    /// <summary>
    /// Take damage to this block
    /// </summary>
    public void TakeDamage(float damage)
    {
        Durability -= damage;
        if (Durability <= 0)
        {
            Durability = 0;
            IsDestroyed = true;
        }
    }

    /// <summary>
    /// Check if this voxel intersects with another
    /// </summary>
    public bool Intersects(VoxelBlock other)
    {
        return Position.X < other.Position.X + other.Size.X &&
               Position.X + Size.X > other.Position.X &&
               Position.Y < other.Position.Y + other.Size.Y &&
               Position.Y + Size.Y > other.Position.Y &&
               Position.Z < other.Position.Z + other.Size.Z &&
               Position.Z + Size.Z > other.Position.Z;
    }

    /// <summary>
    /// Serialize the voxel block to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        return new Dictionary<string, object>
        {
            ["Position"] = new Dictionary<string, object>
            {
                ["X"] = Position.X,
                ["Y"] = Position.Y,
                ["Z"] = Position.Z
            },
            ["Size"] = new Dictionary<string, object>
            {
                ["X"] = Size.X,
                ["Y"] = Size.Y,
                ["Z"] = Size.Z
            },
            ["MaterialType"] = MaterialType,
            ["BlockType"] = BlockType.ToString(),
            ["Durability"] = Durability,
            ["MaxDurability"] = MaxDurability,
            ["ColorRGB"] = ColorRGB,
            ["ThrustPower"] = ThrustPower,
            ["PowerGeneration"] = PowerGeneration,
            ["ShieldCapacity"] = ShieldCapacity
        };
    }

    /// <summary>
    /// Deserialize the voxel block from a dictionary
    /// </summary>
    public static VoxelBlock Deserialize(Dictionary<string, object> data)
    {
        // Handle Position
        Vector3 position;
        if (data["Position"] is JsonElement posJsonElement)
        {
            position = new Vector3(
                posJsonElement.GetProperty("X").GetSingle(),
                posJsonElement.GetProperty("Y").GetSingle(),
                posJsonElement.GetProperty("Z").GetSingle()
            );
        }
        else
        {
            var posData = (Dictionary<string, object>)data["Position"];
            position = new Vector3(
                Convert.ToSingle(posData["X"]),
                Convert.ToSingle(posData["Y"]),
                Convert.ToSingle(posData["Z"])
            );
        }
        
        // Handle Size
        Vector3 size;
        if (data["Size"] is JsonElement sizeJsonElement)
        {
            size = new Vector3(
                sizeJsonElement.GetProperty("X").GetSingle(),
                sizeJsonElement.GetProperty("Y").GetSingle(),
                sizeJsonElement.GetProperty("Z").GetSingle()
            );
        }
        else
        {
            var sizeData = (Dictionary<string, object>)data["Size"];
            size = new Vector3(
                Convert.ToSingle(sizeData["X"]),
                Convert.ToSingle(sizeData["Y"]),
                Convert.ToSingle(sizeData["Z"])
            );
        }
        
        var materialType = data["MaterialType"].ToString() ?? "Iron";
        var blockTypeStr = data["BlockType"].ToString() ?? "Hull";
        var blockType = Enum.Parse<BlockType>(blockTypeStr);
        
        // Create block with basic properties
        var block = new VoxelBlock(position, size, materialType, blockType);
        
        // Restore damage state
        if (data.ContainsKey("Durability"))
        {
            var durability = data["Durability"];
            block.Durability = durability is JsonElement durJsonElement 
                ? durJsonElement.GetSingle() 
                : Convert.ToSingle(durability);
        }
        
        // Override functional properties if they were saved
        if (data.ContainsKey("ThrustPower"))
        {
            var thrust = data["ThrustPower"];
            block.ThrustPower = thrust is JsonElement thrustJsonElement 
                ? thrustJsonElement.GetSingle() 
                : Convert.ToSingle(thrust);
        }
        if (data.ContainsKey("PowerGeneration"))
        {
            var power = data["PowerGeneration"];
            block.PowerGeneration = power is JsonElement powerJsonElement 
                ? powerJsonElement.GetSingle() 
                : Convert.ToSingle(power);
        }
        if (data.ContainsKey("ShieldCapacity"))
        {
            var shield = data["ShieldCapacity"];
            block.ShieldCapacity = shield is JsonElement shieldJsonElement 
                ? shieldJsonElement.GetSingle() 
                : Convert.ToSingle(shield);
        }
        
        return block;
    }
}
