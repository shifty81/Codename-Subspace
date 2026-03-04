#pragma once

#include "core/resources/Inventory.h"

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace subspace {

/// Simple 3D float vector for procedural generation positions.
struct SectorPosition {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

/// Data for an asteroid within a sector.
struct AsteroidData {
    SectorPosition position;
    float size = 10.0f;
    ResourceType resourceType = ResourceType::Iron;
};

/// Data for a station within a sector.
struct StationData {
    SectorPosition position;
    std::string stationType = "Trading";
    std::string name = "Unknown Station";
};

/// Data for a ship within a sector.
struct ShipData {
    SectorPosition position;
    std::string shipType = "Fighter";
    std::string faction = "Neutral";
};

/// Data for a wormhole connection within a sector.
struct WormholeData {
    SectorPosition position;
    std::string designation = "Unknown";
    int wormholeClass = 1;     // 1-6
    std::string type = "Wandering";
    SectorPosition destinationSector;
};

/// Types of spatial anomalies found in sectors.
enum class AnomalyType { Nebula, BlackHole, RadiationZone, IonStorm, GravityWell };

/// Data for an anomaly within a sector.
struct AnomalyData {
    SectorPosition position;
    AnomalyType type = AnomalyType::Nebula;
    float radius = 50.0f;           // area of effect radius
    float intensity = 1.0f;         // effect strength (0-1 scale typically, but can exceed)
    std::string name = "Unknown Anomaly";
};

/// Represents a generated galaxy sector.
struct GalaxySector {
    int x = 0;
    int y = 0;
    int z = 0;
    std::vector<AsteroidData> asteroids;
    std::vector<ShipData> ships;
    std::vector<WormholeData> wormholes;
    bool hasStation = false;
    StationData station;
    std::vector<AnomalyData> anomalies;

    GalaxySector() = default;
    GalaxySector(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
};

/// Deterministic procedural galaxy generator.
/// Uses seeded RNG so the same coordinates always produce the same sector.
class GalaxyGenerator {
public:
    explicit GalaxyGenerator(int seed = 0);

    /// Generate a galaxy sector at the given coordinates.
    GalaxySector GenerateSector(int x, int y, int z) const;

    /// Get the generator seed.
    int GetSeed() const { return _seed; }

    /// Station spawn probability (0-1, default 0.2).
    float stationProbability = 0.2f;

    /// Wormhole spawn probability (0-1, default 0.05).
    float wormholeProbability = 0.05f;

    /// Anomaly spawn probability (0-1, default 0.15).
    float anomalyProbability = 0.15f;

    /// Min/max asteroid count per sector.
    int minAsteroids = 5;
    int maxAsteroids = 20;

private:
    int _seed;

    /// Deterministic hash of sector coordinates and seed.
    int HashCoordinates(int x, int y, int z) const;

    /// Pick a random resource type weighted by distance from center.
    ResourceType GetRandomResourceType(std::mt19937& rng) const;

    /// Pick a random station type.
    std::string GetRandomStationType(std::mt19937& rng) const;

    /// Generate a station name.
    std::string GenerateStationName(std::mt19937& rng) const;

    /// Generate a wormhole designation (e.g. "A123").
    std::string GenerateWormholeDesignation(std::mt19937& rng) const;

    /// Pick a random anomaly type.
    AnomalyType GetRandomAnomalyType(std::mt19937& rng) const;

    /// Generate an anomaly name.
    std::string GenerateAnomalyName(std::mt19937& rng, AnomalyType type) const;
};

} // namespace subspace
