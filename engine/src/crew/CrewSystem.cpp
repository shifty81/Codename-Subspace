#include "crew/CrewSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// Pilot
// ---------------------------------------------------------------------------
float Pilot::GetOverallSkill() const {
    return (combatSkill + navigationSkill + engineeringSkill) / 3.0f;
}

bool Pilot::AddExperience(int xp) {
    experience += xp;
    int xpNeeded = level * 500;

    if (experience >= xpNeeded) {
        experience -= xpNeeded;
        level++;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// CrewComponent
// ---------------------------------------------------------------------------
bool CrewComponent::HasSufficientCrew() const {
    return currentCrew >= minimumCrew;
}

bool CrewComponent::HasPilot() const {
    return hasPilot;
}

bool CrewComponent::IsOperational() const {
    return HasPilot() && HasSufficientCrew();
}

bool CrewComponent::AssignPilot(Pilot& pilot) {
    if (pilot.IsAssigned()) return false;

    assignedPilot = pilot;
    hasPilot = true;
    pilot.assignedShipId = entityId;
    return true;
}

bool CrewComponent::RemovePilot(Pilot& outPilot) {
    if (!hasPilot) return false;

    outPilot = assignedPilot;
    outPilot.assignedShipId = InvalidEntityId;
    hasPilot = false;
    assignedPilot = Pilot{};
    return true;
}

bool CrewComponent::AddCrew(int count) {
    if (currentCrew + count > maxCrew) return false;
    currentCrew += count;
    UpdateCrewEfficiency();
    return true;
}

bool CrewComponent::RemoveCrew(int count) {
    if (currentCrew - count < 0) return false;
    currentCrew -= count;
    UpdateCrewEfficiency();
    return true;
}

float CrewComponent::GetCrewEfficiency() const {
    return crewEfficiency;
}

void CrewComponent::UpdateCrewEfficiency() {
    if (currentCrew < minimumCrew) {
        crewEfficiency = static_cast<float>(currentCrew) / static_cast<float>(minimumCrew);
    } else if (currentCrew > minimumCrew) {
        float bonus = std::min(0.2f, (currentCrew - minimumCrew) * 0.02f);
        crewEfficiency = 1.0f + bonus;
    } else {
        crewEfficiency = 1.0f;
    }
}

} // namespace subspace
