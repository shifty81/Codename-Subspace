#pragma once

#include "core/Math.h"

#include <cstdint>
#include <vector>

namespace subspace {

enum class MissilePayloadType {
    HighExplosive,
    MiningFracture,
    SalvageBreacher
};

struct MissileDefinition {
    MissilePayloadType payload = MissilePayloadType::HighExplosive;
    float launchSpeed = 12.0f;
    float acceleration = 10.0f;
    float maxSpeed = 34.0f;
    float guidanceStrength = 4.0f;
    float proximityFuse = 0.75f;
    float lifetime = 8.0f;
    float detonationRadius = 1.5f;
    float impulse = 2500.0f;
    float damage = 80.0f;
    float fractureEnergy = 0.0f;
    int particleBudget = 80;
};

struct MissileProjectile {
    std::uint64_t id = 0;
    MissilePayloadType payload = MissilePayloadType::HighExplosive;
    Vector3 position{};
    Vector3 previousPosition{};
    Vector3 velocity{};
    Vector3 targetPosition{};
    float age = 0.0f;
    float lifetime = 8.0f;
    float acceleration = 10.0f;
    float maxSpeed = 34.0f;
    float guidanceStrength = 4.0f;
    float proximityFuse = 0.75f;
    float detonationRadius = 1.5f;
    float impulse = 2500.0f;
    float damage = 80.0f;
    float fractureEnergy = 0.0f;
    int particleBudget = 80;
    bool guided = false;
    bool alive = true;
};

struct MissileDetonation {
    std::uint64_t missileId = 0;
    MissilePayloadType payload = MissilePayloadType::HighExplosive;
    Vector3 position{};
    float radius = 1.0f;
    float impulse = 0.0f;
    float damage = 0.0f;
    float fractureEnergy = 0.0f;
    int particleBudget = 0;
    float age = 0.0f;
    float lifetime = 1.25f;
};

/// Native Newtonian-ish missile authority. Missiles inherit launcher velocity,
/// preserve inertia, and can apply bounded guidance acceleration rather than
/// being animated directly toward a target.
class MissileSystem {
public:
    static MissileDefinition DefinitionFor(MissilePayloadType payload);

    std::uint64_t Launch(const Vector3& position,
                         const Vector3& launcherVelocity,
                         const Vector3& forward,
                         const Vector3& targetPosition,
                         MissilePayloadType payload,
                         bool guided = true);

    void Update(float deltaTime);
    void Clear();

    const std::vector<MissileProjectile>& GetProjectiles() const { return _projectiles; }
    const std::vector<MissileDetonation>& GetDetonations() const { return _detonations; }

    /// Forces a projectile to detonate immediately. Returns false if no live
    /// projectile exists with that id.
    bool Detonate(std::uint64_t missileId);

private:
    void EmitDetonation(const MissileProjectile& missile);

    std::vector<MissileProjectile> _projectiles;
    std::vector<MissileDetonation> _detonations;
    std::uint64_t _nextId = 1;
};

} // namespace subspace
