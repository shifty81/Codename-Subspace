using System.Text.Json;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Persistence;

/// <summary>
/// Save game data structure
/// </summary>
public class SaveGameData
{
    public string SaveName { get; set; } = "";
    public DateTime SaveTime { get; set; }
    public string Version { get; set; } = "1.0.0";
    public int GalaxySeed { get; set; }
    public Dictionary<string, object> GameState { get; set; } = new();
    public List<EntityData> Entities { get; set; } = new();
}

/// <summary>
/// Serialized entity data
/// </summary>
public class EntityData
{
    public Guid EntityId { get; set; }
    public string EntityName { get; set; } = "";
    public bool IsActive { get; set; }
    public List<ComponentData> Components { get; set; } = new();
}

/// <summary>
/// Serialized component data
/// </summary>
public class ComponentData
{
    public string ComponentType { get; set; } = "";
    public Dictionary<string, object> Data { get; set; } = new();
}

/// <summary>
/// Manages saving and loading game state
/// </summary>
public class SaveGameManager
{
    private static SaveGameManager? _instance;
    private readonly string _saveDirectory;

    public static SaveGameManager Instance
    {
        get
        {
            _instance ??= new SaveGameManager();
            return _instance;
        }
    }

    private SaveGameManager()
    {
        _saveDirectory = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "Codename-Subspace",
            "Saves"
        );

        // Ensure save directory exists
        if (!Directory.Exists(_saveDirectory))
        {
            Directory.CreateDirectory(_saveDirectory);
            Logger.Instance.Info("SaveGameManager", $"Created save directory: {_saveDirectory}");
        }
    }

    /// <summary>
    /// Get the saves directory path
    /// </summary>
    public string GetSaveDirectory()
    {
        return _saveDirectory;
    }

    /// <summary>
    /// List all available save files
    /// </summary>
    public List<SaveGameInfo> ListSaveGames()
    {
        var saves = new List<SaveGameInfo>();

        try
        {
            var files = Directory.GetFiles(_saveDirectory, "*.save");
            
            foreach (var file in files)
            {
                try
                {
                    var json = File.ReadAllText(file);
                    var saveData = JsonSerializer.Deserialize<SaveGameData>(json);
                    
                    if (saveData != null)
                    {
                        saves.Add(new SaveGameInfo
                        {
                            FileName = Path.GetFileName(file),
                            SaveName = saveData.SaveName,
                            SaveTime = saveData.SaveTime,
                            Version = saveData.Version
                        });
                    }
                }
                catch (Exception ex)
                {
                    Logger.Instance.Warning("SaveGameManager", $"Failed to read save file {file}: {ex.Message}");
                }
            }
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("SaveGameManager", "Failed to list save games", ex);
        }

        return saves.OrderByDescending(s => s.SaveTime).ToList();
    }

    /// <summary>
    /// Save game state to file
    /// </summary>
    public bool SaveGame(SaveGameData saveData, string fileName)
    {
        try
        {
            if (!fileName.EndsWith(".save"))
            {
                fileName += ".save";
            }

            var filePath = Path.Combine(_saveDirectory, fileName);
            
            saveData.SaveTime = DateTime.UtcNow;
            
            var options = new JsonSerializerOptions
            {
                WriteIndented = true
            };
            
            var json = JsonSerializer.Serialize(saveData, options);
            File.WriteAllText(filePath, json);
            
            Logger.Instance.Info("SaveGameManager", $"Game saved to: {filePath}");
            return true;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("SaveGameManager", $"Failed to save game to {fileName}", ex);
            return false;
        }
    }

    /// <summary>
    /// Load game state from file
    /// </summary>
    public SaveGameData? LoadGame(string fileName)
    {
        try
        {
            if (!fileName.EndsWith(".save"))
            {
                fileName += ".save";
            }

            var filePath = Path.Combine(_saveDirectory, fileName);
            
            if (!File.Exists(filePath))
            {
                Logger.Instance.Warning("SaveGameManager", $"Save file not found: {filePath}");
                return null;
            }

            var json = File.ReadAllText(filePath);
            var saveData = JsonSerializer.Deserialize<SaveGameData>(json);
            
            if (saveData == null)
            {
                Logger.Instance.Error("SaveGameManager", $"Failed to deserialize save file: {filePath}");
                return null;
            }

            Logger.Instance.Info("SaveGameManager", $"Game loaded from: {filePath}");
            return saveData;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("SaveGameManager", $"Failed to load game from {fileName}", ex);
            return null;
        }
    }

    /// <summary>
    /// Delete a save file
    /// </summary>
    public bool DeleteSave(string fileName)
    {
        try
        {
            if (!fileName.EndsWith(".save"))
            {
                fileName += ".save";
            }

            var filePath = Path.Combine(_saveDirectory, fileName);
            
            if (File.Exists(filePath))
            {
                File.Delete(filePath);
                Logger.Instance.Info("SaveGameManager", $"Deleted save file: {filePath}");
                return true;
            }
            
            return false;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("SaveGameManager", $"Failed to delete save {fileName}", ex);
            return false;
        }
    }

    /// <summary>
    /// Create a quick save with auto-generated name
    /// </summary>
    public bool QuickSave(SaveGameData saveData)
    {
        var fileName = $"quicksave_{DateTime.Now:yyyyMMdd_HHmmss}.save";
        saveData.SaveName = "Quick Save";
        return SaveGame(saveData, fileName);
    }

    /// <summary>
    /// Get the most recent save file
    /// </summary>
    public SaveGameInfo? GetMostRecentSave()
    {
        var saves = ListSaveGames();
        return saves.FirstOrDefault();
    }
}

/// <summary>
/// Information about a save game file
/// </summary>
public class SaveGameInfo
{
    public string FileName { get; set; } = "";
    public string SaveName { get; set; } = "";
    public DateTime SaveTime { get; set; }
    public string Version { get; set; } = "";
}
