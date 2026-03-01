#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/persistence/SaveGameManager.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// Achievement types and criteria
// ---------------------------------------------------------------------------

enum class AchievementCategory { Combat, Exploration, Building, Trading, Progression, Social };

struct AchievementCriterion {
    std::string eventType;     // GameEvents constant to listen for
    int requiredCount = 1;
    int currentCount  = 0;

    bool IsComplete() const;
    float GetProgress() const; // [0, 1]
};

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    AchievementCategory category = AchievementCategory::Progression;

    int rewardXP      = 0;
    int rewardCredits = 0;

    std::vector<AchievementCriterion> criteria;

    bool   unlocked        = false;
    double unlockTimestamp  = 0.0;   // seconds since epoch (0 = not unlocked)

    /// True when every criterion is complete.
    bool IsComplete() const;

    /// Overall progress [0, 1] averaged across criteria.
    float GetProgress() const;
};

// ---------------------------------------------------------------------------
// Achievement component — per-entity (typically the player)
// ---------------------------------------------------------------------------

struct AchievementComponent : public IComponent {
    std::vector<Achievement> achievements;

    void AddAchievement(const Achievement& achievement);
    Achievement* GetAchievement(const std::string& id);
    const Achievement* GetAchievement(const std::string& id) const;
    bool IsUnlocked(const std::string& id) const;
    int  GetUnlockedCount() const;
    int  GetTotalCount() const;
    float GetOverallProgress() const;

    std::vector<Achievement*> GetByCategory(AchievementCategory cat);

    /// Increment the matching criterion for a given event type.
    /// Returns true if the achievement became fully complete as a result.
    bool RecordEvent(const std::string& achievementId,
                     const std::string& eventType, int amount = 1);

    /// Persistence.
    ComponentData Serialize() const;
    void Deserialize(const ComponentData& data);
};

// ---------------------------------------------------------------------------
// Achievement system — manages the achievement registry and checks progress
// ---------------------------------------------------------------------------

class AchievementSystem : public SystemBase {
public:
    AchievementSystem();

    void Initialize() override;
    void Update(float deltaTime) override;
    void Shutdown() override;

    // -- Registry (system-wide templates) --
    void RegisterAchievement(const Achievement& achievement);
    bool HasAchievement(const std::string& id) const;
    const Achievement* GetAchievement(const std::string& id) const;
    size_t GetRegisteredCount() const;

    /// Record progress on a registered achievement by event type.
    /// Returns true if the achievement is now complete.
    bool RecordProgress(const std::string& achievementId,
                        const std::string& eventType, int amount = 1);

    /// Get all registered achievements.
    std::vector<const Achievement*> GetAllAchievements() const;

    /// Get all unlocked achievement ids.
    std::vector<std::string> GetUnlockedIds() const;

    // -- Template achievements (built-in definitions) --
    static Achievement CreateFirstBlood();      // First enemy destroyed
    static Achievement CreateExplorer();         // Visit 10 sectors
    static Achievement CreateShipwright();        // Build 5 ships
    static Achievement CreateTrader();           // Complete 20 trades
    static Achievement CreateVeteran();          // Reach level 10
    static Achievement CreateMiner();            // Mine 100 asteroids
    static Achievement CreateFleetCommander();   // Have 5 ships in fleet
    static Achievement CreateRichPilot();        // Earn 10000 credits

private:
    std::unordered_map<std::string, Achievement> _registry;
};

} // namespace subspace
