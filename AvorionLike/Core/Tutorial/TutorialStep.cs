using System.Text.Json.Serialization;

namespace AvorionLike.Core.Tutorial;

/// <summary>
/// Type of tutorial step
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum TutorialStepType
{
    /// <summary>
    /// Show a message/instructions
    /// </summary>
    Message,
    
    /// <summary>
    /// Wait for player to press a specific key
    /// </summary>
    WaitForKey,
    
    /// <summary>
    /// Wait for player to perform an action
    /// </summary>
    WaitForAction,
    
    /// <summary>
    /// Highlight a UI element
    /// </summary>
    HighlightUI,
    
    /// <summary>
    /// Wait for a specific time
    /// </summary>
    WaitForTime
}

/// <summary>
/// Status of a tutorial step
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum TutorialStepStatus
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
    /// Completed
    /// </summary>
    Completed,
    
    /// <summary>
    /// Skipped by player
    /// </summary>
    Skipped
}

/// <summary>
/// Represents a single step in a tutorial sequence
/// </summary>
public class TutorialStep
{
    /// <summary>
    /// Unique identifier for this step
    /// </summary>
    public string Id { get; set; } = Guid.NewGuid().ToString();
    
    /// <summary>
    /// Type of step
    /// </summary>
    public TutorialStepType Type { get; set; } = TutorialStepType.Message;
    
    /// <summary>
    /// Title of the step
    /// </summary>
    public string Title { get; set; } = string.Empty;
    
    /// <summary>
    /// Message to display to the player
    /// </summary>
    public string Message { get; set; } = string.Empty;
    
    /// <summary>
    /// Current status of this step
    /// </summary>
    public TutorialStepStatus Status { get; set; } = TutorialStepStatus.NotStarted;
    
    /// <summary>
    /// Key to wait for (if Type is WaitForKey)
    /// </summary>
    public string? RequiredKey { get; set; }
    
    /// <summary>
    /// Action to wait for (if Type is WaitForAction)
    /// </summary>
    public string? RequiredAction { get; set; }
    
    /// <summary>
    /// UI element to highlight (if Type is HighlightUI)
    /// </summary>
    public string? UIElementId { get; set; }
    
    /// <summary>
    /// Duration in seconds (if Type is WaitForTime)
    /// </summary>
    public float Duration { get; set; } = 0f;
    
    /// <summary>
    /// Time when step was started
    /// </summary>
    public DateTime? StartTime { get; set; }
    
    /// <summary>
    /// Whether this step can be skipped
    /// </summary>
    public bool CanSkip { get; set; } = true;
    
    /// <summary>
    /// Start this step
    /// </summary>
    public void Start()
    {
        Status = TutorialStepStatus.Active;
        StartTime = DateTime.UtcNow;
    }
    
    /// <summary>
    /// Complete this step
    /// </summary>
    public void Complete()
    {
        Status = TutorialStepStatus.Completed;
    }
    
    /// <summary>
    /// Skip this step
    /// </summary>
    public void Skip()
    {
        if (CanSkip)
        {
            Status = TutorialStepStatus.Skipped;
        }
    }
    
    /// <summary>
    /// Reset this step
    /// </summary>
    public void Reset()
    {
        Status = TutorialStepStatus.NotStarted;
        StartTime = null;
    }
    
    /// <summary>
    /// Check if time-based step is complete
    /// </summary>
    public bool IsTimeElapsed()
    {
        if (Type != TutorialStepType.WaitForTime || !StartTime.HasValue)
            return false;
            
        var elapsed = (DateTime.UtcNow - StartTime.Value).TotalSeconds;
        return elapsed >= Duration;
    }
}
