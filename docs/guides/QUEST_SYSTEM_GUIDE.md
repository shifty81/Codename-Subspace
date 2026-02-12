# Quest System Guide

**Version:** 1.0  
**Last Updated:** December 9, 2025  
**Status:** ✅ Core Implementation Complete

---

## Overview

The Quest System provides structured objectives and progression for players. It supports various quest types, objective tracking, rewards, and integration with all game systems through the event system.

### Key Features
- **Multiple Objective Types** - Destroy, collect, mine, trade, build, escort, scan, deliver, talk
- **JSON-Based Quest Definitions** - Easy to create and modify quests
- **Progress Tracking** - Real-time objective tracking with percentages
- **Quest Chains** - Prerequisites and unlockable quests
- **Time Limits** - Optional quest expiration
- **Rewards** - Credits, resources, experience, reputation, items, unlocks
- **Event Integration** - Automatic progress tracking through game events
- **Repeatable Quests** - Support for daily/repeatable content

---

## Architecture

### Core Components

#### Quest
Represents a complete quest with objectives and rewards.

**Properties:**
- `Id` - Unique quest identifier
- `Title` - Display name
- `Description` - Quest lore/instructions
- `Status` - Available, Active, Completed, Failed, TurnedIn
- `Difficulty` - Trivial, Easy, Normal, Hard, Elite
- `Objectives` - List of quest objectives
- `Rewards` - List of rewards
- `TimeLimit` - Optional time limit in seconds
- `Prerequisites` - Quests that must be completed first
- `UnlocksQuests` - Quests unlocked by completing this one
- `Tags` - Category tags for filtering

#### QuestObjective
Individual objective within a quest.

**Objective Types:**
1. **Destroy** - Eliminate specific entities
2. **Collect** - Gather resources (any method)
3. **Mine** - Extract resources from asteroids
4. **Visit** - Travel to a location
5. **Trade** - Buy/sell at stations
6. **Build** - Construct blocks on ships
7. **Escort** - Protect an entity
8. **Scan** - Scan objects
9. **Deliver** - Transport items
10. **Talk** - Interact with NPCs

**Properties:**
- `Type` - Objective type
- `Description` - What the player needs to do
- `Target` - What to interact with (resource type, entity type, location)
- `RequiredQuantity` - How many needed
- `CurrentProgress` - Current progress
- `Status` - NotStarted, Active, Completed, Failed
- `IsOptional` - Whether objective is optional
- `IsHidden` - Whether to hide until conditions met
- `Prerequisites` - Other objectives that must complete first

#### QuestComponent
ECS component attached to entities (typically player) to track quests.

**Capabilities:**
- Track multiple active quests
- Maximum active quest limit
- Quest acceptance/abandonment
- Progress queries
- Tag-based filtering

#### QuestSystem
System that manages quest logic, progression, and events.

**Responsibilities:**
- Update quest states
- Track objective progress
- Distribute rewards
- Manage quest templates
- Handle quest events
- Subscribe to game events for automatic progress

---

## Creating Quests

### JSON Quest Format

Quests are defined in JSON files stored in `GameData/Quests/`.

**Example: Simple Mining Quest**
```json
{
  "Id": "quest_tutorial_mining",
  "Title": "First Steps: Mining",
  "Description": "Mine 100 Iron ore from asteroids",
  "Difficulty": "Easy",
  "CanAbandon": false,
  "IsRepeatable": false,
  "TimeLimit": 0,
  "Tags": ["tutorial", "mining"],
  "Objectives": [
    {
      "Type": "Mine",
      "Description": "Mine 100 Iron ore",
      "Target": "Iron",
      "RequiredQuantity": 100,
      "IsOptional": false,
      "IsHidden": false,
      "Prerequisites": []
    }
  ],
  "Rewards": [
    {
      "Type": "Credits",
      "RewardId": "credits",
      "Amount": 1000,
      "Description": "1,000 Credits"
    },
    {
      "Type": "Experience",
      "RewardId": "xp",
      "Amount": 50,
      "Description": "50 Experience"
    }
  ]
}
```

**Example: Multi-Objective Quest Chain**
```json
{
  "Id": "quest_advanced_shipyard",
  "Title": "Building a Fleet",
  "Description": "Establish your own shipyard",
  "Difficulty": "Hard",
  "CanAbandon": true,
  "IsRepeatable": false,
  "TimeLimit": 0,
  "Prerequisites": ["quest_tutorial_mining", "quest_tutorial_trading"],
  "UnlocksQuests": ["quest_fleet_command"],
  "Tags": ["building", "progression"],
  "Objectives": [
    {
      "Type": "Collect",
      "Description": "Gather 10,000 Iron",
      "Target": "Iron",
      "RequiredQuantity": 10000,
      "IsOptional": false,
      "Prerequisites": []
    },
    {
      "Type": "Collect",
      "Description": "Gather 5,000 Titanium",
      "Target": "Titanium",
      "RequiredQuantity": 5000,
      "IsOptional": false,
      "Prerequisites": []
    },
    {
      "Type": "Build",
      "Description": "Build a shipyard station",
      "Target": "shipyard",
      "RequiredQuantity": 1,
      "IsOptional": false,
      "Prerequisites": ["obj_iron", "obj_titanium"]
    }
  ],
  "Rewards": [
    {
      "Type": "Unlock",
      "RewardId": "blueprint_capital_ship",
      "Amount": 1,
      "Description": "Capital Ship Blueprint"
    }
  ]
}
```

---

## Usage

### Setting Up Quest System

```csharp
// Initialize quest system
var questSystem = new QuestSystem(entityManager, eventSystem);

// Load quest templates from directory
var quests = QuestLoader.LoadQuestsFromDirectory("GameData/Quests");
foreach (var quest in quests)
{
    questSystem.AddQuestTemplate(quest);
}

// Add to game engine
gameEngine.RegisterSystem(questSystem);
```

### Giving Quests to Players

```csharp
// Add quest component to player entity
var questComponent = new QuestComponent
{
    EntityId = playerEntityId,
    MaxActiveQuests = 10
};
entityManager.AddComponent(playerEntityId, questComponent);

// Give a quest to player
questSystem.GiveQuest(playerEntityId, "quest_tutorial_mining");
```

### Accepting Quests

```csharp
var questComponent = entityManager.GetComponent<QuestComponent>(playerEntityId);

// Check if can accept more quests
if (questComponent.CanAcceptMoreQuests)
{
    // Accept a quest
    questComponent.AcceptQuest("quest_tutorial_mining");
}
```

### Manual Progress Updates

While most progress is tracked automatically through events, you can manually update progress:

```csharp
// Progress an objective
questSystem.ProgressObjective(
    playerEntityId, 
    ObjectiveType.Visit, 
    "sector_0_0_0", 
    1
);
```

### Checking Quest Status

```csharp
var questComponent = entityManager.GetComponent<QuestComponent>(playerEntityId);

// Get all active quests
foreach (var quest in questComponent.ActiveQuests)
{
    Console.WriteLine($"{quest.Title}: {quest.CompletionPercentage}% complete");
    
    foreach (var objective in quest.Objectives)
    {
        Console.WriteLine($"  - {objective.Description}: {objective.CurrentProgress}/{objective.RequiredQuantity}");
    }
}

// Get completed quests ready to turn in
foreach (var quest in questComponent.CompletedQuests)
{
    questComponent.TurnInQuest(quest.Id);
}
```

### Quest Events

The system publishes several events:

- **QuestOffered** - Quest becomes available to player
- **QuestAccepted** - Player accepts a quest
- **QuestObjectiveCompleted** - Individual objective completed
- **QuestCompleted** - All required objectives complete
- **QuestFailed** - Quest failed (time limit or failed objective)
- **QuestTurnedIn** - Quest turned in and rewards given
- **QuestRewardGiven** - Individual reward distributed

Subscribe to these events to update UI or trigger other game logic:

```csharp
eventSystem.Subscribe("QuestCompleted", (eventData) =>
{
    if (eventData is QuestEvent questEvent)
    {
        Console.WriteLine($"Completed: {questEvent.QuestTitle}!");
        // Show UI notification, play sound, etc.
    }
});
```

---

## Automatic Progress Tracking

The Quest System automatically tracks progress by subscribing to game events:

### Currently Tracked Events
- **EntityDestroyed** → Progress "Destroy" objectives
- **ResourceCollected** → Progress "Collect" objectives
- **ResourceMined** → Progress "Mine" objectives

### Adding More Event Handlers

To track additional objective types, add event subscriptions in QuestSystem constructor:

```csharp
// In QuestSystem constructor
_eventSystem.Subscribe("PlayerEnteredSector", OnSectorEntered);
_eventSystem.Subscribe("TradeCompleted", OnTradeCompleted);

// Add handler methods
private void OnSectorEntered(GameEvent eventData)
{
    if (eventData is SectorEvent sectorEvent)
    {
        ProgressObjective(sectorEvent.EntityId, ObjectiveType.Visit, 
            sectorEvent.SectorCoordinates, 1);
    }
}
```

---

## Reward Distribution

When a quest is turned in, rewards are distributed automatically. The system publishes `QuestRewardGiven` events that other systems can handle:

```csharp
// In your inventory/progression system
eventSystem.Subscribe("QuestRewardGiven", (eventData) =>
{
    if (eventData is QuestRewardEvent rewardEvent)
    {
        switch (rewardEvent.RewardType)
        {
            case RewardType.Credits:
                // Add credits to player
                AddCredits(rewardEvent.EntityId, rewardEvent.Amount);
                break;
                
            case RewardType.Experience:
                // Add XP to player
                AddExperience(rewardEvent.EntityId, rewardEvent.Amount);
                break;
                
            case RewardType.Resource:
                // Add resource to inventory
                AddResource(rewardEvent.EntityId, rewardEvent.RewardId, rewardEvent.Amount);
                break;
                
            case RewardType.Reputation:
                // Increase faction reputation
                AddReputation(rewardEvent.RewardId, rewardEvent.Amount);
                break;
                
            case RewardType.Item:
                // Give item to player
                GiveItem(rewardEvent.EntityId, rewardEvent.RewardId);
                break;
                
            case RewardType.Unlock:
                // Unlock feature/blueprint
                UnlockFeature(rewardEvent.EntityId, rewardEvent.RewardId);
                break;
        }
    }
});
```

---

## Quest UI Integration

The Quest System is designed to integrate with ImGui or any UI framework:

```csharp
// Example ImGui quest log
void RenderQuestLog()
{
    var questComponent = entityManager.GetComponent<QuestComponent>(playerEntityId);
    
    ImGui.Begin("Quest Log");
    
    if (ImGui.CollapsingHeader("Active Quests"))
    {
        foreach (var quest in questComponent.ActiveQuests)
        {
            ImGui.PushID(quest.Id);
            ImGui.Text(quest.Title);
            ImGui.ProgressBar(quest.CompletionPercentage / 100f);
            
            if (ImGui.TreeNode("Objectives"))
            {
                foreach (var objective in quest.Objectives)
                {
                    string status = objective.IsComplete ? "[✓]" : "[ ]";
                    ImGui.Text($"{status} {objective.Description} ({objective.CurrentProgress}/{objective.RequiredQuantity})");
                }
                ImGui.TreePop();
            }
            
            if (quest.CanAbandon && ImGui.Button("Abandon"))
            {
                questComponent.AbandonQuest(quest.Id);
            }
            
            ImGui.PopID();
        }
    }
    
    if (ImGui.CollapsingHeader("Available Quests"))
    {
        foreach (var quest in questComponent.AvailableQuests)
        {
            ImGui.PushID(quest.Id);
            ImGui.Text(quest.Title);
            ImGui.TextWrapped(quest.Description);
            
            if (questComponent.CanAcceptMoreQuests && ImGui.Button("Accept"))
            {
                questComponent.AcceptQuest(quest.Id);
            }
            
            ImGui.PopID();
        }
    }
    
    ImGui.End();
}
```

---

## Best Practices

### Quest Design
1. **Clear Objectives** - Make it obvious what the player needs to do
2. **Appropriate Rewards** - Match rewards to difficulty and time investment
3. **Varied Objectives** - Mix different objective types to keep gameplay interesting
4. **Progressive Difficulty** - Use quest chains to gradually increase challenge
5. **Optional Objectives** - Include optional objectives for bonus rewards

### Technical
1. **Use Quest Templates** - Define quests in JSON, load them as templates
2. **Event-Driven Progress** - Let the quest system track progress automatically
3. **Test Quest Chains** - Verify prerequisites work correctly
4. **Balance Time Limits** - Ensure time limits are generous but challenging
5. **Validate Quest Data** - Check that targets and rewards are valid

### Performance
1. **Limit Active Quests** - Set reasonable MaxActiveQuests (10-20)
2. **Clean Up Completed** - Remove or archive old completed quests
3. **Batch Updates** - Quest system updates once per frame for all quests
4. **Efficient Event Handlers** - Keep event handlers fast and simple

---

## Troubleshooting

### Quest Not Progressing
- Verify the objective Target matches the event data
- Check that quest is in Active status
- Ensure objective Status is Active
- Verify event system is publishing the right events

### Quest Fails Immediately
- Check TimeLimit isn't too short
- Verify no required objectives have failed
- Check prerequisites are met

### Rewards Not Given
- Ensure quest status is TurnedIn
- Check that reward handlers are subscribed to QuestRewardGiven event
- Verify reward types and IDs are valid

---

## Future Enhancements

### Planned Features
- [ ] Quest priority/importance levels
- [ ] Quest markers and waypoints
- [ ] Dialog system integration
- [ ] Dynamic quest generation
- [ ] Quest journal with history
- [ ] Achievement integration
- [ ] Co-op/shared quests
- [ ] Quest difficulty scaling
- [ ] Branching quest paths
- [ ] Hidden quests and secrets

### Potential Expansions
- Quest giver NPCs and dialog trees
- Faction-specific quest lines
- Story campaigns
- Daily/weekly quest rotation
- Special event quests
- Player-created quests (modding)

---

## API Reference

### QuestSystem Methods

```csharp
// Template management
void AddQuestTemplate(Quest quest)
Quest? CreateQuestFromTemplate(string templateId)
IReadOnlyDictionary<string, Quest> GetQuestTemplates()

// Quest assignment
bool GiveQuest(Guid entityId, string questTemplateId)

// Progress tracking
void ProgressObjective(Guid entityId, ObjectiveType type, string target, int amount = 1)

// System lifecycle
void Update(float deltaTime)
void Initialize()
void Shutdown()
```

### QuestComponent Methods

```csharp
// Quest management
bool AddQuest(Quest quest)
bool RemoveQuest(string questId)
Quest? GetQuest(string questId)

// Quest actions
bool AcceptQuest(string questId)
bool AbandonQuest(string questId)
bool TurnInQuest(string questId)

// Queries
IEnumerable<Quest> ActiveQuests
IEnumerable<Quest> AvailableQuests
IEnumerable<Quest> CompletedQuests
IEnumerable<Quest> FailedQuests
IEnumerable<Quest> GetQuestsByTag(string tag)
bool CanAcceptMoreQuests
```

### Quest Methods

```csharp
// State management
bool Accept()
bool Complete()
void Fail()
bool TurnIn()
void Update()
void Reset()

// Properties
float CompletionPercentage
bool AreRequiredObjectivesComplete
bool HasFailedObjective
int TimeRemaining
bool IsExpired
```

### QuestObjective Methods

```csharp
// Progress
bool Progress(int amount = 1)
void Activate()
void Fail()
void Reset()

// Properties
float CompletionPercentage
bool IsComplete
bool IsFailed
```

---

## Examples

See `GameData/Quests/` for complete quest examples:
- `tutorial_mining.json` - Simple single-objective quest
- `combat_pirates.json` - Repeatable combat quest with time limit

---

## Integration Checklist

- [ ] Add QuestSystem to GameEngine
- [ ] Add QuestComponent to player entity
- [ ] Load quest templates on startup
- [ ] Implement quest UI (quest log, tracker, notifications)
- [ ] Subscribe to quest events for UI updates
- [ ] Implement reward distribution handlers
- [ ] Create tutorial and starter quests
- [ ] Test quest progression and completion
- [ ] Add quest markers/waypoints to world
- [ ] Balance rewards and difficulty

---

**Status:** Core implementation complete, ready for UI integration and content creation.  
**Next Steps:** Create quest UI, add more quest types, create quest content library.
