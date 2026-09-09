#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"

#include <algorithm>
#include <string>
#include <vector>

namespace subspace {

/// Represents a pilot that can be assigned to a ship.
struct Pilot {
    std::string name;
    int level = 1;
    int experience = 0;

    float combatSkill = 0.5f;      // 0-1, affects weapon accuracy
    float navigationSkill = 0.5f;  // 0-1, affects maneuverability
    float engineeringSkill = 0.5f; // 0-1, affects power efficiency

    int hiringCost = 1000;
    int dailySalary = 100;

    EntityId assignedShipId = InvalidEntityId;

    bool IsAssigned() const { return assignedShipId != InvalidEntityId; }

    /// Average of the three skills.
    float GetOverallSkill() const;

    /// Add experience. Returns true if a level-up occurred.
    bool AddExperience(int xp);
};

/// Component for managing crew and pilot on a ship entity.
struct CrewComponent : public IComponent {
    int minimumCrew = 1;
    int currentCrew = 0;
    int maxCrew = 10;
    int crewQuartersCapacity = 10;
    float crewEfficiency = 1.0f;

    /// Pilot currently assigned (check hasPilot before use).
    bool hasPilot = false;
    Pilot assignedPilot;

    /// Check if ship has sufficient crew.
    bool HasSufficientCrew() const;

    /// Check if ship has a pilot.
    bool HasPilot() const;

    /// Check if ship is operational (has pilot and crew).
    bool IsOperational() const;

    /// Assign a pilot. Returns false if pilot is already assigned elsewhere.
    bool AssignPilot(Pilot& pilot);

    /// Remove and return the assigned pilot. Returns false if no pilot.
    bool RemovePilot(Pilot& outPilot);

    /// Add crew members. Returns false if exceeds max.
    bool AddCrew(int count);

    /// Remove crew members. Returns false if not enough.
    bool RemoveCrew(int count);

    /// Get current crew efficiency.
    float GetCrewEfficiency() const;

private:
    void UpdateCrewEfficiency();
};

} // namespace subspace
