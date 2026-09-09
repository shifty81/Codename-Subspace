#pragma once

#include "core/Math.h"
#include "core/resources/Inventory.h"
#include "procedural/GalaxyGenerator.h"
#include "weapons/MissileSystem.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace subspace {

struct AsteroidFractureState {
    std::size_t asteroidIndex = 0;
    float integrity = 100.0f;
    bool fractured = false;
};

struct MiningFragment {
    std::uint64_t fragmentId = 0;
    std::size_t sourceAsteroidIndex = 0;
    Vector3 position{};
    Vector3 velocity{};
    float size = 0.20f;
    float resourceUnits = 1.0f;
    ResourceType resourceType = ResourceType::Iron;
    bool recovered = false;
};

struct MiningDustCloud {
    Vector3 position{};
    float baseRadius = 1.0f;
    float age = 0.0f;
    float lifetime = 4.5f;
    float density = 0.6f;
    std::uint32_t seed = 1;
};

struct AsteroidFractureResult {
    bool hit = false;
    bool fractured = false;
    std::size_t asteroidIndex = 0;
    int fragmentsSpawned = 0;
    float resourceUnitsReleased = 0.0f;
};

/// Runtime fracture authority for generated asteroid fields. It does not alter
/// the deterministic sector seed; instead it layers mutable mining state over
/// generated AsteroidData for the current save/session.
class AsteroidFractureSystem {
public:
    void Initialize(const GalaxySector& sector);
    void Clear();
    void Update(float deltaTime);

    AsteroidFractureResult ApplyDetonation(const MissileDetonation& detonation,
                                           const GalaxySector& sector);

    bool IsFractured(std::size_t asteroidIndex) const;
    float GetIntegrity(std::size_t asteroidIndex) const;

    /// Recover nearby loose fragments. Recovered fragments remain in the list
    /// for deterministic visual fade/state inspection but are marked recovered.
    float RecoverNearby(const Vector3& position, float radius, ResourceType* lastType = nullptr);

    const std::vector<AsteroidFractureState>& GetStates() const { return _states; }
    const std::vector<MiningFragment>& GetFragments() const { return _fragments; }
    const std::vector<MiningDustCloud>& GetDustClouds() const { return _dustClouds; }
    float GetRecoveredResourceUnits() const { return _recoveredResourceUnits; }

private:
    void SpawnFragments(std::size_t asteroidIndex,
                        const AsteroidData& asteroid,
                        const Vector3& impactPosition,
                        float energy);

    std::vector<AsteroidFractureState> _states;
    std::vector<MiningFragment> _fragments;
    std::vector<MiningDustCloud> _dustClouds;
    std::uint64_t _nextFragmentId = 1;
    float _recoveredResourceUnits = 0.0f;
};

} // namespace subspace
