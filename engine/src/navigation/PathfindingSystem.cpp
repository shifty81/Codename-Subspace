#include "navigation/PathfindingSystem.h"

#include "core/physics/PhysicsComponent.h"

namespace subspace {

PathfindingSystem::PathfindingSystem(EntityManager& entityManager)
    : SystemBase("PathfindingSystem")
    , _entityManager(entityManager)
    , _pathfinder(_navGraph)
{
}

void PathfindingSystem::Update(float deltaTime)
{
    auto components = _entityManager.GetAllComponents<PathfindingComponent>();

    for (auto* pf : components) {
        if (!pf->hasTarget) continue;

        pf->timeSinceLastRepath += deltaTime;

        // Repath if needed.
        if (pf->needsRepath || pf->timeSinceLastRepath >= pf->repathInterval) {
            auto* physics = _entityManager.GetComponent<PhysicsComponent>(pf->entityId);
            if (!physics) continue; // Cannot pathfind without a position.

            NavPath path = _pathfinder.FindPathByPosition(physics->position, pf->targetPosition);
            if (path.valid) {
                pf->currentPath = path;
                pf->currentWaypointIndex = 0;
            } else {
                pf->currentPath = NavPath();
                pf->currentWaypointIndex = 0;
            }
            pf->needsRepath = false;
            pf->timeSinceLastRepath = 0.0f;
            _totalPathsCalculated++;
        }

        // Advance waypoints based on current position.
        auto* physics = _entityManager.GetComponent<PhysicsComponent>(pf->entityId);
        if (physics) {
            pf->AdvanceWaypoint(physics->position);
        }
    }
}

NavPath PathfindingSystem::RequestPath(const Vector3& from, const Vector3& to)
{
    _totalPathsCalculated++;
    return _pathfinder.FindPathByPosition(from, to);
}

void PathfindingSystem::BuildNavGrid(const Vector3& center, float spacing,
                                      int countX, int countY, int countZ)
{
    _navGraph.BuildGrid(center, spacing, countX, countY, countZ);
}

} // namespace subspace
