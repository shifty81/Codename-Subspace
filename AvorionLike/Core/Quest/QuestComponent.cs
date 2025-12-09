using AvorionLike.Core.ECS;

namespace AvorionLike.Core.Quest;

/// <summary>
/// Component that tracks quests for an entity (typically the player)
/// </summary>
public class QuestComponent : IComponent
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
}
