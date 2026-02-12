#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class QuestStatus { Available, Active, Completed, Failed, TurnedIn };
enum class QuestDifficulty { Trivial, Easy, Normal, Hard, Elite };
enum class RewardType { Credits, Resource, Experience, Reputation, Item, Unlock };
enum class ObjectiveType { Destroy, Collect, Mine, Visit, Trade, Build, Escort, Scan, Deliver, Talk };
enum class ObjectiveStatus { NotStarted, Active, Completed, Failed };

/// Describes a reward granted upon quest completion.
struct QuestReward {
    RewardType type = RewardType::Credits;
    std::string rewardId;
    int amount = 0;
    std::string description;
};

/// A single objective within a quest.
struct QuestObjective {
    std::string id;
    ObjectiveType type = ObjectiveType::Destroy;
    std::string description;
    std::string target;
    int requiredQuantity = 1;
    int currentProgress = 0;
    ObjectiveStatus status = ObjectiveStatus::NotStarted;
    bool isOptional = false;
    bool isHidden = false;

    /// Advance progress. Returns true when the objective becomes complete.
    bool Progress(int amount = 1);

    /// Mark this objective as active.
    void Activate();

    /// Mark this objective as failed.
    void Fail();

    /// Reset progress and status.
    void Reset();

    /// Completion percentage in [0, 1].
    float GetCompletionPercentage() const;

    /// Whether current progress meets the required quantity.
    bool IsComplete() const;
};

/// A quest containing objectives and rewards.
class Quest {
public:
    std::string id;
    std::string title;
    std::string description;
    QuestStatus status = QuestStatus::Available;
    QuestDifficulty difficulty = QuestDifficulty::Normal;
    std::vector<QuestObjective> objectives;
    std::vector<QuestReward> rewards;
    std::vector<std::string> prerequisites;
    bool canAbandon = true;
    bool isRepeatable = false;
    int timeLimit = 0; // seconds, 0 = no limit

    /// Accept the quest. Returns false if not Available.
    bool Accept();

    /// Complete the quest. Returns false if not Active or required objectives incomplete.
    bool Complete();

    /// Fail the quest.
    void Fail();

    /// Turn in a completed quest. Returns false if not Completed.
    bool TurnIn();

    /// Reset the quest to Available state.
    void Reset();

    /// Average completion of required (non-optional) objectives in [0, 1].
    float GetCompletionPercentage() const;

    /// Whether all required objectives are complete.
    bool AreRequiredObjectivesComplete() const;

    /// Whether any objective has failed.
    bool HasFailedObjective() const;
};

/// Component that tracks quests for an entity.
struct QuestComponent : public IComponent {
    std::vector<Quest> quests;
    int maxActiveQuests = 10;

    /// Add a quest to this component.
    void AddQuest(Quest quest);

    /// Remove a quest by id. Returns true if found and removed.
    bool RemoveQuest(const std::string& id);

    /// Find a quest by id. Returns nullptr if not found.
    Quest* GetQuest(const std::string& id);

    /// Accept a quest by id. Returns false if not found or cannot accept.
    bool AcceptQuest(const std::string& id);

    /// Abandon a quest by id. Returns false if not found or cannot abandon.
    bool AbandonQuest(const std::string& id);

    /// Turn in a quest by id. Returns false if not found or cannot turn in.
    bool TurnInQuest(const std::string& id);

    /// Number of quests with Active status.
    int GetActiveQuestCount() const;

    /// Number of quests with Available status.
    int GetAvailableQuestCount() const;

    /// Number of quests with Completed or TurnedIn status.
    int GetCompletedQuestCount() const;
};

/// System that manages quest templates and quest progression.
class QuestSystem : public SystemBase {
public:
    QuestSystem();

    void Update(float deltaTime) override;

    /// Register a quest template.
    void AddQuestTemplate(const Quest& quest);

    /// Create a new quest instance from a template.
    Quest CreateQuestFromTemplate(const std::string& templateId);

    /// Give a quest from a template to an entity. Returns false on failure.
    bool GiveQuest(EntityId entityId, const std::string& templateId,
                   QuestComponent& comp);

    /// Progress matching objectives in active quests.
    void ProgressObjective(QuestComponent& comp, ObjectiveType type,
                           const std::string& target, int amount = 1);

    /// Get all quest templates.
    const std::unordered_map<std::string, Quest>& GetQuestTemplates() const;

    /// Get the number of registered templates.
    size_t GetTemplateCount() const;

private:
    std::unordered_map<std::string, Quest> _questTemplates;
};

} // namespace subspace
