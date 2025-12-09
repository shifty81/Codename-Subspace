using System.Text.Json.Serialization;

namespace AvorionLike.Core.Quest;

/// <summary>
/// Status of a quest
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum QuestStatus
{
    /// <summary>
    /// Quest is available but not yet accepted
    /// </summary>
    Available,
    
    /// <summary>
    /// Quest has been accepted and is in progress
    /// </summary>
    Active,
    
    /// <summary>
    /// Quest has been completed successfully
    /// </summary>
    Completed,
    
    /// <summary>
    /// Quest has failed
    /// </summary>
    Failed,
    
    /// <summary>
    /// Quest is turned in and rewards collected
    /// </summary>
    TurnedIn
}

/// <summary>
/// Difficulty level of a quest
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum QuestDifficulty
{
    /// <summary>
    /// Very easy quest
    /// </summary>
    Trivial,
    
    /// <summary>
    /// Easy quest
    /// </summary>
    Easy,
    
    /// <summary>
    /// Normal difficulty
    /// </summary>
    Normal,
    
    /// <summary>
    /// Hard quest
    /// </summary>
    Hard,
    
    /// <summary>
    /// Very hard quest
    /// </summary>
    Elite
}

/// <summary>
/// Type of reward
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum RewardType
{
    /// <summary>
    /// Credits reward
    /// </summary>
    Credits,
    
    /// <summary>
    /// Resource reward
    /// </summary>
    Resource,
    
    /// <summary>
    /// Experience points
    /// </summary>
    Experience,
    
    /// <summary>
    /// Reputation with faction
    /// </summary>
    Reputation,
    
    /// <summary>
    /// Item or equipment
    /// </summary>
    Item,
    
    /// <summary>
    /// Unlock new feature or blueprint
    /// </summary>
    Unlock
}

/// <summary>
/// A reward given for completing a quest
/// </summary>
public class QuestReward
{
    /// <summary>
    /// Type of reward
    /// </summary>
    public RewardType Type { get; set; }
    
    /// <summary>
    /// Identifier for the reward (resource type, item id, etc.)
    /// </summary>
    public string RewardId { get; set; } = string.Empty;
    
    /// <summary>
    /// Amount of the reward
    /// </summary>
    public int Amount { get; set; } = 0;
    
    /// <summary>
    /// Description of the reward
    /// </summary>
    public string Description { get; set; } = string.Empty;
}

/// <summary>
/// Represents a quest in the game
/// </summary>
public class Quest
{
    /// <summary>
    /// Unique identifier for this quest
    /// </summary>
    public string Id { get; set; } = Guid.NewGuid().ToString();
    
    /// <summary>
    /// Quest title shown to player
    /// </summary>
    public string Title { get; set; } = string.Empty;
    
    /// <summary>
    /// Quest description/lore
    /// </summary>
    public string Description { get; set; } = string.Empty;
    
    /// <summary>
    /// Current status of the quest
    /// </summary>
    public QuestStatus Status { get; set; } = QuestStatus.Available;
    
    /// <summary>
    /// Difficulty level
    /// </summary>
    public QuestDifficulty Difficulty { get; set; } = QuestDifficulty.Normal;
    
    /// <summary>
    /// List of objectives for this quest
    /// </summary>
    public List<QuestObjective> Objectives { get; set; } = new();
    
    /// <summary>
    /// List of rewards for completing this quest
    /// </summary>
    public List<QuestReward> Rewards { get; set; } = new();
    
    /// <summary>
    /// Quest giver entity ID (if applicable)
    /// </summary>
    public Guid? QuestGiverId { get; set; }
    
    /// <summary>
    /// Location where quest was given (sector coordinates as string)
    /// </summary>
    public string QuestGiverLocation { get; set; } = string.Empty;
    
    /// <summary>
    /// Whether this quest can be abandoned
    /// </summary>
    public bool CanAbandon { get; set; } = true;
    
    /// <summary>
    /// Whether this quest is repeatable
    /// </summary>
    public bool IsRepeatable { get; set; } = false;
    
    /// <summary>
    /// Quest IDs that must be completed before this quest becomes available
    /// </summary>
    public List<string> Prerequisites { get; set; } = new();
    
    /// <summary>
    /// Quest IDs that become available after completing this quest
    /// </summary>
    public List<string> UnlocksQuests { get; set; } = new();
    
    /// <summary>
    /// Time when the quest was accepted
    /// </summary>
    public DateTime? AcceptedTime { get; set; }
    
    /// <summary>
    /// Time when the quest was completed
    /// </summary>
    public DateTime? CompletedTime { get; set; }
    
    /// <summary>
    /// Time limit for completing the quest (in seconds, 0 = no limit)
    /// </summary>
    public int TimeLimit { get; set; } = 0;
    
    /// <summary>
    /// Tags for categorizing quests
    /// </summary>
    public List<string> Tags { get; set; } = new();
    
    /// <summary>
    /// Gets the overall completion percentage (0-100)
    /// </summary>
    public float CompletionPercentage
    {
        get
        {
            if (Objectives.Count == 0)
                return 0f;
                
            var requiredObjectives = Objectives.Where(o => !o.IsOptional).ToList();
            if (requiredObjectives.Count == 0)
                return 100f;
                
            float totalProgress = requiredObjectives.Sum(o => o.CompletionPercentage);
            return totalProgress / requiredObjectives.Count;
        }
    }
    
    /// <summary>
    /// Whether all required objectives are complete
    /// </summary>
    public bool AreRequiredObjectivesComplete => 
        Objectives.Where(o => !o.IsOptional).All(o => o.IsComplete);
    
    /// <summary>
    /// Whether any required objective has failed
    /// </summary>
    public bool HasFailedObjective => 
        Objectives.Where(o => !o.IsOptional).Any(o => o.IsFailed);
    
    /// <summary>
    /// Gets the time remaining in seconds (0 if no time limit or expired)
    /// </summary>
    public int TimeRemaining
    {
        get
        {
            if (TimeLimit <= 0 || !AcceptedTime.HasValue)
                return TimeLimit;
                
            var elapsed = (DateTime.UtcNow - AcceptedTime.Value).TotalSeconds;
            var remaining = TimeLimit - elapsed;
            return remaining > 0 ? (int)remaining : 0;
        }
    }
    
    /// <summary>
    /// Whether the quest has expired (time limit exceeded)
    /// </summary>
    public bool IsExpired => TimeLimit > 0 && TimeRemaining <= 0;
    
    /// <summary>
    /// Accept this quest
    /// </summary>
    /// <returns>True if quest was accepted, false if already accepted or failed</returns>
    public bool Accept()
    {
        if (Status != QuestStatus.Available)
            return false;
            
        Status = QuestStatus.Active;
        AcceptedTime = DateTime.UtcNow;
        
        // Activate first objectives that have no prerequisites
        foreach (var objective in Objectives.Where(o => o.Prerequisites.Count == 0))
        {
            objective.Activate();
        }
        
        return true;
    }
    
    /// <summary>
    /// Complete this quest
    /// </summary>
    /// <returns>True if quest was completed, false if not active or objectives incomplete</returns>
    public bool Complete()
    {
        if (Status != QuestStatus.Active)
            return false;
            
        if (!AreRequiredObjectivesComplete)
            return false;
            
        Status = QuestStatus.Completed;
        CompletedTime = DateTime.UtcNow;
        return true;
    }
    
    /// <summary>
    /// Fail this quest
    /// </summary>
    public void Fail()
    {
        Status = QuestStatus.Failed;
        CompletedTime = DateTime.UtcNow;
    }
    
    /// <summary>
    /// Turn in this quest and collect rewards
    /// </summary>
    /// <returns>True if quest was turned in successfully</returns>
    public bool TurnIn()
    {
        if (Status != QuestStatus.Completed)
            return false;
            
        Status = QuestStatus.TurnedIn;
        return true;
    }
    
    /// <summary>
    /// Update quest objectives and check for completion
    /// </summary>
    public void Update()
    {
        // Check for expired quest
        if (IsExpired && Status == QuestStatus.Active)
        {
            Fail();
            return;
        }
        
        // Check if any objectives have failed
        if (HasFailedObjective && Status == QuestStatus.Active)
        {
            Fail();
            return;
        }
        
        // Activate objectives whose prerequisites are met
        foreach (var objective in Objectives.Where(o => o.Status == ObjectiveStatus.NotStarted))
        {
            if (objective.Prerequisites.Count == 0)
                continue;
                
            var prerequisitesMet = objective.Prerequisites.All(prereqId =>
                Objectives.Any(o => o.Id == prereqId && o.IsComplete));
                
            if (prerequisitesMet)
            {
                objective.Activate();
            }
        }
        
        // Check for quest completion
        if (Status == QuestStatus.Active && AreRequiredObjectivesComplete)
        {
            Complete();
        }
    }
    
    /// <summary>
    /// Reset this quest to initial state
    /// </summary>
    public void Reset()
    {
        Status = QuestStatus.Available;
        AcceptedTime = null;
        CompletedTime = null;
        
        foreach (var objective in Objectives)
        {
            objective.Reset();
        }
    }
}
