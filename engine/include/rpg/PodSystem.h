#pragma once

#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace subspace {

// ---------------------------------------------------------------------------
// PodSkillTree
// ---------------------------------------------------------------------------

enum class SkillCategory {
    Combat,       ///< Attack, defense, weapon bonuses
    Engineering,  ///< Repair speed, module efficiency
    Navigation,   ///< Jump range, travel speed
    Trade,        ///< Buy/sell bonuses, market access
    Science,      ///< Scanning, research speed
    Leadership    ///< Fleet bonuses, crew morale
};

const char* SkillCategoryName(SkillCategory cat);

struct PodSkill {
    int          skillId    = 0;
    std::string  name;
    std::string  description;
    SkillCategory category  = SkillCategory::Combat;
    int          rank       = 0;   ///< Current rank (0 = unlearned)
    int          maxRank    = 5;
    int          pointCost  = 1;   ///< Skill points per rank
    float        bonusPerRank = 0.05f; ///< Bonus magnitude per rank level
    int          requiredSkillId = 0;  ///< Prerequisite (0 = none)

    bool IsLearned() const { return rank > 0; }
    float TotalBonus() const { return rank * bonusPerRank; }
};

/// Manages the pilot skill tree for a Pod (player character).
class PodSkillTree {
public:
    PodSkillTree();

    /// Learn a skill by id (spend skill points). Returns false if insufficient
    /// points or prerequisites not met.
    bool LearnSkill(int skillId, int& availablePoints);

    /// Unlearn a skill, refunding points. Returns false if dependents exist.
    bool UnlearnSkill(int skillId, int& availablePoints);

    /// Get skill by id. Returns nullptr if not found.
    PodSkill* GetSkill(int skillId);
    const PodSkill* GetSkill(int skillId) const;

    /// All skills in a given category.
    std::vector<const PodSkill*> GetByCategory(SkillCategory cat) const;

    /// Sum of all bonus values for a category.
    float CategoryBonus(SkillCategory cat) const;

    /// Total skill points invested.
    int TotalPointsSpent() const;

    /// Load default skill tree definitions.
    static std::vector<PodSkill> DefaultSkills();

    const std::vector<PodSkill>& AllSkills() const { return _skills; }

    ComponentData Serialize()   const;
    void Deserialize(const ComponentData& data);

private:
    std::vector<PodSkill> _skills;
};

// ---------------------------------------------------------------------------
// PodAbilities
// ---------------------------------------------------------------------------

enum class PodAbilityType {
    Active,     ///< Player-activated, has cooldown
    Passive,    ///< Always on
    Toggle      ///< On/off switch
};

const char* PodAbilityTypeName(PodAbilityType type);

struct PodAbility {
    int             abilityId    = 0;
    std::string     name;
    std::string     description;
    PodAbilityType  type         = PodAbilityType::Passive;
    SkillCategory   category     = SkillCategory::Combat;
    float           cooldownSec  = 0.0f;
    float           remainingCooldown = 0.0f;
    float           duration     = 0.0f;    ///< Duration of active effect (0 = instant)
    float           magnitude    = 1.0f;
    bool            isUnlocked   = false;
    bool            isActive     = false;   ///< For Toggle abilities
    int             requiredSkillId = 0;
    int             requiredSkillRank = 1;

    bool IsReady() const { return remainingCooldown <= 0.0f; }
};

/// Manages the Pod's ability set — unlocks, cooldowns, and activation.
class PodAbilities {
public:
    PodAbilities();

    /// Unlock an ability (requires the prerequisite skill at specified rank).
    bool UnlockAbility(int abilityId, const PodSkillTree& skillTree);

    /// Activate an active/toggle ability. Returns false if on cooldown or locked.
    bool ActivateAbility(int abilityId);

    /// Deactivate a toggle ability.
    bool DeactivateAbility(int abilityId);

    /// Tick ability cooldowns and durations.
    void Update(float deltaTime);

    PodAbility*       GetAbility(int id);
    const PodAbility* GetAbility(int id) const;

    std::vector<const PodAbility*> UnlockedAbilities() const;
    std::vector<const PodAbility*> ReadyActiveAbilities() const;

    static std::vector<PodAbility> DefaultAbilities();

    const std::vector<PodAbility>& AllAbilities() const { return _abilities; }

    ComponentData Serialize()   const;
    void Deserialize(const ComponentData& data);

private:
    std::vector<PodAbility> _abilities;
};

} // namespace subspace
