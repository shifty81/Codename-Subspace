#include "rpg/ProgressionSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// ProgressionComponent
// ---------------------------------------------------------------------------
bool ProgressionComponent::AddExperience(int amount) {
    experience += amount;
    if (experience >= experienceToNextLevel) {
        LevelUp();
        return true;
    }
    return false;
}

void ProgressionComponent::LevelUp() {
    level++;
    experience -= experienceToNextLevel;
    experienceToNextLevel = static_cast<int>(experienceToNextLevel * 1.5f);
    skillPoints += 3;
}

// ---------------------------------------------------------------------------
// FactionComponent
// ---------------------------------------------------------------------------
void FactionComponent::ModifyReputation(const std::string& faction, int amount) {
    reputation[faction] += amount;
    reputation[faction] = std::max(-100, std::min(100, reputation[faction]));
}

int FactionComponent::GetReputation(const std::string& faction) const {
    auto it = reputation.find(faction);
    if (it != reputation.end()) return it->second;
    return 0;
}

bool FactionComponent::IsFriendly(const std::string& faction) const {
    return GetReputation(faction) >= 50;
}

bool FactionComponent::IsHostile(const std::string& faction) const {
    return GetReputation(faction) <= -50;
}

} // namespace subspace
