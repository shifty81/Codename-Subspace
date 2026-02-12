#include "quest/QuestSystem.h"

namespace subspace {

// --- QuestObjective ---

bool QuestObjective::Progress(int amount) {
    if (status == ObjectiveStatus::Failed || status == ObjectiveStatus::Completed)
        return false;

    if (status == ObjectiveStatus::NotStarted)
        status = ObjectiveStatus::Active;

    currentProgress = std::min(currentProgress + amount, requiredQuantity);

    if (IsComplete()) {
        status = ObjectiveStatus::Completed;
        return true;
    }
    return false;
}

void QuestObjective::Activate() {
    if (status == ObjectiveStatus::NotStarted)
        status = ObjectiveStatus::Active;
}

void QuestObjective::Fail() {
    status = ObjectiveStatus::Failed;
}

void QuestObjective::Reset() {
    currentProgress = 0;
    status = ObjectiveStatus::NotStarted;
}

float QuestObjective::GetCompletionPercentage() const {
    if (requiredQuantity <= 0) return 1.0f;
    return std::min(static_cast<float>(currentProgress) / static_cast<float>(requiredQuantity), 1.0f);
}

bool QuestObjective::IsComplete() const {
    return currentProgress >= requiredQuantity;
}

// --- Quest ---

bool Quest::Accept() {
    if (status != QuestStatus::Available) return false;

    status = QuestStatus::Active;

    // Activate the first non-complete objective.
    for (auto& obj : objectives) {
        if (obj.status != ObjectiveStatus::Completed) {
            obj.Activate();
            break;
        }
    }
    return true;
}

bool Quest::Complete() {
    if (status != QuestStatus::Active) return false;
    if (!AreRequiredObjectivesComplete()) return false;

    status = QuestStatus::Completed;
    return true;
}

void Quest::Fail() {
    status = QuestStatus::Failed;
}

bool Quest::TurnIn() {
    if (status != QuestStatus::Completed) return false;

    status = QuestStatus::TurnedIn;
    return true;
}

void Quest::Reset() {
    status = QuestStatus::Available;
    for (auto& obj : objectives) {
        obj.Reset();
    }
}

float Quest::GetCompletionPercentage() const {
    int count = 0;
    float total = 0.0f;
    for (const auto& obj : objectives) {
        if (obj.isOptional) continue;
        total += obj.GetCompletionPercentage();
        ++count;
    }
    if (count == 0) return 1.0f;
    return total / static_cast<float>(count);
}

bool Quest::AreRequiredObjectivesComplete() const {
    for (const auto& obj : objectives) {
        if (obj.isOptional) continue;
        if (!obj.IsComplete()) return false;
    }
    return true;
}

bool Quest::HasFailedObjective() const {
    for (const auto& obj : objectives) {
        if (obj.status == ObjectiveStatus::Failed) return true;
    }
    return false;
}

// --- QuestComponent ---

void QuestComponent::AddQuest(Quest quest) {
    quests.push_back(std::move(quest));
}

bool QuestComponent::RemoveQuest(const std::string& id) {
    auto it = std::find_if(quests.begin(), quests.end(),
                           [&id](const Quest& q) { return q.id == id; });
    if (it == quests.end()) return false;
    quests.erase(it);
    return true;
}

Quest* QuestComponent::GetQuest(const std::string& id) {
    for (auto& q : quests) {
        if (q.id == id) return &q;
    }
    return nullptr;
}

bool QuestComponent::AcceptQuest(const std::string& id) {
    if (GetActiveQuestCount() >= maxActiveQuests) return false;

    Quest* q = GetQuest(id);
    if (!q) return false;
    return q->Accept();
}

bool QuestComponent::AbandonQuest(const std::string& id) {
    Quest* q = GetQuest(id);
    if (!q) return false;
    if (q->status != QuestStatus::Active) return false;
    if (!q->canAbandon) return false;

    q->Reset();
    return true;
}

bool QuestComponent::TurnInQuest(const std::string& id) {
    Quest* q = GetQuest(id);
    if (!q) return false;
    return q->TurnIn();
}

int QuestComponent::GetActiveQuestCount() const {
    int count = 0;
    for (const auto& q : quests) {
        if (q.status == QuestStatus::Active) ++count;
    }
    return count;
}

int QuestComponent::GetAvailableQuestCount() const {
    int count = 0;
    for (const auto& q : quests) {
        if (q.status == QuestStatus::Available) ++count;
    }
    return count;
}

int QuestComponent::GetCompletedQuestCount() const {
    int count = 0;
    for (const auto& q : quests) {
        if (q.status == QuestStatus::Completed || q.status == QuestStatus::TurnedIn)
            ++count;
    }
    return count;
}

// --- QuestSystem ---

QuestSystem::QuestSystem() : SystemBase("QuestSystem") {}

void QuestSystem::Update(float /*deltaTime*/) {
    // Standalone: callers drive quest progression via ProgressObjective.
}

void QuestSystem::AddQuestTemplate(const Quest& quest) {
    _questTemplates[quest.id] = quest;
}

Quest QuestSystem::CreateQuestFromTemplate(const std::string& templateId) {
    auto it = _questTemplates.find(templateId);
    if (it == _questTemplates.end()) return Quest{};
    return it->second;
}

// entityId reserved for future entity-lookup integration.
bool QuestSystem::GiveQuest(EntityId /*entityId*/, const std::string& templateId,
                            QuestComponent& comp) {
    auto it = _questTemplates.find(templateId);
    if (it == _questTemplates.end()) return false;

    Quest quest = it->second;
    comp.AddQuest(std::move(quest));
    return true;
}

void QuestSystem::ProgressObjective(QuestComponent& comp, ObjectiveType type,
                                    const std::string& target, int amount) {
    for (auto& quest : comp.quests) {
        if (quest.status != QuestStatus::Active) continue;

        for (auto& obj : quest.objectives) {
            if (obj.status == ObjectiveStatus::Completed ||
                obj.status == ObjectiveStatus::Failed)
                continue;

            if (obj.type == type && obj.target == target) {
                obj.Progress(amount);
            }
        }

        // Auto-complete quest if all required objectives are done.
        if (quest.AreRequiredObjectivesComplete()) {
            quest.Complete();
        }
    }
}

const std::unordered_map<std::string, Quest>& QuestSystem::GetQuestTemplates() const {
    return _questTemplates;
}

size_t QuestSystem::GetTemplateCount() const {
    return _questTemplates.size();
}

} // namespace subspace
