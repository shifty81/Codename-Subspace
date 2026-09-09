#pragma once

#include "core/ecs/IComponent.h"
#include "navigation/Pathfinding.h"

namespace subspace {

/// Component for entities that use pathfinding.
struct PathfindingComponent : IComponent {
    NavPath currentPath;                     ///< Currently active path.
    int currentWaypointIndex = 0;            ///< Index into currentPath.waypoints.
    float arrivalThreshold = 5.0f;           ///< Distance to consider a waypoint reached.
    float repathInterval = 2.0f;             ///< How often to recalculate path (seconds).
    float timeSinceLastRepath = 0.0f;        ///< Accumulator for repath timer.
    Vector3 targetPosition;                  ///< Destination to path toward.
    bool hasTarget = false;                  ///< Whether a target position is set.
    bool needsRepath = false;                ///< Whether the path should be recalculated.
    float pathfindingSpeed = 1.0f;           ///< Speed multiplier along path.

    /// Whether the entity has reached its destination.
    bool HasReachedDestination() const;

    /// Get the next waypoint to move toward (returns zero vector if done).
    Vector3 GetNextWaypoint() const;

    /// Advance to the next waypoint if close enough to the current one.
    bool AdvanceWaypoint(const Vector3& currentPosition);

    /// Set a new pathfinding target.
    void SetTarget(const Vector3& target);

    /// Clear the current path and target.
    void ClearPath();
};

} // namespace subspace
