using AvorionLike.Core.ECS;
using AvorionLike.Core.Events;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Tutorial;

/// <summary>
/// System that manages tutorial progression and events
/// </summary>
public class TutorialSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly EventSystem _eventSystem;
    
    /// <summary>
    /// All tutorial templates available in the game
    /// </summary>
    private readonly Dictionary<string, Tutorial> _tutorialTemplates = new();
    
    /// <summary>
    /// Active tutorials for each entity
    /// </summary>
    private readonly Dictionary<Guid, List<Tutorial>> _activeTutorials = new();
    
    /// <summary>
    /// Completed tutorial IDs for each entity
    /// </summary>
    private readonly Dictionary<Guid, HashSet<string>> _completedTutorials = new();
    
    /// <summary>
    /// Create a new tutorial system
    /// </summary>
    /// <param name="entityManager">Entity manager</param>
    /// <param name="eventSystem">Event system</param>
    public TutorialSystem(EntityManager entityManager, EventSystem eventSystem) : base("TutorialSystem")
    {
        _entityManager = entityManager ?? throw new ArgumentNullException(nameof(entityManager));
        _eventSystem = eventSystem ?? throw new ArgumentNullException(nameof(eventSystem));
        
        // Subscribe to game events to track player actions
        _eventSystem.Subscribe(GameEvents.EntityCreated, OnEntityCreated);
        _eventSystem.Subscribe(GameEvents.ResourceCollected, OnResourceCollected);
        _eventSystem.Subscribe("EntityDestroyed", OnEntityDestroyed);
        _eventSystem.Subscribe("QuestAccepted", OnQuestAccepted);
    }
    
    /// <summary>
    /// Update all active tutorials
    /// </summary>
    /// <param name="deltaTime">Time elapsed since last update in seconds</param>
    public override void Update(float deltaTime)
    {
        foreach (var kvp in _activeTutorials.ToList())
        {
            var entityId = kvp.Key;
            var tutorials = kvp.Value;
            
            for (int i = tutorials.Count - 1; i >= 0; i--)
            {
                var tutorial = tutorials[i];
                var previousStatus = tutorial.Status;
                
                tutorial.Update();
                
                // Check if tutorial status changed
                if (tutorial.Status != previousStatus)
                {
                    HandleTutorialStatusChange(entityId, tutorial, previousStatus);
                    
                    // Remove completed/skipped tutorials
                    if (tutorial.Status == TutorialStatus.Completed || 
                        tutorial.Status == TutorialStatus.Skipped)
                    {
                        tutorials.RemoveAt(i);
                        
                        // Track completed tutorials
                        if (!_completedTutorials.ContainsKey(entityId))
                        {
                            _completedTutorials[entityId] = new HashSet<string>();
                        }
                        _completedTutorials[entityId].Add(tutorial.Id);
                        
                        // Check for tutorials that can now be started
                        CheckAutoStartTutorials(entityId);
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// Handle tutorial status changes
    /// </summary>
    private void HandleTutorialStatusChange(Guid entityId, Tutorial tutorial, TutorialStatus previousStatus)
    {
        switch (tutorial.Status)
        {
            case TutorialStatus.Active:
                Logger.Instance.Info("TutorialSystem", $"Tutorial '{tutorial.Title}' started for entity {entityId}");
                _eventSystem.Publish("TutorialStarted", new TutorialEvent
                {
                    EntityId = entityId,
                    TutorialId = tutorial.Id,
                    TutorialTitle = tutorial.Title
                });
                break;
                
            case TutorialStatus.Completed:
                Logger.Instance.Info("TutorialSystem", $"Tutorial '{tutorial.Title}' completed by entity {entityId}");
                _eventSystem.Publish("TutorialCompleted", new TutorialEvent
                {
                    EntityId = entityId,
                    TutorialId = tutorial.Id,
                    TutorialTitle = tutorial.Title
                });
                break;
                
            case TutorialStatus.Skipped:
                Logger.Instance.Info("TutorialSystem", $"Tutorial '{tutorial.Title}' skipped by entity {entityId}");
                _eventSystem.Publish("TutorialSkipped", new TutorialEvent
                {
                    EntityId = entityId,
                    TutorialId = tutorial.Id,
                    TutorialTitle = tutorial.Title
                });
                break;
        }
    }
    
    /// <summary>
    /// Add a tutorial template
    /// </summary>
    /// <param name="tutorial">Tutorial template</param>
    public void AddTutorialTemplate(Tutorial tutorial)
    {
        if (_tutorialTemplates.ContainsKey(tutorial.Id))
        {
            Logger.Instance.Warning("TutorialSystem", $"Tutorial template '{tutorial.Id}' already exists, replacing");
        }
        
        _tutorialTemplates[tutorial.Id] = tutorial;
        Logger.Instance.Info("TutorialSystem", $"Added tutorial template: {tutorial.Title} ({tutorial.Id})");
    }
    
    /// <summary>
    /// Start a tutorial for an entity
    /// </summary>
    /// <param name="entityId">Entity to start tutorial for</param>
    /// <param name="tutorialId">Tutorial template ID</param>
    /// <returns>True if tutorial was started</returns>
    public bool StartTutorial(Guid entityId, string tutorialId)
    {
        if (!_tutorialTemplates.TryGetValue(tutorialId, out var template))
        {
            Logger.Instance.Warning("TutorialSystem", $"Tutorial template '{tutorialId}' not found");
            return false;
        }
        
        // Check if already completed
        if (_completedTutorials.TryGetValue(entityId, out var completed) && 
            completed.Contains(tutorialId))
        {
            Logger.Instance.Info("TutorialSystem", $"Tutorial '{tutorialId}' already completed by entity {entityId}");
            return false;
        }
        
        // Check prerequisites
        if (!ArePrerequisitesMet(entityId, template.Prerequisites))
        {
            Logger.Instance.Warning("TutorialSystem", $"Prerequisites not met for tutorial '{tutorialId}'");
            return false;
        }
        
        // Create instance from template
        var tutorial = CloneTutorial(template);
        
        if (!_activeTutorials.ContainsKey(entityId))
        {
            _activeTutorials[entityId] = new List<Tutorial>();
        }
        
        _activeTutorials[entityId].Add(tutorial);
        tutorial.Start();
        
        return true;
    }
    
    /// <summary>
    /// Complete the current step of a tutorial
    /// </summary>
    /// <param name="entityId">Entity with the tutorial</param>
    /// <param name="tutorialId">Tutorial ID (optional, completes first active if null)</param>
    public void CompleteCurrentStep(Guid entityId, string? tutorialId = null)
    {
        if (!_activeTutorials.TryGetValue(entityId, out var tutorials))
            return;
            
        Tutorial? targetTutorial = tutorialId != null 
            ? tutorials.FirstOrDefault(t => t.Id == tutorialId)
            : tutorials.FirstOrDefault(t => t.Status == TutorialStatus.Active);
            
        if (targetTutorial != null)
        {
            targetTutorial.CompleteCurrentStep();
            
            _eventSystem.Publish("TutorialStepCompleted", new TutorialStepEvent
            {
                EntityId = entityId,
                TutorialId = targetTutorial.Id,
                StepTitle = targetTutorial.CurrentStep?.Title ?? "Unknown"
            });
        }
    }
    
    /// <summary>
    /// Skip a tutorial
    /// </summary>
    /// <param name="entityId">Entity with the tutorial</param>
    /// <param name="tutorialId">Tutorial ID</param>
    public void SkipTutorial(Guid entityId, string tutorialId)
    {
        if (!_activeTutorials.TryGetValue(entityId, out var tutorials))
            return;
            
        var tutorial = tutorials.FirstOrDefault(t => t.Id == tutorialId);
        if (tutorial != null)
        {
            tutorial.Skip();
        }
    }
    
    /// <summary>
    /// Get active tutorials for an entity
    /// </summary>
    /// <param name="entityId">Entity ID</param>
    /// <returns>List of active tutorials</returns>
    public List<Tutorial> GetActiveTutorials(Guid entityId)
    {
        return _activeTutorials.TryGetValue(entityId, out var tutorials) 
            ? tutorials 
            : new List<Tutorial>();
    }
    
    /// <summary>
    /// Check if an entity has completed a tutorial
    /// </summary>
    /// <param name="entityId">Entity ID</param>
    /// <param name="tutorialId">Tutorial ID</param>
    /// <returns>True if completed</returns>
    public bool HasCompletedTutorial(Guid entityId, string tutorialId)
    {
        return _completedTutorials.TryGetValue(entityId, out var completed) && 
               completed.Contains(tutorialId);
    }
    
    /// <summary>
    /// Check if prerequisites are met
    /// </summary>
    private bool ArePrerequisitesMet(Guid entityId, List<string> prerequisites)
    {
        if (prerequisites.Count == 0)
            return true;
            
        if (!_completedTutorials.TryGetValue(entityId, out var completed))
            return false;
            
        return prerequisites.All(prereq => completed.Contains(prereq));
    }
    
    /// <summary>
    /// Check for auto-start tutorials
    /// </summary>
    private void CheckAutoStartTutorials(Guid entityId)
    {
        foreach (var template in _tutorialTemplates.Values.Where(t => t.AutoStart))
        {
            if (ArePrerequisitesMet(entityId, template.Prerequisites))
            {
                StartTutorial(entityId, template.Id);
            }
        }
    }
    
    /// <summary>
    /// Clone a tutorial template
    /// </summary>
    private Tutorial CloneTutorial(Tutorial template)
    {
        var tutorial = new Tutorial
        {
            Id = template.Id, // Keep template ID for tracking
            Title = template.Title,
            Description = template.Description,
            AutoStart = template.AutoStart,
            Prerequisites = new List<string>(template.Prerequisites)
        };
        
        foreach (var stepTemplate in template.Steps)
        {
            tutorial.Steps.Add(new TutorialStep
            {
                Type = stepTemplate.Type,
                Title = stepTemplate.Title,
                Message = stepTemplate.Message,
                RequiredKey = stepTemplate.RequiredKey,
                RequiredAction = stepTemplate.RequiredAction,
                UIElementId = stepTemplate.UIElementId,
                Duration = stepTemplate.Duration,
                CanSkip = stepTemplate.CanSkip
            });
        }
        
        return tutorial;
    }
    
    // Event handlers
    
    private void OnEntityCreated(GameEvent eventData)
    {
        if (eventData is EntityEvent entityEvent)
        {
            CheckAutoStartTutorials(entityEvent.EntityId);
        }
    }
    
    private void OnResourceCollected(GameEvent eventData)
    {
        if (eventData is ResourceEvent resourceEvent)
        {
            // Progress any tutorials waiting for resource collection
            CompleteActionBasedSteps(resourceEvent.EntityId, "collect_resource");
        }
    }
    
    private void OnEntityDestroyed(GameEvent eventData)
    {
        if (eventData is EntityEvent entityEvent)
        {
            // Progress any tutorials waiting for combat
            var allEntities = _activeTutorials.Keys.ToList();
            foreach (var entityId in allEntities)
            {
                CompleteActionBasedSteps(entityId, "destroy_entity");
            }
        }
    }
    
    private void OnQuestAccepted(GameEvent eventData)
    {
        // Quest system events can trigger tutorial progression
        CompleteActionBasedSteps(Guid.Empty, "accept_quest");
    }
    
    /// <summary>
    /// Complete steps waiting for a specific action
    /// </summary>
    private void CompleteActionBasedSteps(Guid entityId, string action)
    {
        if (!_activeTutorials.TryGetValue(entityId, out var tutorials))
            return;
            
        foreach (var tutorial in tutorials.Where(t => t.Status == TutorialStatus.Active))
        {
            var currentStep = tutorial.CurrentStep;
            if (currentStep != null &&
                currentStep.Type == TutorialStepType.WaitForAction &&
                currentStep.RequiredAction == action &&
                currentStep.Status == TutorialStepStatus.Active)
            {
                tutorial.CompleteCurrentStep();
            }
        }
    }
    
    /// <summary>
    /// Get all tutorial templates
    /// </summary>
    /// <returns>All tutorial templates</returns>
    public IReadOnlyDictionary<string, Tutorial> GetTutorialTemplates()
    {
        return _tutorialTemplates;
    }
}

/// <summary>
/// Event data for tutorial-related events
/// </summary>
public class TutorialEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public string TutorialId { get; set; } = string.Empty;
    public string TutorialTitle { get; set; } = string.Empty;
}

/// <summary>
/// Event data for tutorial step events
/// </summary>
public class TutorialStepEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public string TutorialId { get; set; } = string.Empty;
    public string StepTitle { get; set; } = string.Empty;
}
