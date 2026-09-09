#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/Math.h"
#include "core/physics/PhysicsComponent.h"

#include <vector>

namespace subspace {

/// Status of a target lock.
enum class LockState { None, Acquiring, Locked };

/// Component that gives an entity target-locking capability.
struct TargetLockComponent : public IComponent {
    EntityId targetId = InvalidEntityId;
    LockState lockState = LockState::None;
    float lockRange = 500.0f;       // max lock-on distance
    float lockAcquireTime = 2.0f;   // seconds to achieve full lock
    float lockTimer = 0.0f;         // current progress toward lock
    float lockBreakRange = 600.0f;  // distance at which lock is broken (> lockRange)
    int maxTrackedTargets = 1;      // for future multi-lock
    std::vector<EntityId> trackedTargets; // secondary tracked targets

    /// Begin acquiring a lock on a target.
    void BeginLock(EntityId target);

    /// Cancel any active lock.
    void ClearLock();

    /// Returns true when the lock is fully acquired.
    bool IsLocked() const;

    /// Returns true when in the process of acquiring a lock.
    bool IsAcquiring() const;

    /// Returns lock progress as percentage (0-100).
    float GetLockProgress() const;
};

/// System that updates target lock state each frame.
class TargetLockSystem : public SystemBase {
public:
    TargetLockSystem();
    explicit TargetLockSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    /// Get the distance between two entities using their PhysicsComponents.
    /// Returns -1 if either entity lacks a PhysicsComponent.
    float GetDistanceBetween(EntityId a, EntityId b) const;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
