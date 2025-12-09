using System.Text.Json.Serialization;

namespace AvorionLike.Core.Quest;

/// <summary>
/// Type of quest objective
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum ObjectiveType
{
    /// <summary>
    /// Destroy specific number of entities
    /// </summary>
    Destroy,
    
    /// <summary>
    /// Collect specific amount of resources
    /// </summary>
    Collect,
    
    /// <summary>
    /// Mine specific amount of resources
    /// </summary>
    Mine,
    
    /// <summary>
    /// Visit a specific location
    /// </summary>
    Visit,
    
    /// <summary>
    /// Trade specific amount at a station
    /// </summary>
    Trade,
    
    /// <summary>
    /// Build specific blocks on a ship
    /// </summary>
    Build,
    
    /// <summary>
    /// Escort an entity to a location
    /// </summary>
    Escort,
    
    /// <summary>
    /// Scan specific number of objects
    /// </summary>
    Scan,
    
    /// <summary>
    /// Deliver items to a location
    /// </summary>
    Deliver,
    
    /// <summary>
    /// Talk to an NPC
    /// </summary>
    Talk
}

/// <summary>
/// Status of a quest objective
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum ObjectiveStatus
{
    /// <summary>
    /// Not yet started
    /// </summary>
    NotStarted,
    
    /// <summary>
    /// Currently active
    /// </summary>
    Active,
    
    /// <summary>
    /// Completed successfully
    /// </summary>
    Completed,
    
    /// <summary>
    /// Failed to complete
    /// </summary>
    Failed
}

/// <summary>
/// A single objective within a quest
/// </summary>
public class QuestObjective
{
    /// <summary>
    /// Unique identifier for this objective
    /// </summary>
    public string Id { get; set; } = Guid.NewGuid().ToString();
    
    /// <summary>
    /// Type of objective
    /// </summary>
    public ObjectiveType Type { get; set; }
    
    /// <summary>
    /// Description shown to player
    /// </summary>
    public string Description { get; set; } = string.Empty;
    
    /// <summary>
    /// Target for the objective (entity type, resource type, location, etc.)
    /// </summary>
    public string Target { get; set; } = string.Empty;
    
    /// <summary>
    /// Required quantity to complete
    /// </summary>
    public int RequiredQuantity { get; set; } = 1;
    
    /// <summary>
    /// Current progress toward required quantity
    /// </summary>
    public int CurrentProgress { get; set; } = 0;
    
    /// <summary>
    /// Current status of this objective
    /// </summary>
    public ObjectiveStatus Status { get; set; } = ObjectiveStatus.NotStarted;
    
    /// <summary>
    /// Whether this objective is optional
    /// </summary>
    public bool IsOptional { get; set; } = false;
    
    /// <summary>
    /// Whether this objective is hidden until certain conditions are met
    /// </summary>
    public bool IsHidden { get; set; } = false;
    
    /// <summary>
    /// List of objective IDs that must be completed before this one becomes active
    /// </summary>
    public List<string> Prerequisites { get; set; } = new();
    
    /// <summary>
    /// Gets the completion percentage (0-100)
    /// </summary>
    public float CompletionPercentage => RequiredQuantity > 0 
        ? Math.Min(100f, (float)CurrentProgress / RequiredQuantity * 100f) 
        : 0f;
    
    /// <summary>
    /// Whether this objective is complete
    /// </summary>
    public bool IsComplete => Status == ObjectiveStatus.Completed;
    
    /// <summary>
    /// Whether this objective has failed
    /// </summary>
    public bool IsFailed => Status == ObjectiveStatus.Failed;
    
    /// <summary>
    /// Progress this objective by a specified amount
    /// </summary>
    /// <param name="amount">Amount to progress</param>
    /// <returns>True if objective was completed by this progress</returns>
    public bool Progress(int amount = 1)
    {
        if (Status != ObjectiveStatus.Active)
            return false;
            
        CurrentProgress += amount;
        
        if (CurrentProgress >= RequiredQuantity)
        {
            CurrentProgress = RequiredQuantity;
            Status = ObjectiveStatus.Completed;
            return true;
        }
        
        return false;
    }
    
    /// <summary>
    /// Activate this objective
    /// </summary>
    public void Activate()
    {
        if (Status == ObjectiveStatus.NotStarted)
        {
            Status = ObjectiveStatus.Active;
        }
    }
    
    /// <summary>
    /// Fail this objective
    /// </summary>
    public void Fail()
    {
        Status = ObjectiveStatus.Failed;
    }
    
    /// <summary>
    /// Reset this objective
    /// </summary>
    public void Reset()
    {
        CurrentProgress = 0;
        Status = ObjectiveStatus.NotStarted;
    }
}
