#pragma once

#include "core/Math.h"
#include "procedural/GalaxyGenerator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class OrbitalBodyKind { Star, Planet, Moon, Station, BeltObject, AsteroidStation, DeepSpace };

struct OrbitalElements {
    double semiMajorAxis = 1000.0;
    double eccentricity = 0.0;
    double inclinationDegrees = 0.0;
    double longitudeAscendingNodeDegrees = 0.0;
    double argumentOfPeriapsisDegrees = 0.0;
    double meanAnomalyAtEpochDegrees = 0.0;
    double orbitalPeriodSeconds = 3600.0;
    double epochSeconds = 0.0;
};

struct OrbitalBodyRecord {
    std::uint64_t id = 0;
    std::uint64_t parentId = 0;
    std::string name;
    OrbitalBodyKind kind = OrbitalBodyKind::Planet;
    OrbitalElements orbit{};
    float physicalRadius = 1.0f;
    bool dockable = false;
};

struct StellarSafetyEnvelope {
    float visualRadius = 0.0f;
    float coronaRadius = 0.0f;
    float hazardRadius = 0.0f;
    float spawnRadius = 0.0f;
};

struct OrbitalIntercept {
    bool valid = false;
    Vector3 predictedTarget{};
    double arrivalTimeSeconds = 0.0;
    double missDistance = 0.0;
};

class OrbitalDynamicsSystem {
public:
    Vector3 Evaluate(const OrbitalBodyRecord& body, double simulationSeconds, const Vector3& parentPosition = {}) const;
    StellarSafetyEnvelope StarSafety(const StarData& star) const;
    bool IsOutsideStarSafety(const Vector3& position, const Vector3& starPosition, const StellarSafetyEnvelope& envelope) const;
    OrbitalIntercept PredictIntercept(const OrbitalBodyRecord& body, const Vector3& parentPosition, double nowSeconds, double travelSeconds) const;
    std::vector<OrbitalBodyRecord> DeriveSystemOrbits(const GalaxySector& sector) const;
};

} // namespace subspace
