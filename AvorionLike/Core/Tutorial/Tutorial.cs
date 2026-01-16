using System.Text.Json.Serialization;

namespace AvorionLike.Core.Tutorial;

/// <summary>
/// Status of a tutorial
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum TutorialStatus
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
/// Represents a complete tutorial sequence
/// </summary>
public class Tutorial
{
    /// <summary>
    /// Unique identifier for this tutorial
    /// </summary>
    public string Id { get; set; } = Guid.NewGuid().ToString();
    
    /// <summary>
    /// Title of the tutorial
    /// </summary>
    public string Title { get; set; } = string.Empty;
    
    /// <summary>
    /// Description of the tutorial
    /// </summary>
    public string Description { get; set; } = string.Empty;
    
    /// <summary>
    /// Current status of the tutorial
    /// </summary>
    public TutorialStatus Status { get; set; } = TutorialStatus.NotStarted;
    
    /// <summary>
    /// List of steps in this tutorial
    /// </summary>
    public List<TutorialStep> Steps { get; set; } = new();
    
    /// <summary>
    /// Index of the current step
    /// </summary>
    public int CurrentStepIndex { get; set; } = 0;
    
    /// <summary>
    /// Whether this tutorial should auto-start
    /// </summary>
    public bool AutoStart { get; set; } = false;
    
    /// <summary>
    /// Prerequisites - tutorial IDs that must be completed first
    /// </summary>
    public List<string> Prerequisites { get; set; } = new();
    
    /// <summary>
    /// Time when tutorial was started
    /// </summary>
    public DateTime? StartTime { get; set; }
    
    /// <summary>
    /// Time when tutorial was completed
    /// </summary>
    public DateTime? CompletedTime { get; set; }
    
    /// <summary>
    /// Gets the current step
    /// </summary>
    public TutorialStep? CurrentStep => 
        CurrentStepIndex >= 0 && CurrentStepIndex < Steps.Count 
            ? Steps[CurrentStepIndex] 
            : null;
    
    /// <summary>
    /// Gets the completion percentage (0-100)
    /// </summary>
    public float CompletionPercentage
    {
        get
        {
            if (Steps.Count == 0)
                return 0f;
                
            int completedSteps = Steps.Count(s => 
                s.Status == TutorialStepStatus.Completed || 
                s.Status == TutorialStepStatus.Skipped);
            return (float)completedSteps / Steps.Count * 100f;
        }
    }
    
    /// <summary>
    /// Whether all steps are complete
    /// </summary>
    public bool AreAllStepsComplete => 
        Steps.All(s => s.Status == TutorialStepStatus.Completed || 
                      s.Status == TutorialStepStatus.Skipped);
    
    /// <summary>
    /// Start this tutorial
    /// </summary>
    /// <returns>True if started successfully</returns>
    public bool Start()
    {
        if (Status != TutorialStatus.NotStarted)
            return false;
            
        Status = TutorialStatus.Active;
        StartTime = DateTime.UtcNow;
        CurrentStepIndex = 0;
        
        // Start first step
        if (CurrentStep != null)
        {
            CurrentStep.Start();
        }
        
        return true;
    }
    
    /// <summary>
    /// Complete the current step and move to next
    /// </summary>
    /// <returns>True if there are more steps</returns>
    public bool CompleteCurrentStep()
    {
        if (CurrentStep == null)
            return false;
            
        CurrentStep.Complete();
        CurrentStepIndex++;
        
        // Check if tutorial is complete
        if (CurrentStepIndex >= Steps.Count)
        {
            Complete();
            return false;
        }
        
        // Start next step
        if (CurrentStep != null)
        {
            CurrentStep.Start();
        }
        
        return true;
    }
    
    /// <summary>
    /// Skip the current step
    /// </summary>
    public void SkipCurrentStep()
    {
        if (CurrentStep != null && CurrentStep.CanSkip)
        {
            CurrentStep.Skip();
            CurrentStepIndex++;
            
            if (CurrentStepIndex >= Steps.Count)
            {
                Complete();
            }
            else if (CurrentStep != null)
            {
                CurrentStep.Start();
            }
        }
    }
    
    /// <summary>
    /// Skip the entire tutorial
    /// </summary>
    public void Skip()
    {
        Status = TutorialStatus.Skipped;
        CompletedTime = DateTime.UtcNow;
        
        foreach (var step in Steps.Where(s => s.Status == TutorialStepStatus.NotStarted || 
                                               s.Status == TutorialStepStatus.Active))
        {
            step.Skip();
        }
    }
    
    /// <summary>
    /// Complete this tutorial
    /// </summary>
    public void Complete()
    {
        Status = TutorialStatus.Completed;
        CompletedTime = DateTime.UtcNow;
    }
    
    /// <summary>
    /// Reset this tutorial to initial state
    /// </summary>
    public void Reset()
    {
        Status = TutorialStatus.NotStarted;
        StartTime = null;
        CompletedTime = null;
        CurrentStepIndex = 0;
        
        foreach (var step in Steps)
        {
            step.Reset();
        }
    }
    
    /// <summary>
    /// Update the tutorial (check for time-based steps)
    /// </summary>
    public void Update()
    {
        if (Status != TutorialStatus.Active || CurrentStep == null)
            return;
            
        // Check if current step is a time-based step that's complete
        if (CurrentStep.Type == TutorialStepType.WaitForTime && 
            CurrentStep.Status == TutorialStepStatus.Active &&
            CurrentStep.IsTimeElapsed())
        {
            CompleteCurrentStep();
        }
        
        // Check if all steps are complete
        if (AreAllStepsComplete && Status == TutorialStatus.Active)
        {
            Complete();
        }
    }
}
