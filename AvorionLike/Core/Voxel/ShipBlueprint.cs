using System.Numerics;
using System.Text.Json;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Voxel;

/// <summary>
/// Data structure for a single block in a blueprint
/// </summary>
public class VoxelBlockData
{
    public Vector3 Position { get; set; }
    public Vector3 Size { get; set; }
    public string MaterialType { get; set; } = "Iron";
    public BlockType BlockType { get; set; } = BlockType.Hull;
}

/// <summary>
/// Represents a saved ship design (blueprint)
/// </summary>
public class ShipBlueprint
{
    public string Name { get; set; } = "Unnamed Ship";
    public string Description { get; set; } = "";
    public DateTime CreatedDate { get; set; } = DateTime.UtcNow;
    public List<VoxelBlockData> Blocks { get; set; } = new List<VoxelBlockData>();
    
    /// <summary>
    /// Create a blueprint from a voxel structure component
    /// </summary>
    public static ShipBlueprint FromVoxelStructure(string name, VoxelStructureComponent structure)
    {
        var blueprint = new ShipBlueprint
        {
            Name = name,
            CreatedDate = DateTime.UtcNow
        };
        
        foreach (var block in structure.Blocks)
        {
            blueprint.Blocks.Add(new VoxelBlockData
            {
                Position = block.Position,
                Size = block.Size,
                MaterialType = block.MaterialType,
                BlockType = block.BlockType
            });
        }
        
        return blueprint;
    }
    
    /// <summary>
    /// Apply this blueprint to a voxel structure component
    /// </summary>
    public void ApplyToVoxelStructure(VoxelStructureComponent structure)
    {
        structure.Blocks.Clear();
        
        foreach (var blockData in Blocks)
        {
            var block = new VoxelBlock(blockData.Position, blockData.Size, blockData.MaterialType, blockData.BlockType);
            structure.AddBlock(block);
        }
    }
    
    /// <summary>
    /// Save blueprint to file
    /// </summary>
    public bool SaveToFile(string filePath)
    {
        try
        {
            var json = JsonSerializer.Serialize(this, new JsonSerializerOptions
            {
                WriteIndented = true
            });
            
            File.WriteAllText(filePath, json);
            Logger.Instance.Info("ShipBlueprint", $"Blueprint saved: {filePath}");
            return true;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("ShipBlueprint", $"Failed to save blueprint: {ex.Message}");
            return false;
        }
    }
    
    /// <summary>
    /// Load blueprint from file
    /// </summary>
    public static ShipBlueprint? LoadFromFile(string filePath)
    {
        try
        {
            var json = File.ReadAllText(filePath);
            var blueprint = JsonSerializer.Deserialize<ShipBlueprint>(json);
            
            if (blueprint != null)
            {
                Logger.Instance.Info("ShipBlueprint", $"Blueprint loaded: {filePath}");
            }
            
            return blueprint;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("ShipBlueprint", $"Failed to load blueprint: {ex.Message}");
            return null;
        }
    }
    
    /// <summary>
    /// Get the default blueprints directory
    /// </summary>
    public static string GetBlueprintsDirectory()
    {
        var appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        var blueprintsPath = Path.Combine(appDataPath, "Codename-Subspace", "Blueprints");
        
        if (!Directory.Exists(blueprintsPath))
        {
            Directory.CreateDirectory(blueprintsPath);
        }
        
        return blueprintsPath;
    }
    
    /// <summary>
    /// List all available blueprints
    /// </summary>
    public static List<string> ListBlueprints()
    {
        try
        {
            var blueprintsDir = GetBlueprintsDirectory();
            var files = Directory.GetFiles(blueprintsDir, "*.blueprint");
            return files.Select(f => Path.GetFileNameWithoutExtension(f) ?? "Unknown").ToList();
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("ShipBlueprint", $"Failed to list blueprints: {ex.Message}");
            return new List<string>();
        }
    }
}
