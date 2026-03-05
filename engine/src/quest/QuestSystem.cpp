#include "quest/QuestSystem.h"
#include "inventory/InventorySystem.h"
#include "rpg/ProgressionSystem.h"

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

// --- QuestComponent serialization helpers ---

static std::string QuestStatusToString(QuestStatus s) {
    switch (s) {
    case QuestStatus::Available: return "Available";
    case QuestStatus::Active:    return "Active";
    case QuestStatus::Completed: return "Completed";
    case QuestStatus::Failed:    return "Failed";
    case QuestStatus::TurnedIn:  return "TurnedIn";
    }
    return "Available";
}

static QuestStatus QuestStatusFromString(const std::string& s) {
    if (s == "Active")    return QuestStatus::Active;
    if (s == "Completed") return QuestStatus::Completed;
    if (s == "Failed")    return QuestStatus::Failed;
    if (s == "TurnedIn")  return QuestStatus::TurnedIn;
    return QuestStatus::Available;
}

static std::string ObjectiveStatusToString(ObjectiveStatus s) {
    switch (s) {
    case ObjectiveStatus::NotStarted: return "NotStarted";
    case ObjectiveStatus::Active:     return "Active";
    case ObjectiveStatus::Completed:  return "Completed";
    case ObjectiveStatus::Failed:     return "Failed";
    }
    return "NotStarted";
}

static ObjectiveStatus ObjectiveStatusFromString(const std::string& s) {
    if (s == "Active")    return ObjectiveStatus::Active;
    if (s == "Completed") return ObjectiveStatus::Completed;
    if (s == "Failed")    return ObjectiveStatus::Failed;
    return ObjectiveStatus::NotStarted;
}

static std::string ObjectiveTypeToString(ObjectiveType t) {
    switch (t) {
    case ObjectiveType::Destroy: return "Destroy";
    case ObjectiveType::Collect: return "Collect";
    case ObjectiveType::Mine:    return "Mine";
    case ObjectiveType::Visit:   return "Visit";
    case ObjectiveType::Trade:   return "Trade";
    case ObjectiveType::Build:   return "Build";
    case ObjectiveType::Escort:  return "Escort";
    case ObjectiveType::Scan:    return "Scan";
    case ObjectiveType::Deliver: return "Deliver";
    case ObjectiveType::Talk:    return "Talk";
    }
    return "Destroy";
}

static ObjectiveType ObjectiveTypeFromString(const std::string& s) {
    if (s == "Collect") return ObjectiveType::Collect;
    if (s == "Mine")    return ObjectiveType::Mine;
    if (s == "Visit")   return ObjectiveType::Visit;
    if (s == "Trade")   return ObjectiveType::Trade;
    if (s == "Build")   return ObjectiveType::Build;
    if (s == "Escort")  return ObjectiveType::Escort;
    if (s == "Scan")    return ObjectiveType::Scan;
    if (s == "Deliver") return ObjectiveType::Deliver;
    if (s == "Talk")    return ObjectiveType::Talk;
    return ObjectiveType::Destroy;
}

ComponentData QuestComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "QuestComponent";
    cd.data["questCount"] = std::to_string(quests.size());
    cd.data["maxActiveQuests"] = std::to_string(maxActiveQuests);

    for (size_t i = 0; i < quests.size(); ++i) {
        const auto& q = quests[i];
        std::string prefix = "quest_" + std::to_string(i) + "_";
        cd.data[prefix + "id"] = q.id;
        cd.data[prefix + "title"] = q.title;
        cd.data[prefix + "status"] = QuestStatusToString(q.status);
        cd.data[prefix + "canAbandon"] = q.canAbandon ? "true" : "false";
        cd.data[prefix + "isRepeatable"] = q.isRepeatable ? "true" : "false";
        cd.data[prefix + "timeLimit"] = std::to_string(q.timeLimit);

        cd.data[prefix + "objectiveCount"] = std::to_string(q.objectives.size());
        for (size_t j = 0; j < q.objectives.size(); ++j) {
            const auto& obj = q.objectives[j];
            std::string oprefix = prefix + "obj_" + std::to_string(j) + "_";
            cd.data[oprefix + "id"] = obj.id;
            cd.data[oprefix + "type"] = ObjectiveTypeToString(obj.type);
            cd.data[oprefix + "target"] = obj.target;
            cd.data[oprefix + "required"] = std::to_string(obj.requiredQuantity);
            cd.data[oprefix + "progress"] = std::to_string(obj.currentProgress);
            cd.data[oprefix + "status"] = ObjectiveStatusToString(obj.status);
            cd.data[oprefix + "optional"] = obj.isOptional ? "true" : "false";
            cd.data[oprefix + "hidden"] = obj.isHidden ? "true" : "false";
        }
    }
    return cd;
}

void QuestComponent::Deserialize(const ComponentData& data) {
    quests.clear();

    auto getStr = [&](const std::string& key) -> std::string {
        auto it = data.data.find(key);
        return it != data.data.end() ? it->second : "";
    };
    auto getInt = [&](const std::string& key, int def = 0) -> int {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };

    maxActiveQuests = getInt("maxActiveQuests", 10);
    int questCount = getInt("questCount", 0);

    for (int i = 0; i < questCount; ++i) {
        std::string prefix = "quest_" + std::to_string(i) + "_";
        Quest q;
        q.id = getStr(prefix + "id");
        q.title = getStr(prefix + "title");
        q.status = QuestStatusFromString(getStr(prefix + "status"));
        q.canAbandon = getStr(prefix + "canAbandon") != "false";
        q.isRepeatable = getStr(prefix + "isRepeatable") == "true";
        q.timeLimit = getInt(prefix + "timeLimit", 0);

        int objCount = getInt(prefix + "objectiveCount", 0);
        for (int j = 0; j < objCount; ++j) {
            std::string oprefix = prefix + "obj_" + std::to_string(j) + "_";
            QuestObjective obj;
            obj.id = getStr(oprefix + "id");
            obj.type = ObjectiveTypeFromString(getStr(oprefix + "type"));
            obj.target = getStr(oprefix + "target");
            obj.requiredQuantity = getInt(oprefix + "required", 1);
            obj.currentProgress = getInt(oprefix + "progress", 0);
            obj.status = ObjectiveStatusFromString(getStr(oprefix + "status"));
            obj.isOptional = getStr(oprefix + "optional") == "true";
            obj.isHidden = getStr(oprefix + "hidden") == "true";
            q.objectives.push_back(std::move(obj));
        }

        quests.push_back(std::move(q));
    }
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

void QuestSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

int QuestSystem::DistributeRewards(EntityId entityId,
                                    const std::vector<QuestReward>& rewards) {
    if (!_entityManager) return 0;

    int distributed = 0;

    for (const auto& reward : rewards) {
        switch (reward.type) {
        case RewardType::Credits: {
            auto* inv = _entityManager->GetComponent<InventoryComponent>(entityId);
            if (inv) {
                InventoryItem creditItem;
                creditItem.itemId = "credits";
                creditItem.name = "Credits";
                creditItem.description = "Currency";
                creditItem.category = "currency";
                creditItem.weight = 0.0f;
                creditItem.stackSize = reward.amount;
                creditItem.maxStackSize = 999999;
                creditItem.value = reward.amount;
                if (inv->AddItem(creditItem)) ++distributed;
            }
            break;
        }
        case RewardType::Resource: {
            auto* inv = _entityManager->GetComponent<InventoryComponent>(entityId);
            if (inv) {
                InventoryItem resourceItem;
                resourceItem.itemId = reward.rewardId;
                resourceItem.name = reward.rewardId;
                resourceItem.description = reward.description;
                resourceItem.category = "resource";
                resourceItem.weight = 1.0f;
                resourceItem.stackSize = reward.amount;
                resourceItem.maxStackSize = 9999;
                resourceItem.value = reward.amount;
                if (inv->AddItem(resourceItem)) ++distributed;
            }
            break;
        }
        case RewardType::Experience: {
            auto* prog = _entityManager->GetComponent<ProgressionComponent>(entityId);
            if (prog) {
                prog->AddExperience(reward.amount);
                ++distributed;
            }
            break;
        }
        case RewardType::Reputation: {
            auto* faction = _entityManager->GetComponent<FactionComponent>(entityId);
            if (faction) {
                // When rewardId is empty, reputation is applied to the
                // entity's own faction; otherwise use the named faction.
                std::string targetFaction = reward.rewardId.empty()
                    ? faction->factionName : reward.rewardId;
                faction->ModifyReputation(targetFaction, reward.amount);
                ++distributed;
            }
            break;
        }
        case RewardType::Item: {
            auto* inv = _entityManager->GetComponent<InventoryComponent>(entityId);
            if (inv) {
                InventoryItem item;
                item.itemId = reward.rewardId;
                item.name = reward.rewardId;
                item.description = reward.description;
                item.category = "item";
                item.weight = 1.0f;
                item.stackSize = reward.amount;
                item.maxStackSize = 99;
                item.value = reward.amount * 10;
                if (inv->AddItem(item)) ++distributed;
            }
            break;
        }
        case RewardType::Unlock:
            // Unlock rewards are not yet distributed to a specific system;
            // they are tracked in the quest but do not count as distributed.
            break;
        }
    }

    return distributed;
}

// ---------------------------------------------------------------------------
// QuestGenerator
// ---------------------------------------------------------------------------

void QuestGenerator::SetSeed(unsigned int seed) {
    _seed = (seed == 0) ? kDefaultSeed : seed;
}

unsigned int QuestGenerator::NextRandom() {
    // Simple LCG (same family as glibc's)
    _seed = _seed * 1103515245u + 12345u;
    return (_seed >> 16) & 0x7FFF;
}

int QuestGenerator::RandomRange(int lo, int hi) {
    if (lo >= hi) return lo;
    return lo + static_cast<int>(NextRandom() % static_cast<unsigned int>(hi - lo + 1));
}

float QuestGenerator::RandomFloat() {
    return static_cast<float>(NextRandom()) / 32767.0f;
}

// Tables used by the generator ------------------------------------------------

static const char* const kQuestTitlePrefixes[] = {
    "Eliminate", "Collect", "Mine", "Deliver", "Explore",
    "Scan", "Escort", "Build", "Trade", "Investigate"
};
static constexpr int kPrefixCount = 10;

static const char* const kQuestTitleTargets[] = {
    "Pirate Raiders", "Rare Minerals", "Ancient Artifacts",
    "Cargo Shipment", "Unknown Sector", "Signal Source",
    "Merchant Convoy", "Defense Platform", "Trade Goods", "Anomaly"
};
static constexpr int kTargetCount = 10;

static const ObjectiveType kObjectivePool[] = {
    ObjectiveType::Destroy, ObjectiveType::Collect, ObjectiveType::Mine,
    ObjectiveType::Visit,   ObjectiveType::Trade,   ObjectiveType::Build,
    ObjectiveType::Escort,  ObjectiveType::Scan,    ObjectiveType::Deliver,
    ObjectiveType::Talk
};
static constexpr int kObjectivePoolSize = 10;

static const char* const kObjectiveTargets[] = {
    "pirate_ship", "iron_ore", "titanium_ore", "naonite_crystal",
    "trinium_chunk", "cargo_crate", "station_alpha", "sector_7",
    "merchant_vessel", "signal_beacon"
};
static constexpr int kObjTargetCount = 10;

Quest QuestGenerator::Generate(int playerLevel, int sectorSecurityLevel) {
    Quest q;

    // Unique id
    q.id = "gen_quest_" + std::to_string(++_generatedCount);

    // Title
    int prefixIdx = RandomRange(0, kPrefixCount - 1);
    int targetIdx = RandomRange(0, kTargetCount - 1);
    q.title = std::string(kQuestTitlePrefixes[prefixIdx]) + " " +
              kQuestTitleTargets[targetIdx];

    q.description = "Dynamically generated quest for level " +
                    std::to_string(playerLevel) + " in security " +
                    std::to_string(sectorSecurityLevel) + " space.";

    // Difficulty scales with player level and inversely with security
    int diffIdx = std::min(playerLevel / kLevelsPerDifficultyTier, kMaxDifficultyIndex);
    if (sectorSecurityLevel < kLowSecurityThreshold)
        diffIdx = std::min(diffIdx + 1, kMaxDifficultyIndex);
    q.difficulty = static_cast<QuestDifficulty>(diffIdx);

    q.canAbandon  = true;
    q.isRepeatable = false;

    // 1-3 objectives based on difficulty
    int objCount = 1 + static_cast<int>(q.difficulty);
    if (objCount > kMaxObjectivesPerQuest) objCount = kMaxObjectivesPerQuest;

    for (int i = 0; i < objCount; ++i) {
        QuestObjective obj;
        obj.id = q.id + "_obj_" + std::to_string(i);

        int typeIdx = RandomRange(0, kObjectivePoolSize - 1);
        obj.type = kObjectivePool[typeIdx];

        int tgtIdx = RandomRange(0, kObjTargetCount - 1);
        obj.target = kObjectiveTargets[tgtIdx];

        // Quantity scales with level
        int baseQty = 1 + playerLevel / 3;
        obj.requiredQuantity = RandomRange(baseQty, baseQty + 3);
        obj.description = "Objective " + std::to_string(i + 1) + " of " +
                          q.title;

        // Last objective may be optional on higher difficulties
        if (i == objCount - 1 && static_cast<int>(q.difficulty) >= 2) {
            obj.isOptional = RandomFloat() > 0.5f;
        }

        q.objectives.push_back(std::move(obj));
    }

    // Rewards scale with difficulty and level
    int rewardMultiplier = 1 + static_cast<int>(q.difficulty);
    {
        QuestReward r;
        r.type = RewardType::Credits;
        r.amount = rewardMultiplier * (100 + playerLevel * 25);
        r.description = "Credit reward";
        q.rewards.push_back(r);
    }
    {
        QuestReward r;
        r.type = RewardType::Experience;
        r.amount = rewardMultiplier * (50 + playerLevel * 10);
        r.description = "Experience reward";
        q.rewards.push_back(r);
    }

    // Higher-difficulty quests may also grant reputation
    if (static_cast<int>(q.difficulty) >= 2) {
        QuestReward r;
        r.type = RewardType::Reputation;
        r.amount = rewardMultiplier * 5;
        r.description = "Reputation reward";
        q.rewards.push_back(r);
    }

    return q;
}

std::vector<Quest> QuestGenerator::GenerateBatch(int count, int playerLevel,
                                                  int sectorSecurityLevel) {
    std::vector<Quest> out;
    out.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        out.push_back(Generate(playerLevel, sectorSecurityLevel));
    }
    return out;
}

int QuestGenerator::GetGeneratedCount() const {
    return _generatedCount;
}

} // namespace subspace
