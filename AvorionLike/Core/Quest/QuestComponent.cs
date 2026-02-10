using AvorionLike.Core.ECS;
using AvorionLike.Core.Persistence;
using System.Text.Json;

namespace AvorionLike.Core.Quest;

/// <summary>
/// Component that tracks quests for an entity (typically the player)
/// </summary>
public class QuestComponent : IComponent, ISerializable
{
    public Guid EntityId { get; set; }
    
    /// <summary>
    /// List of all quests currently tracked by this entity
    /// </summary>
    public List<Quest> Quests { get; set; } = new();
    
    /// <summary>
    /// Maximum number of active quests allowed
    /// </summary>
    public int MaxActiveQuests { get; set; } = 10;
    
    /// <summary>
    /// Gets all active quests
    /// </summary>
    public IEnumerable<Quest> ActiveQuests => Quests.Where(q => q.Status == QuestStatus.Active);
    
    /// <summary>
    /// Gets all available quests
    /// </summary>
    public IEnumerable<Quest> AvailableQuests => Quests.Where(q => q.Status == QuestStatus.Available);
    
    /// <summary>
    /// Gets all completed quests
    /// </summary>
    public IEnumerable<Quest> CompletedQuests => Quests.Where(q => q.Status == QuestStatus.Completed);
    
    /// <summary>
    /// Gets all failed quests
    /// </summary>
    public IEnumerable<Quest> FailedQuests => Quests.Where(q => q.Status == QuestStatus.Failed);
    
    /// <summary>
    /// Gets the number of active quests
    /// </summary>
    public int ActiveQuestCount => Quests.Count(q => q.Status == QuestStatus.Active);
    
    /// <summary>
    /// Whether this entity can accept more quests
    /// </summary>
    public bool CanAcceptMoreQuests => ActiveQuestCount < MaxActiveQuests;
    
    /// <summary>
    /// Add a quest to this component
    /// </summary>
    /// <param name="quest">Quest to add</param>
    /// <returns>True if quest was added, false if it already exists</returns>
    public bool AddQuest(Quest quest)
    {
        if (Quests.Any(q => q.Id == quest.Id))
            return false;
            
        Quests.Add(quest);
        return true;
    }
    
    /// <summary>
    /// Remove a quest from this component
    /// </summary>
    /// <param name="questId">ID of quest to remove</param>
    /// <returns>True if quest was removed</returns>
    public bool RemoveQuest(string questId)
    {
        var quest = Quests.FirstOrDefault(q => q.Id == questId);
        if (quest == null)
            return false;
            
        Quests.Remove(quest);
        return true;
    }
    
    /// <summary>
    /// Get a quest by ID
    /// </summary>
    /// <param name="questId">Quest ID</param>
    /// <returns>Quest if found, null otherwise</returns>
    public Quest? GetQuest(string questId)
    {
        return Quests.FirstOrDefault(q => q.Id == questId);
    }
    
    /// <summary>
    /// Accept a quest
    /// </summary>
    /// <param name="questId">ID of quest to accept</param>
    /// <returns>True if quest was accepted</returns>
    public bool AcceptQuest(string questId)
    {
        if (!CanAcceptMoreQuests)
            return false;
            
        var quest = GetQuest(questId);
        if (quest == null)
            return false;
            
        return quest.Accept();
    }
    
    /// <summary>
    /// Abandon a quest
    /// </summary>
    /// <param name="questId">ID of quest to abandon</param>
    /// <returns>True if quest was abandoned</returns>
    public bool AbandonQuest(string questId)
    {
        var quest = GetQuest(questId);
        if (quest == null || !quest.CanAbandon || quest.Status != QuestStatus.Active)
            return false;
            
        quest.Fail();
        return true;
    }
    
    /// <summary>
    /// Turn in a completed quest
    /// </summary>
    /// <param name="questId">ID of quest to turn in</param>
    /// <returns>True if quest was turned in</returns>
    public bool TurnInQuest(string questId)
    {
        var quest = GetQuest(questId);
        if (quest == null)
            return false;
            
        return quest.TurnIn();
    }
    
    /// <summary>
    /// Get all quests with a specific tag
    /// </summary>
    /// <param name="tag">Tag to search for</param>
    /// <returns>Quests with the specified tag</returns>
    public IEnumerable<Quest> GetQuestsByTag(string tag)
    {
        return Quests.Where(q => q.Tags.Contains(tag));
    }
    
    /// <summary>
    /// Serialize the component to a dictionary
    /// </summary>
    public Dictionary<string, object> Serialize()
    {
        var questsData = new List<Dictionary<string, object>>();
        
        foreach (var quest in Quests)
        {
            var objectivesData = new List<Dictionary<string, object>>();
            foreach (var obj in quest.Objectives)
            {
                objectivesData.Add(new Dictionary<string, object>
                {
                    ["Id"] = obj.Id,
                    ["Type"] = obj.Type.ToString(),
                    ["Description"] = obj.Description,
                    ["Target"] = obj.Target,
                    ["RequiredQuantity"] = obj.RequiredQuantity,
                    ["CurrentProgress"] = obj.CurrentProgress,
                    ["Status"] = obj.Status.ToString(),
                    ["IsOptional"] = obj.IsOptional,
                    ["IsHidden"] = obj.IsHidden
                });
            }
            
            var rewardsData = new List<Dictionary<string, object>>();
            foreach (var reward in quest.Rewards)
            {
                rewardsData.Add(new Dictionary<string, object>
                {
                    ["Type"] = reward.Type.ToString(),
                    ["RewardId"] = reward.RewardId,
                    ["Amount"] = reward.Amount,
                    ["Description"] = reward.Description
                });
            }
            
            questsData.Add(new Dictionary<string, object>
            {
                ["Id"] = quest.Id,
                ["Title"] = quest.Title,
                ["Description"] = quest.Description,
                ["Status"] = quest.Status.ToString(),
                ["Difficulty"] = quest.Difficulty.ToString(),
                ["CanAbandon"] = quest.CanAbandon,
                ["IsRepeatable"] = quest.IsRepeatable,
                ["TimeLimit"] = quest.TimeLimit,
                ["AcceptedTime"] = quest.AcceptedTime?.ToString("o") ?? string.Empty,
                ["CompletedTime"] = quest.CompletedTime?.ToString("o") ?? string.Empty,
                ["Objectives"] = objectivesData,
                ["Rewards"] = rewardsData,
                ["Tags"] = quest.Tags,
                ["Prerequisites"] = quest.Prerequisites,
                ["UnlocksQuests"] = quest.UnlocksQuests
            });
        }
        
        return new Dictionary<string, object>
        {
            ["EntityId"] = EntityId.ToString(),
            ["MaxActiveQuests"] = MaxActiveQuests,
            ["Quests"] = questsData
        };
    }
    
    /// <summary>
    /// Deserialize the component from a dictionary
    /// </summary>
    public void Deserialize(Dictionary<string, object> data)
    {
        EntityId = Guid.Parse(SerializationHelper.GetValue(data, "EntityId", Guid.Empty.ToString()));
        MaxActiveQuests = SerializationHelper.GetValue(data, "MaxActiveQuests", 10);
        
        Quests.Clear();
        
        if (!data.ContainsKey("Quests"))
            return;
            
        List<Dictionary<string, object>> questsData;
        
        if (data["Quests"] is JsonElement questsElement)
        {
            questsData = JsonSerializer.Deserialize<List<Dictionary<string, object>>>(questsElement.GetRawText())
                ?? new List<Dictionary<string, object>>();
        }
        else
        {
            questsData = data["Quests"] as List<Dictionary<string, object>> ?? new List<Dictionary<string, object>>();
        }
        
        foreach (var questData in questsData)
        {
            var quest = DeserializeQuest(questData);
            if (quest != null)
            {
                Quests.Add(quest);
            }
        }
    }
    
    private static Quest? DeserializeQuest(Dictionary<string, object> data)
    {
        try
        {
            var quest = new Quest
            {
                Id = SerializationHelper.GetValue(data, "Id", string.Empty),
                Title = SerializationHelper.GetValue(data, "Title", string.Empty),
                Description = SerializationHelper.GetValue(data, "Description", string.Empty),
                CanAbandon = SerializationHelper.GetValue(data, "CanAbandon", true),
                IsRepeatable = SerializationHelper.GetValue(data, "IsRepeatable", false),
                TimeLimit = SerializationHelper.GetValue(data, "TimeLimit", 0)
            };
            
            if (Enum.TryParse<QuestStatus>(SerializationHelper.GetValue(data, "Status", "Available"), out var status))
                quest.Status = status;
                
            if (Enum.TryParse<QuestDifficulty>(SerializationHelper.GetValue(data, "Difficulty", "Normal"), out var difficulty))
                quest.Difficulty = difficulty;
                
            var acceptedTimeStr = SerializationHelper.GetValue(data, "AcceptedTime", string.Empty);
            if (!string.IsNullOrEmpty(acceptedTimeStr) && DateTime.TryParse(acceptedTimeStr, out var acceptedTime))
                quest.AcceptedTime = acceptedTime;
                
            var completedTimeStr = SerializationHelper.GetValue(data, "CompletedTime", string.Empty);
            if (!string.IsNullOrEmpty(completedTimeStr) && DateTime.TryParse(completedTimeStr, out var completedTime))
                quest.CompletedTime = completedTime;
            
            // Deserialize objectives
            if (data.ContainsKey("Objectives"))
            {
                List<Dictionary<string, object>> objectivesData;
                
                if (data["Objectives"] is JsonElement objElement)
                {
                    objectivesData = JsonSerializer.Deserialize<List<Dictionary<string, object>>>(objElement.GetRawText())
                        ?? new List<Dictionary<string, object>>();
                }
                else
                {
                    objectivesData = data["Objectives"] as List<Dictionary<string, object>> ?? new List<Dictionary<string, object>>();
                }
                
                foreach (var objData in objectivesData)
                {
                    var objective = new QuestObjective
                    {
                        Id = SerializationHelper.GetValue(objData, "Id", string.Empty),
                        Description = SerializationHelper.GetValue(objData, "Description", string.Empty),
                        Target = SerializationHelper.GetValue(objData, "Target", string.Empty),
                        RequiredQuantity = SerializationHelper.GetValue(objData, "RequiredQuantity", 1),
                        CurrentProgress = SerializationHelper.GetValue(objData, "CurrentProgress", 0),
                        IsOptional = SerializationHelper.GetValue(objData, "IsOptional", false),
                        IsHidden = SerializationHelper.GetValue(objData, "IsHidden", false)
                    };
                    
                    if (Enum.TryParse<ObjectiveType>(SerializationHelper.GetValue(objData, "Type", "Collect"), out var objType))
                        objective.Type = objType;
                        
                    if (Enum.TryParse<ObjectiveStatus>(SerializationHelper.GetValue(objData, "Status", "NotStarted"), out var objStatus))
                        objective.Status = objStatus;
                    
                    quest.Objectives.Add(objective);
                }
            }
            
            // Deserialize rewards
            if (data.ContainsKey("Rewards"))
            {
                List<Dictionary<string, object>> rewardsData;
                
                if (data["Rewards"] is JsonElement rwdElement)
                {
                    rewardsData = JsonSerializer.Deserialize<List<Dictionary<string, object>>>(rwdElement.GetRawText())
                        ?? new List<Dictionary<string, object>>();
                }
                else
                {
                    rewardsData = data["Rewards"] as List<Dictionary<string, object>> ?? new List<Dictionary<string, object>>();
                }
                
                foreach (var rwdData in rewardsData)
                {
                    var reward = new QuestReward
                    {
                        RewardId = SerializationHelper.GetValue(rwdData, "RewardId", string.Empty),
                        Amount = SerializationHelper.GetValue(rwdData, "Amount", 0),
                        Description = SerializationHelper.GetValue(rwdData, "Description", string.Empty)
                    };
                    
                    if (Enum.TryParse<RewardType>(SerializationHelper.GetValue(rwdData, "Type", "Credits"), out var rwdType))
                        reward.Type = rwdType;
                    
                    quest.Rewards.Add(reward);
                }
            }
            
            // Deserialize tags
            if (data.ContainsKey("Tags"))
            {
                if (data["Tags"] is JsonElement tagsElement)
                {
                    quest.Tags = JsonSerializer.Deserialize<List<string>>(tagsElement.GetRawText()) ?? new List<string>();
                }
                else if (data["Tags"] is List<string> tagsList)
                {
                    quest.Tags = tagsList;
                }
            }
            
            // Deserialize prerequisites
            if (data.ContainsKey("Prerequisites"))
            {
                if (data["Prerequisites"] is JsonElement prereqElement)
                {
                    quest.Prerequisites = JsonSerializer.Deserialize<List<string>>(prereqElement.GetRawText()) ?? new List<string>();
                }
                else if (data["Prerequisites"] is List<string> prereqList)
                {
                    quest.Prerequisites = prereqList;
                }
            }
            
            // Deserialize unlock quests
            if (data.ContainsKey("UnlocksQuests"))
            {
                if (data["UnlocksQuests"] is JsonElement unlocksElement)
                {
                    quest.UnlocksQuests = JsonSerializer.Deserialize<List<string>>(unlocksElement.GetRawText()) ?? new List<string>();
                }
                else if (data["UnlocksQuests"] is List<string> unlocksList)
                {
                    quest.UnlocksQuests = unlocksList;
                }
            }
            
            return quest;
        }
        catch
        {
            return null;
        }
    }
}
