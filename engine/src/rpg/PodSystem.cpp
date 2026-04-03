#include "rpg/PodSystem.h"

#include <algorithm>
#include <numeric>

namespace subspace {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

const char* SkillCategoryName(SkillCategory cat) {
    switch (cat) {
        case SkillCategory::Combat:      return "Combat";
        case SkillCategory::Engineering: return "Engineering";
        case SkillCategory::Navigation:  return "Navigation";
        case SkillCategory::Trade:       return "Trade";
        case SkillCategory::Science:     return "Science";
        case SkillCategory::Leadership:  return "Leadership";
        default:                         return "Unknown";
    }
}

const char* PodAbilityTypeName(PodAbilityType type) {
    switch (type) {
        case PodAbilityType::Active:  return "Active";
        case PodAbilityType::Passive: return "Passive";
        case PodAbilityType::Toggle:  return "Toggle";
        default:                      return "Unknown";
    }
}

// ---------------------------------------------------------------------------
// PodSkillTree
// ---------------------------------------------------------------------------

PodSkillTree::PodSkillTree() {
    _skills = DefaultSkills();
}

bool PodSkillTree::LearnSkill(int skillId, int& availablePoints) {
    auto* skill = GetSkill(skillId);
    if (!skill) return false;
    if (skill->rank >= skill->maxRank) return false;
    if (availablePoints < skill->pointCost) return false;

    // Check prerequisite
    if (skill->requiredSkillId != 0) {
        const auto* prereq = GetSkill(skill->requiredSkillId);
        if (!prereq || prereq->rank == 0) return false;
    }

    availablePoints -= skill->pointCost;
    ++skill->rank;
    return true;
}

bool PodSkillTree::UnlearnSkill(int skillId, int& availablePoints) {
    auto* skill = GetSkill(skillId);
    if (!skill || skill->rank == 0) return false;

    // Check no other learned skill depends on this one
    for (const auto& s : _skills) {
        if (s.requiredSkillId == skillId && s.rank > 0) return false;
    }

    availablePoints += skill->pointCost;
    --skill->rank;
    return true;
}

PodSkill* PodSkillTree::GetSkill(int id) {
    for (auto& s : _skills) if (s.skillId == id) return &s;
    return nullptr;
}

const PodSkill* PodSkillTree::GetSkill(int id) const {
    for (const auto& s : _skills) if (s.skillId == id) return &s;
    return nullptr;
}

std::vector<const PodSkill*>
PodSkillTree::GetByCategory(SkillCategory cat) const {
    std::vector<const PodSkill*> out;
    for (const auto& s : _skills)
        if (s.category == cat) out.push_back(&s);
    return out;
}

float PodSkillTree::CategoryBonus(SkillCategory cat) const {
    float total = 0.0f;
    for (const auto& s : _skills)
        if (s.category == cat) total += s.TotalBonus();
    return total;
}

int PodSkillTree::TotalPointsSpent() const {
    int total = 0;
    for (const auto& s : _skills) total += s.rank * s.pointCost;
    return total;
}

std::vector<PodSkill> PodSkillTree::DefaultSkills() {
    return {
        // Combat
        { 1, "Gunnery I",       "Increases projectile damage",     SkillCategory::Combat,      0, 5, 1, 0.05f, 0 },
        { 2, "Gunnery II",      "Increases projectile damage more", SkillCategory::Combat,     0, 5, 2, 0.07f, 1 },
        { 3, "Shield Mastery",  "Increases shield capacity",        SkillCategory::Combat,     0, 5, 1, 0.08f, 0 },
        // Engineering
        { 4, "Repair Tech",     "Faster hull repairs",              SkillCategory::Engineering, 0, 5, 1, 0.10f, 0 },
        { 5, "Module Tuning",   "Reduces module energy use",        SkillCategory::Engineering, 0, 5, 2, 0.06f, 4 },
        // Navigation
        { 6, "Jump Drive",      "Extends jump range",               SkillCategory::Navigation, 0, 5, 1, 0.15f, 0 },
        { 7, "Warp Calculation","Reduces jump cooldown",            SkillCategory::Navigation,  0, 5, 2, 0.10f, 6 },
        // Trade
        { 8, "Negotiation",     "Better buy/sell prices",           SkillCategory::Trade,       0, 5, 1, 0.04f, 0 },
        // Science
        { 9, "Deep Scan",       "Improves scan range",              SkillCategory::Science,     0, 5, 1, 0.12f, 0 },
        // Leadership
        {10, "Command",         "Increases fleet command capacity", SkillCategory::Leadership,  0, 5, 2, 0.05f, 0 },
    };
}

ComponentData PodSkillTree::Serialize() const {
    ComponentData cd;
    cd.componentType = "PodSkillTree";
    cd.data["count"] = std::to_string(_skills.size());
    for (size_t i = 0; i < _skills.size(); ++i) {
        std::string p = "sk" + std::to_string(i) + "_";
        cd.data[p + "id"]   = std::to_string(_skills[i].skillId);
        cd.data[p + "rank"] = std::to_string(_skills[i].rank);
    }
    return cd;
}

void PodSkillTree::Deserialize(const ComponentData& data) {
    auto f = [&](const std::string& k, const std::string& def = "0") {
        auto it = data.data.find(k);
        return (it != data.data.end()) ? it->second : def;
    };
    // Re-initialize defaults first, then apply saved ranks
    _skills = DefaultSkills();
    int count = std::stoi(f("count", "0"));
    for (int i = 0; i < count; ++i) {
        std::string p = "sk" + std::to_string(i) + "_";
        int id   = std::stoi(f(p + "id", "0"));
        int rank = std::stoi(f(p + "rank", "0"));
        auto* skill = GetSkill(id);
        if (skill) skill->rank = rank;
    }
}

// ---------------------------------------------------------------------------
// PodAbilities
// ---------------------------------------------------------------------------

PodAbilities::PodAbilities() {
    _abilities = DefaultAbilities();
}

bool PodAbilities::UnlockAbility(int abilityId, const PodSkillTree& skillTree) {
    auto* ab = GetAbility(abilityId);
    if (!ab || ab->isUnlocked) return false;

    if (ab->requiredSkillId != 0) {
        const auto* skill = skillTree.GetSkill(ab->requiredSkillId);
        if (!skill || skill->rank < ab->requiredSkillRank) return false;
    }

    ab->isUnlocked = true;
    return true;
}

bool PodAbilities::ActivateAbility(int abilityId) {
    auto* ab = GetAbility(abilityId);
    if (!ab || !ab->isUnlocked || !ab->IsReady()) return false;

    if (ab->type == PodAbilityType::Toggle) {
        ab->isActive = !ab->isActive;
        return true;
    }
    if (ab->type == PodAbilityType::Active) {
        ab->remainingCooldown = ab->cooldownSec;
        return true;
    }
    return false; // Passive cannot be activated
}

bool PodAbilities::DeactivateAbility(int abilityId) {
    auto* ab = GetAbility(abilityId);
    if (!ab || ab->type != PodAbilityType::Toggle) return false;
    ab->isActive = false;
    return true;
}

void PodAbilities::Update(float deltaTime) {
    for (auto& ab : _abilities) {
        if (ab.remainingCooldown > 0.0f)
            ab.remainingCooldown = std::max(0.0f, ab.remainingCooldown - deltaTime);
    }
}

PodAbility* PodAbilities::GetAbility(int id) {
    for (auto& a : _abilities) if (a.abilityId == id) return &a;
    return nullptr;
}

const PodAbility* PodAbilities::GetAbility(int id) const {
    for (const auto& a : _abilities) if (a.abilityId == id) return &a;
    return nullptr;
}

std::vector<const PodAbility*> PodAbilities::UnlockedAbilities() const {
    std::vector<const PodAbility*> out;
    for (const auto& a : _abilities) if (a.isUnlocked) out.push_back(&a);
    return out;
}

std::vector<const PodAbility*> PodAbilities::ReadyActiveAbilities() const {
    std::vector<const PodAbility*> out;
    for (const auto& a : _abilities)
        if (a.isUnlocked && a.type == PodAbilityType::Active && a.IsReady())
            out.push_back(&a);
    return out;
}

std::vector<PodAbility> PodAbilities::DefaultAbilities() {
    return {
        { 1, "Overclock Weapons", "Doubles fire rate for 10s",       PodAbilityType::Active,  SkillCategory::Combat,      20.0f, 0, 10.0f, 2.0f, false, false, 1, 2 },
        { 2, "Emergency Shield",  "Instantly restores 50% shields",  PodAbilityType::Active,  SkillCategory::Combat,      60.0f, 0,  0.0f, 0.5f, false, false, 3, 1 },
        { 3, "Combat Aura",       "Passively increases damage 10%",  PodAbilityType::Passive, SkillCategory::Combat,       0.0f, 0,  0.0f, 0.1f, false, false, 1, 3 },
        { 4, "Repair Nanobots",   "Continuous hull repair",          PodAbilityType::Toggle,  SkillCategory::Engineering,  0.0f, 0,  0.0f, 0.02f,false, false, 4, 1 },
        { 5, "Micro Jump",        "Short-range jump (50 sectors)",   PodAbilityType::Active,  SkillCategory::Navigation,  30.0f, 0,  0.0f, 50.0f,false, false, 6, 2 },
        { 6, "Trade Insight",     "Reveals station commodity prices",PodAbilityType::Active,  SkillCategory::Trade,       120.0f,0, 30.0f, 1.0f, false, false, 8, 1 },
        { 7, "Enhanced Scan",     "Reveals hidden signatures",       PodAbilityType::Active,  SkillCategory::Science,     45.0f, 0, 15.0f, 1.5f, false, false, 9, 2 },
    };
}

ComponentData PodAbilities::Serialize() const {
    ComponentData cd;
    cd.componentType = "PodAbilities";
    cd.data["count"] = std::to_string(_abilities.size());
    for (size_t i = 0; i < _abilities.size(); ++i) {
        std::string p = "ab" + std::to_string(i) + "_";
        cd.data[p + "id"]      = std::to_string(_abilities[i].abilityId);
        cd.data[p + "unlocked"]= _abilities[i].isUnlocked ? "1" : "0";
        cd.data[p + "active"]  = _abilities[i].isActive   ? "1" : "0";
        cd.data[p + "cd"]      = std::to_string(_abilities[i].remainingCooldown);
    }
    return cd;
}

void PodAbilities::Deserialize(const ComponentData& data) {
    auto f = [&](const std::string& k, const std::string& def = "0") {
        auto it = data.data.find(k);
        return (it != data.data.end()) ? it->second : def;
    };
    _abilities = DefaultAbilities();
    int count = std::stoi(f("count", "0"));
    for (int i = 0; i < count; ++i) {
        std::string p = "ab" + std::to_string(i) + "_";
        int id = std::stoi(f(p + "id", "0"));
        auto* ab = GetAbility(id);
        if (ab) {
            ab->isUnlocked        = (f(p + "unlocked") == "1");
            ab->isActive          = (f(p + "active")   == "1");
            ab->remainingCooldown = std::stof(f(p + "cd", "0"));
        }
    }
}

} // namespace subspace
