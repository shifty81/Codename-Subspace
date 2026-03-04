#include "combat/TargetLockSystem.h"

#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// TargetLockComponent
// ---------------------------------------------------------------------------
void TargetLockComponent::BeginLock(EntityId target) {
    targetId = target;
    lockState = LockState::Acquiring;
    lockTimer = 0.0f;
}

void TargetLockComponent::ClearLock() {
    targetId = InvalidEntityId;
    lockState = LockState::None;
    lockTimer = 0.0f;
    trackedTargets.clear();
}

bool TargetLockComponent::IsLocked() const {
    return lockState == LockState::Locked;
}

bool TargetLockComponent::IsAcquiring() const {
    return lockState == LockState::Acquiring;
}

float TargetLockComponent::GetLockProgress() const {
    if (lockAcquireTime <= 0.0f) return 100.0f;
    float progress = (lockTimer / lockAcquireTime) * 100.0f;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 100.0f) progress = 100.0f;
    return progress;
}

// ---------------------------------------------------------------------------
// TargetLockSystem
// ---------------------------------------------------------------------------
TargetLockSystem::TargetLockSystem() : SystemBase("TargetLockSystem") {}

TargetLockSystem::TargetLockSystem(EntityManager& entityManager)
    : SystemBase("TargetLockSystem")
    , _entityManager(&entityManager)
{
}

float TargetLockSystem::GetDistanceBetween(EntityId a, EntityId b) const {
    if (!_entityManager) return -1.0f;

    auto* physA = _entityManager->GetComponent<PhysicsComponent>(a);
    auto* physB = _entityManager->GetComponent<PhysicsComponent>(b);
    if (!physA || !physB) return -1.0f;

    float dx = physB->position.x - physA->position.x;
    float dy = physB->position.y - physA->position.y;
    float dz = physB->position.z - physA->position.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

void TargetLockSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto lockComponents = _entityManager->GetAllComponents<TargetLockComponent>();
    for (auto* comp : lockComponents) {
        if (comp->lockState == LockState::None) continue;
        if (comp->targetId == InvalidEntityId) continue;

        float distance = GetDistanceBetween(comp->entityId, comp->targetId);

        if (comp->lockState == LockState::Acquiring) {
            if (distance < 0.0f || distance > comp->lockRange) {
                comp->ClearLock();
                continue;
            }
            comp->lockTimer += deltaTime;
            if (comp->lockTimer >= comp->lockAcquireTime) {
                comp->lockState = LockState::Locked;
            }
        } else if (comp->lockState == LockState::Locked) {
            if (distance < 0.0f || distance > comp->lockBreakRange) {
                comp->ClearLock();
            }
        }
    }
}

} // namespace subspace
