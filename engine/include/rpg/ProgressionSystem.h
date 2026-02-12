#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"

#include <algorithm>
#include <string>
#include <unordered_map>

namespace subspace {

/// Component for entity progression (XP, levels, skill points).
struct ProgressionComponent : public IComponent {
    int level = 1;
    int experience = 0;
    int experienceToNextLevel = 100;
    int skillPoints = 0;

    /// Add experience. Returns true if a level-up occurred.
    bool AddExperience(int amount);

private:
    void LevelUp();
};

/// Component for faction relations (reputation per named faction).
struct FactionComponent : public IComponent {
    std::string factionName = "Neutral";
    std::unordered_map<std::string, int> reputation; // faction name -> value

    /// Modify reputation with a faction, clamped to [-100, 100].
    void ModifyReputation(const std::string& faction, int amount);

    /// Get reputation with a faction (0 if unknown).
    int GetReputation(const std::string& faction) const;

    /// Returns true if reputation >= 50.
    bool IsFriendly(const std::string& faction) const;

    /// Returns true if reputation <= -50.
    bool IsHostile(const std::string& faction) const;
};

} // namespace subspace
