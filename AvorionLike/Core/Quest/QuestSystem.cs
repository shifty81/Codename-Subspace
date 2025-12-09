using AvorionLike.Core.ECS;
using AvorionLike.Core.Events;
using AvorionLike.Core.Logging;

namespace AvorionLike.Core.Quest;

/// <summary>
/// System that manages quest logic, progression, and events
/// </summary>
public class QuestSystem : SystemBase
{
    private readonly EntityManager _entityManager;
    private readonly EventSystem _eventSystem;
    
    /// <summary>
    /// All quest templates available in the game
    /// </summary>
    private readonly Dictionary<string, Quest> _questTemplates = new();
    
    /// <summary>
    /// Create a new quest system
    /// </summary>
    /// <param name="entityManager">Entity manager</param>
    /// <param name="eventSystem">Event system</param>
    public QuestSystem(EntityManager entityManager, EventSystem eventSystem) : base("QuestSystem")
    {
        _entityManager = entityManager ?? throw new ArgumentNullException(nameof(entityManager));
        _eventSystem = eventSystem ?? throw new ArgumentNullException(nameof(eventSystem));
        
        // Subscribe to relevant events
        _eventSystem.Subscribe("EntityDestroyed", OnEntityDestroyed);
        _eventSystem.Subscribe(GameEvents.ResourceCollected, OnResourceCollected);
        _eventSystem.Subscribe("ResourceMined", OnResourceMined);
    }
    
    /// <summary>
    /// Update all quest components
    /// </summary>
    /// <param name="deltaTime">Time elapsed since last update in seconds</param>
    public override void Update(float deltaTime)
    {
        var questComponents = _entityManager.GetAllComponents<QuestComponent>();
        
        foreach (var questComponent in questComponents)
        {
            UpdateQuestComponent(questComponent);
        }
    }
    
    /// <summary>
    /// Update a single quest component
    /// </summary>
    /// <param name="questComponent">Quest component to update</param>
    private void UpdateQuestComponent(QuestComponent questComponent)
    {
        foreach (var quest in questComponent.Quests)
        {
            var previousStatus = quest.Status;
            quest.Update();
            
            // Check if quest status changed
            if (quest.Status != previousStatus)
            {
                HandleQuestStatusChange(questComponent.EntityId, quest, previousStatus);
            }
        }
    }
    
    /// <summary>
    /// Handle quest status changes
    /// </summary>
    private void HandleQuestStatusChange(Guid entityId, Quest quest, QuestStatus previousStatus)
    {
        switch (quest.Status)
        {
            case QuestStatus.Active:
                Logger.Instance.Info("QuestSystem", $"Quest '{quest.Title}' accepted by entity {entityId}");
                _eventSystem.Publish("QuestAccepted", new QuestEvent
                {
                    EntityId = entityId,
                    QuestId = quest.Id,
                    QuestTitle = quest.Title
                });
                break;
                
            case QuestStatus.Completed:
                Logger.Instance.Info("QuestSystem", $"Quest '{quest.Title}' completed by entity {entityId}");
                _eventSystem.Publish("QuestCompleted", new QuestEvent
                {
                    EntityId = entityId,
                    QuestId = quest.Id,
                    QuestTitle = quest.Title
                });
                break;
                
            case QuestStatus.Failed:
                Logger.Instance.Info("QuestSystem", $"Quest '{quest.Title}' failed for entity {entityId}");
                _eventSystem.Publish("QuestFailed", new QuestEvent
                {
                    EntityId = entityId,
                    QuestId = quest.Id,
                    QuestTitle = quest.Title
                });
                break;
                
            case QuestStatus.TurnedIn:
                Logger.Instance.Info("QuestSystem", $"Quest '{quest.Title}' turned in by entity {entityId}");
                GiveQuestRewards(entityId, quest);
                _eventSystem.Publish("QuestTurnedIn", new QuestEvent
                {
                    EntityId = entityId,
                    QuestId = quest.Id,
                    QuestTitle = quest.Title
                });
                break;
        }
    }
    
    /// <summary>
    /// Give rewards to entity for completing a quest
    /// </summary>
    private void GiveQuestRewards(Guid entityId, Quest quest)
    {
        foreach (var reward in quest.Rewards)
        {
            _eventSystem.Publish("QuestRewardGiven", new QuestRewardEvent
            {
                EntityId = entityId,
                QuestId = quest.Id,
                RewardType = reward.Type,
                RewardId = reward.RewardId,
                Amount = reward.Amount
            });
            
            Logger.Instance.Info("QuestSystem", $"Gave reward: {reward.Description} ({reward.Amount}) to entity {entityId}");
        }
    }
    
    /// <summary>
    /// Add a quest template that can be instantiated for entities
    /// </summary>
    /// <param name="quest">Quest template</param>
    public void AddQuestTemplate(Quest quest)
    {
        if (_questTemplates.ContainsKey(quest.Id))
        {
            Logger.Instance.Warning("QuestSystem", $"Quest template '{quest.Id}' already exists, replacing");
        }
        
        _questTemplates[quest.Id] = quest;
        Logger.Instance.Info("QuestSystem", $"Added quest template: {quest.Title} ({quest.Id})");
    }
    
    /// <summary>
    /// Create a quest instance from a template
    /// </summary>
    /// <param name="templateId">Template ID</param>
    /// <returns>New quest instance, or null if template not found</returns>
    public Quest? CreateQuestFromTemplate(string templateId)
    {
        if (!_questTemplates.TryGetValue(templateId, out var template))
        {
            Logger.Instance.Warning("QuestSystem", $"Quest template '{templateId}' not found");
            return null;
        }
        
        // Create a deep copy of the quest
        var quest = new Quest
        {
            Id = Guid.NewGuid().ToString(), // New unique ID for this instance
            Title = template.Title,
            Description = template.Description,
            Difficulty = template.Difficulty,
            CanAbandon = template.CanAbandon,
            IsRepeatable = template.IsRepeatable,
            TimeLimit = template.TimeLimit,
            Prerequisites = new List<string>(template.Prerequisites),
            UnlocksQuests = new List<string>(template.UnlocksQuests),
            Tags = new List<string>(template.Tags),
            QuestGiverId = template.QuestGiverId,
            QuestGiverLocation = template.QuestGiverLocation
        };
        
        // Copy objectives
        foreach (var objTemplate in template.Objectives)
        {
            quest.Objectives.Add(new QuestObjective
            {
                Id = Guid.NewGuid().ToString(),
                Type = objTemplate.Type,
                Description = objTemplate.Description,
                Target = objTemplate.Target,
                RequiredQuantity = objTemplate.RequiredQuantity,
                IsOptional = objTemplate.IsOptional,
                IsHidden = objTemplate.IsHidden,
                Prerequisites = new List<string>(objTemplate.Prerequisites)
            });
        }
        
        // Copy rewards
        foreach (var rewardTemplate in template.Rewards)
        {
            quest.Rewards.Add(new QuestReward
            {
                Type = rewardTemplate.Type,
                RewardId = rewardTemplate.RewardId,
                Amount = rewardTemplate.Amount,
                Description = rewardTemplate.Description
            });
        }
        
        return quest;
    }
    
    /// <summary>
    /// Give a quest to an entity
    /// </summary>
    /// <param name="entityId">Entity to give quest to</param>
    /// <param name="questTemplateId">Quest template ID</param>
    /// <returns>True if quest was given successfully</returns>
    public bool GiveQuest(Guid entityId, string questTemplateId)
    {
        var questComponent = _entityManager.GetComponent<QuestComponent>(entityId);
        if (questComponent == null)
        {
            Logger.Instance.Warning("QuestSystem", $"Entity {entityId} does not have a QuestComponent");
            return false;
        }
        
        var quest = CreateQuestFromTemplate(questTemplateId);
        if (quest == null)
        {
            return false;
        }
        
        if (!questComponent.AddQuest(quest))
        {
            Logger.Instance.Warning("QuestSystem", $"Failed to add quest '{quest.Title}' to entity {entityId}");
            return false;
        }
        
        Logger.Instance.Info("QuestSystem", $"Gave quest '{quest.Title}' to entity {entityId}");
        _eventSystem.Publish("QuestOffered", new QuestEvent
        {
            EntityId = entityId,
            QuestId = quest.Id,
            QuestTitle = quest.Title
        });
        
        return true;
    }
    
    /// <summary>
    /// Progress a quest objective
    /// </summary>
    /// <param name="entityId">Entity with the quest</param>
    /// <param name="objectiveType">Type of objective</param>
    /// <param name="target">Target identifier</param>
    /// <param name="amount">Amount to progress</param>
    public void ProgressObjective(Guid entityId, ObjectiveType objectiveType, string target, int amount = 1)
    {
        var questComponent = _entityManager.GetComponent<QuestComponent>(entityId);
        if (questComponent == null)
            return;
            
        foreach (var quest in questComponent.ActiveQuests)
        {
            foreach (var objective in quest.Objectives.Where(o => o.Status == ObjectiveStatus.Active))
            {
                if (objective.Type == objectiveType && objective.Target == target)
                {
                    bool wasCompleted = objective.Progress(amount);
                    
                    if (wasCompleted)
                    {
                        Logger.Instance.Info("QuestSystem", $"Objective completed: {objective.Description} for quest '{quest.Title}'");
                        _eventSystem.Publish("QuestObjectiveCompleted", new QuestObjectiveEvent
                        {
                            EntityId = entityId,
                            QuestId = quest.Id,
                            ObjectiveId = objective.Id,
                            ObjectiveDescription = objective.Description
                        });
                    }
                }
            }
        }
    }
    
    // Event handlers
    
    private void OnEntityDestroyed(GameEvent eventData)
    {
        if (eventData is not EntityEvent entityEvent)
            return;
            
        // Progress "Destroy" objectives for all entities tracking this
        var allQuestComponents = _entityManager.GetAllComponents<QuestComponent>();
        foreach (var questComponent in allQuestComponents)
        {
            // Get entity type or name for targeting
            var entity = _entityManager.GetEntity(entityEvent.EntityId);
            if (entity != null)
            {
                // You could check entity tags, type, or name here
                // For now, we'll use a generic "enemy" target
                ProgressObjective(questComponent.EntityId, ObjectiveType.Destroy, "enemy", 1);
            }
        }
    }
    
    private void OnResourceCollected(GameEvent eventData)
    {
        if (eventData is not ResourceEvent resourceEvent)
            return;
            
        ProgressObjective(resourceEvent.EntityId, ObjectiveType.Collect, 
            resourceEvent.ResourceType, resourceEvent.Amount);
    }
    
    private void OnResourceMined(GameEvent eventData)
    {
        if (eventData is not ResourceEvent resourceEvent)
            return;
            
        ProgressObjective(resourceEvent.EntityId, ObjectiveType.Mine, 
            resourceEvent.ResourceType, resourceEvent.Amount);
    }
    
    /// <summary>
    /// Get all quest templates
    /// </summary>
    /// <returns>All quest templates</returns>
    public IReadOnlyDictionary<string, Quest> GetQuestTemplates()
    {
        return _questTemplates;
    }
}

/// <summary>
/// Event data for quest-related events
/// </summary>
public class QuestEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public string QuestId { get; set; } = string.Empty;
    public string QuestTitle { get; set; } = string.Empty;
}

/// <summary>
/// Event data for quest objective events
/// </summary>
public class QuestObjectiveEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public string QuestId { get; set; } = string.Empty;
    public string ObjectiveId { get; set; } = string.Empty;
    public string ObjectiveDescription { get; set; } = string.Empty;
}

/// <summary>
/// Event data for quest reward events
/// </summary>
public class QuestRewardEvent : GameEvent
{
    public Guid EntityId { get; set; }
    public string QuestId { get; set; } = string.Empty;
    public RewardType RewardType { get; set; }
    public string RewardId { get; set; } = string.Empty;
    public int Amount { get; set; }
}
