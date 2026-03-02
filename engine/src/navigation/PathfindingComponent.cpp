#include "navigation/PathfindingComponent.h"

namespace subspace {

bool PathfindingComponent::HasReachedDestination() const
{
    if (!hasTarget) return true;
    if (!currentPath.valid || currentPath.IsEmpty()) return false;
    return currentWaypointIndex >= static_cast<int>(currentPath.waypoints.size());
}

Vector3 PathfindingComponent::GetNextWaypoint() const
{
    if (!currentPath.valid || currentPath.IsEmpty()) return Vector3();
    if (currentWaypointIndex >= static_cast<int>(currentPath.waypoints.size())) return Vector3();
    return currentPath.waypoints[currentWaypointIndex];
}

bool PathfindingComponent::AdvanceWaypoint(const Vector3& currentPosition)
{
    if (!currentPath.valid || currentPath.IsEmpty()) return false;
    if (currentWaypointIndex >= static_cast<int>(currentPath.waypoints.size())) return false;

    Vector3 diff = currentPath.waypoints[currentWaypointIndex] - currentPosition;
    if (diff.length() <= arrivalThreshold) {
        currentWaypointIndex++;
        return true;
    }
    return false;
}

void PathfindingComponent::SetTarget(const Vector3& target)
{
    targetPosition = target;
    hasTarget = true;
    needsRepath = true;
    currentWaypointIndex = 0;
    currentPath = NavPath();
}

void PathfindingComponent::ClearPath()
{
    currentPath = NavPath();
    currentWaypointIndex = 0;
    hasTarget = false;
    needsRepath = false;
    targetPosition = Vector3();
}

} // namespace subspace
