#pragma once

#include "core/Math.h"
#include "weapons/WeaponSystem.h"

#include <cstdint>
#include <string>

namespace subspace {

enum class FireControlMode { LockedAutomatic, LockedManualAim, FreeFire };

struct WeaponFirePolicy {
    bool requiresTargetLock = true;
    bool allowsManualAim = true;
    bool allowsFreeFire = false;
    bool guided = false;
    float convergenceRange = 120.0f;
};

struct FireControlRequest {
    FireControlMode mode = FireControlMode::LockedAutomatic;
    bool targetLocked = false;
    bool triggerHeld = false;
    Vector3 muzzleWorld{};
    Vector3 lockedTargetWorld{};
    Vector3 pointerWorld{};
    Vector3 targetVelocity{};
    float projectileSpeed = 200.0f;
};

struct FireControlSolution {
    bool valid = false;
    bool mayFire = false;
    bool lockRequired = true;
    Vector3 aimPoint{};
    Vector3 aimDirection{0.0f,1.0f,0.0f};
    float leadSeconds = 0.0f;
    std::string status;
};

class FireControlSystem {
public:
    static WeaponFirePolicy PolicyFor(WeaponType type);
    static FireControlSolution Solve(WeaponType type,const FireControlRequest& request);
};

} // namespace subspace
