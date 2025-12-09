using System.Text.Json;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Quest;

/// <summary>
/// Loads quest definitions from JSON files
/// </summary>
public class QuestLoader
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNameCaseInsensitive = true
    };
    
    /// <summary>
    /// Load a quest from a JSON file
    /// </summary>
    /// <param name="filePath">Path to the JSON file</param>
    /// <returns>Loaded quest, or null if failed</returns>
    public static Quest? LoadQuestFromFile(string filePath)
    {
        try
        {
            if (!File.Exists(filePath))
            {
                Logger.Instance.Warning("QuestLoader", $"Quest file not found: {filePath}");
                return null;
            }
            
            string json = File.ReadAllText(filePath);
            var quest = JsonSerializer.Deserialize<Quest>(json, JsonOptions);
            
            if (quest != null)
            {
                Logger.Instance.Info("QuestLoader", $"Loaded quest '{quest.Title}' from {filePath}");
            }
            
            return quest;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("QuestLoader", $"Failed to load quest from {filePath}: {ex.Message}", ex);
            return null;
        }
    }
    
    /// <summary>
    /// Load all quests from a directory
    /// </summary>
    /// <param name="directoryPath">Path to directory containing quest JSON files</param>
    /// <returns>List of loaded quests</returns>
    public static List<Quest> LoadQuestsFromDirectory(string directoryPath)
    {
        var quests = new List<Quest>();
        
        try
        {
            if (!Directory.Exists(directoryPath))
            {
                Logger.Instance.Warning("QuestLoader", $"Quest directory not found: {directoryPath}");
                return quests;
            }
            
            var jsonFiles = Directory.GetFiles(directoryPath, "*.json", SearchOption.AllDirectories);
            Logger.Instance.Info("QuestLoader", $"Found {jsonFiles.Length} quest files in {directoryPath}");
            
            foreach (var file in jsonFiles)
            {
                var quest = LoadQuestFromFile(file);
                if (quest != null)
                {
                    quests.Add(quest);
                }
            }
            
            Logger.Instance.Info("QuestLoader", $"Successfully loaded {quests.Count} quests");
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("QuestLoader", $"Failed to load quests from directory {directoryPath}: {ex.Message}", ex);
        }
        
        return quests;
    }
    
    /// <summary>
    /// Save a quest to a JSON file
    /// </summary>
    /// <param name="quest">Quest to save</param>
    /// <param name="filePath">Path to save the JSON file</param>
    /// <returns>True if saved successfully</returns>
    public static bool SaveQuestToFile(Quest quest, string filePath)
    {
        try
        {
            // Ensure directory exists
            var directory = Path.GetDirectoryName(filePath);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }
            
            string json = JsonSerializer.Serialize(quest, JsonOptions);
            File.WriteAllText(filePath, json);
            
            Logger.Instance.Info("QuestLoader", $"Saved quest '{quest.Title}' to {filePath}");
            return true;
        }
        catch (Exception ex)
        {
            Logger.Instance.Error("QuestLoader", $"Failed to save quest to {filePath}: {ex.Message}", ex);
            return false;
        }
    }
    
    /// <summary>
    /// Create a sample quest and save it to a file (for testing/reference)
    /// </summary>
    /// <param name="filePath">Path to save the sample quest</param>
    public static void CreateSampleQuest(string filePath)
    {
        var quest = new Quest
        {
            Id = "quest_tutorial_mining",
            Title = "First Steps: Mining",
            Description = "Welcome to the galaxy! Learn the basics of mining by collecting Iron ore from nearby asteroids.",
            Difficulty = QuestDifficulty.Easy,
            CanAbandon = false,
            IsRepeatable = false,
            Tags = new List<string> { "tutorial", "mining" }
        };
        
        quest.Objectives.Add(new QuestObjective
        {
            Id = "obj_mine_iron",
            Type = ObjectiveType.Mine,
            Description = "Mine 100 Iron ore",
            Target = "Iron",
            RequiredQuantity = 100
        });
        
        quest.Rewards.Add(new QuestReward
        {
            Type = RewardType.Credits,
            Amount = 1000,
            Description = "1,000 Credits"
        });
        
        quest.Rewards.Add(new QuestReward
        {
            Type = RewardType.Experience,
            Amount = 50,
            Description = "50 Experience"
        });
        
        SaveQuestToFile(quest, filePath);
    }
}
