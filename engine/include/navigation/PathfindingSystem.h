#pragma once

#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "navigation/Pathfinding.h"
#include "navigation/PathfindingComponent.h"

#include <memory>

namespace subspace {

/// System that manages pathfinding for all entities with PathfindingComponent.
/// Owns a shared NavGraph and Pathfinder that entities use.
class PathfindingSystem : public SystemBase {
public:
    explicit PathfindingSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    /// Access the shared navigation graph (for building/modifying).
    NavGraph& GetNavGraph() { return _navGraph; }
    const NavGraph& GetNavGraph() const { return _navGraph; }

    /// Access the pathfinder.
    const Pathfinder& GetPathfinder() const { return _pathfinder; }

    /// Request a path for a specific component (immediate).
    NavPath RequestPath(const Vector3& from, const Vector3& to);

    /// Rebuild the navigation graph as a 3D grid covering a given volume.
    void BuildNavGrid(const Vector3& center, float spacing,
                      int countX, int countY, int countZ);

    /// Get total paths calculated since construction.
    int GetTotalPathsCalculated() const { return _totalPathsCalculated; }

private:
    EntityManager& _entityManager;
    NavGraph _navGraph;
    Pathfinder _pathfinder;
    int _totalPathsCalculated = 0;
};

} // namespace subspace
