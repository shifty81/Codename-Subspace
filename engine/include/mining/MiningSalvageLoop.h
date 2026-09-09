#pragma once

#include "core/Math.h"
#include "core/resources/Inventory.h"
#include "mining/AsteroidFractureSystem.h"
#include "procedural/GalaxyGenerator.h"
#include "weapons/MissileSystem.h"

#include <cstdint>
#include <unordered_set>

namespace subspace {

struct MiningSalvageTelemetry {
    int miningMissilesLaunched = 0;
    int asteroidsFractured = 0;
    float oreRecovered = 0.0f;
    float salvageRecovered = 0.0f;
    ResourceType lastRecoveredType = ResourceType::Iron;
};

/// Small native gameplay coordinator for the initial discover -> fracture ->
/// recover loop. It intentionally uses MissileSystem and AsteroidFractureSystem
/// rather than creating renderer-owned fake mining effects.
class MiningSalvageLoop {
public:
    void Initialize(const GalaxySector& sector);
    void Update(float deltaTime, const Vector3& playerPosition, GalaxySector& sector);

    std::uint64_t LaunchMiningMissile(const Vector3& position,
                                      const Vector3& velocity,
                                      const Vector3& forward,
                                      const Vector3& target);

    MissileSystem& GetMissileSystem() { return _missiles; }
    const MissileSystem& GetMissileSystem() const { return _missiles; }
    AsteroidFractureSystem& GetFractureSystem() { return _fracture; }
    const AsteroidFractureSystem& GetFractureSystem() const { return _fracture; }
    const MiningSalvageTelemetry& GetTelemetry() const { return _telemetry; }

private:
    MissileSystem _missiles;
    AsteroidFractureSystem _fracture;
    MiningSalvageTelemetry _telemetry{};
    std::uint64_t _lastProcessedDetonation = 0;
    std::unordered_set<std::string> _recoveredDerelicts;
};

} // namespace subspace
