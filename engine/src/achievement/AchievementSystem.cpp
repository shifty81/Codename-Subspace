#include "achievement/AchievementSystem.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {

// ---------------------------------------------------------------------------
// AchievementCriterion
// ---------------------------------------------------------------------------

bool AchievementCriterion::IsComplete() const {
    return currentCount >= requiredCount;
}

float AchievementCriterion::GetProgress() const {
    if (requiredCount <= 0) return 1.0f;
    return std::min(static_cast<float>(currentCount) /
                    static_cast<float>(requiredCount), 1.0f);
}

// ---------------------------------------------------------------------------
// Achievement
// ---------------------------------------------------------------------------

bool Achievement::IsComplete() const {
    for (const auto& c : criteria)
        if (!c.IsComplete()) return false;
    return !criteria.empty();
}

float Achievement::GetProgress() const {
    if (criteria.empty()) return 0.0f;
    float sum = 0.0f;
    for (const auto& c : criteria) sum += c.GetProgress();
    return sum / static_cast<float>(criteria.size());
}

// ---------------------------------------------------------------------------
// AchievementComponent
// ---------------------------------------------------------------------------

void AchievementComponent::AddAchievement(const Achievement& achievement) {
    // Prevent duplicates
    for (const auto& a : achievements)
        if (a.id == achievement.id) return;
    achievements.push_back(achievement);
}

Achievement* AchievementComponent::GetAchievement(const std::string& id) {
    for (auto& a : achievements)
        if (a.id == id) return &a;
    return nullptr;
}

const Achievement* AchievementComponent::GetAchievement(const std::string& id) const {
    for (const auto& a : achievements)
        if (a.id == id) return &a;
    return nullptr;
}

bool AchievementComponent::IsUnlocked(const std::string& id) const {
    const auto* a = GetAchievement(id);
    return a && a->unlocked;
}

int AchievementComponent::GetUnlockedCount() const {
    int count = 0;
    for (const auto& a : achievements)
        if (a.unlocked) ++count;
    return count;
}

int AchievementComponent::GetTotalCount() const {
    return static_cast<int>(achievements.size());
}

float AchievementComponent::GetOverallProgress() const {
    if (achievements.empty()) return 0.0f;
    float sum = 0.0f;
    for (const auto& a : achievements)
        sum += a.unlocked ? 1.0f : a.GetProgress();
    return sum / static_cast<float>(achievements.size());
}

std::vector<Achievement*> AchievementComponent::GetByCategory(AchievementCategory cat) {
    std::vector<Achievement*> result;
    for (auto& a : achievements)
        if (a.category == cat) result.push_back(&a);
    return result;
}

bool AchievementComponent::RecordEvent(const std::string& achievementId,
                                       const std::string& eventType, int amount) {
    auto* a = GetAchievement(achievementId);
    if (!a || a->unlocked) return false;

    for (auto& c : a->criteria) {
        if (c.eventType == eventType) {
            c.currentCount += amount;
        }
    }

    if (a->IsComplete()) {
        a->unlocked = true;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData AchievementComponent::Serialize() const {
    ComponentData data;
    data.componentType = "AchievementComponent";

    data.data["count"] = std::to_string(achievements.size());

    for (size_t i = 0; i < achievements.size(); ++i) {
        const auto& a = achievements[i];
        std::string prefix = "a" + std::to_string(i) + ".";

        data.data[prefix + "id"]          = a.id;
        data.data[prefix + "name"]        = a.name;
        data.data[prefix + "description"] = a.description;
        data.data[prefix + "category"]    = std::to_string(static_cast<int>(a.category));
        data.data[prefix + "rewardXP"]    = std::to_string(a.rewardXP);
        data.data[prefix + "rewardCredits"] = std::to_string(a.rewardCredits);
        data.data[prefix + "unlocked"]    = a.unlocked ? "1" : "0";
        data.data[prefix + "unlockTs"]    = std::to_string(a.unlockTimestamp);

        data.data[prefix + "criteriaCount"] = std::to_string(a.criteria.size());
        for (size_t j = 0; j < a.criteria.size(); ++j) {
            std::string cp = prefix + "c" + std::to_string(j) + ".";
            data.data[cp + "event"]    = a.criteria[j].eventType;
            data.data[cp + "required"] = std::to_string(a.criteria[j].requiredCount);
            data.data[cp + "current"]  = std::to_string(a.criteria[j].currentCount);
        }
    }

    return data;
}

void AchievementComponent::Deserialize(const ComponentData& data) {
    achievements.clear();

    auto it = data.data.find("count");
    if (it == data.data.end()) return;

    int count = std::stoi(it->second);
    for (int i = 0; i < count; ++i) {
        std::string prefix = "a" + std::to_string(i) + ".";

        Achievement a;
        auto getVal = [&](const std::string& key) -> std::string {
            auto found = data.data.find(prefix + key);
            return found != data.data.end() ? found->second : "";
        };

        a.id          = getVal("id");
        a.name        = getVal("name");
        a.description = getVal("description");
        a.category    = static_cast<AchievementCategory>(std::stoi(getVal("category").empty() ? "0" : getVal("category")));
        a.rewardXP    = std::stoi(getVal("rewardXP").empty() ? "0" : getVal("rewardXP"));
        a.rewardCredits = std::stoi(getVal("rewardCredits").empty() ? "0" : getVal("rewardCredits"));
        a.unlocked    = getVal("unlocked") == "1";
        a.unlockTimestamp = std::stod(getVal("unlockTs").empty() ? "0" : getVal("unlockTs"));

        int criteriaCount = std::stoi(getVal("criteriaCount").empty() ? "0" : getVal("criteriaCount"));
        for (int j = 0; j < criteriaCount; ++j) {
            std::string cp = "c" + std::to_string(j) + ".";
            AchievementCriterion c;
            c.eventType     = getVal(cp + "event");
            c.requiredCount = std::stoi(getVal(cp + "required").empty() ? "0" : getVal(cp + "required"));
            c.currentCount  = std::stoi(getVal(cp + "current").empty() ? "0" : getVal(cp + "current"));
            a.criteria.push_back(c);
        }

        achievements.push_back(a);
    }
}

// ---------------------------------------------------------------------------
// AchievementSystem
// ---------------------------------------------------------------------------

AchievementSystem::AchievementSystem() : SystemBase("AchievementSystem") {}

void AchievementSystem::Initialize() {}
void AchievementSystem::Update(float /*deltaTime*/) {}
void AchievementSystem::Shutdown() { _registry.clear(); }

void AchievementSystem::RegisterAchievement(const Achievement& achievement) {
    _registry[achievement.id] = achievement;
}

bool AchievementSystem::HasAchievement(const std::string& id) const {
    return _registry.find(id) != _registry.end();
}

const Achievement* AchievementSystem::GetAchievement(const std::string& id) const {
    auto it = _registry.find(id);
    return it != _registry.end() ? &it->second : nullptr;
}

size_t AchievementSystem::GetRegisteredCount() const {
    return _registry.size();
}

bool AchievementSystem::RecordProgress(const std::string& achievementId,
                                       const std::string& eventType, int amount) {
    auto it = _registry.find(achievementId);
    if (it == _registry.end() || it->second.unlocked) return false;

    for (auto& c : it->second.criteria) {
        if (c.eventType == eventType) {
            c.currentCount += amount;
        }
    }

    if (it->second.IsComplete()) {
        it->second.unlocked = true;
        return true;
    }
    return false;
}

std::vector<const Achievement*> AchievementSystem::GetAllAchievements() const {
    std::vector<const Achievement*> result;
    result.reserve(_registry.size());
    for (const auto& kv : _registry) result.push_back(&kv.second);
    return result;
}

std::vector<std::string> AchievementSystem::GetUnlockedIds() const {
    std::vector<std::string> result;
    for (const auto& kv : _registry)
        if (kv.second.unlocked) result.push_back(kv.first);
    return result;
}

// ---------------------------------------------------------------------------
// Template achievements
// ---------------------------------------------------------------------------

Achievement AchievementSystem::CreateFirstBlood() {
    Achievement a;
    a.id          = "first_blood";
    a.name        = "First Blood";
    a.description = "Destroy your first enemy ship";
    a.category    = AchievementCategory::Combat;
    a.rewardXP    = 50;
    a.rewardCredits = 100;
    a.criteria.push_back({"entity.destroyed", 1, 0});
    return a;
}

Achievement AchievementSystem::CreateExplorer() {
    Achievement a;
    a.id          = "explorer";
    a.name        = "Explorer";
    a.description = "Visit 10 different sectors";
    a.category    = AchievementCategory::Exploration;
    a.rewardXP    = 100;
    a.rewardCredits = 250;
    a.criteria.push_back({"sector.entered", 10, 0});
    return a;
}

Achievement AchievementSystem::CreateShipwright() {
    Achievement a;
    a.id          = "shipwright";
    a.name        = "Shipwright";
    a.description = "Build 5 ships";
    a.category    = AchievementCategory::Building;
    a.rewardXP    = 150;
    a.rewardCredits = 500;
    a.criteria.push_back({"ship.block.added", 50, 0});
    return a;
}

Achievement AchievementSystem::CreateTrader() {
    Achievement a;
    a.id          = "trader";
    a.name        = "Savvy Trader";
    a.description = "Complete 20 trades";
    a.category    = AchievementCategory::Trading;
    a.rewardXP    = 200;
    a.rewardCredits = 1000;
    a.criteria.push_back({"trade.completed", 20, 0});
    return a;
}

Achievement AchievementSystem::CreateVeteran() {
    Achievement a;
    a.id          = "veteran";
    a.name        = "Veteran Pilot";
    a.description = "Reach level 10";
    a.category    = AchievementCategory::Progression;
    a.rewardXP    = 500;
    a.rewardCredits = 2000;
    a.criteria.push_back({"player.levelup", 10, 0});
    return a;
}

Achievement AchievementSystem::CreateMiner() {
    Achievement a;
    a.id          = "miner";
    a.name        = "Asteroid Miner";
    a.description = "Mine 100 asteroids";
    a.category    = AchievementCategory::Exploration;
    a.rewardXP    = 100;
    a.rewardCredits = 300;
    a.criteria.push_back({"resource.collected", 100, 0});
    return a;
}

Achievement AchievementSystem::CreateFleetCommander() {
    Achievement a;
    a.id          = "fleet_commander";
    a.name        = "Fleet Commander";
    a.description = "Have 5 ships in your fleet";
    a.category    = AchievementCategory::Social;
    a.rewardXP    = 250;
    a.rewardCredits = 750;
    a.criteria.push_back({"entity.created", 5, 0});
    return a;
}

Achievement AchievementSystem::CreateRichPilot() {
    Achievement a;
    a.id          = "rich_pilot";
    a.name        = "Rich Pilot";
    a.description = "Earn a total of 10000 credits";
    a.category    = AchievementCategory::Trading;
    a.rewardXP    = 300;
    a.rewardCredits = 0;  // already rich!
    a.criteria.push_back({"trade.sold", 10000, 0});
    return a;
}

} // namespace subspace
