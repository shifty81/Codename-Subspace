#include "procedural/GalaxyGenerator.h"

#include <chrono>

namespace subspace {

// ---------------------------------------------------------------------------
// Sector generation constants
// ---------------------------------------------------------------------------
static constexpr float kSectorPositionRange  = 5000.0f;   // asteroid position spread
static constexpr float kAsteroidMinSize      = 10.0f;
static constexpr float kAsteroidMaxSize      = 60.0f;
static constexpr int   kWormholeMinClass     = 1;
static constexpr int   kWormholeMaxClass     = 6;
static constexpr int   kWormholeDestRange    = 500;       // ±sectors for destination
static constexpr float kAnomalyPositionRange = 400.0f;
static constexpr float kAnomalyMinRadius     = 30.0f;
static constexpr float kAnomalyMaxRadius     = 150.0f;
static constexpr float kAnomalyMinIntensity  = 0.3f;
static constexpr float kAnomalyMaxIntensity  = 1.5f;

GalaxyGenerator::GalaxyGenerator(int seed)
    : _seed(seed != 0
                ? seed
                : static_cast<int>(
                      std::chrono::steady_clock::now().time_since_epoch().count() & 0x7FFFFFFF)) {}

int GalaxyGenerator::HashCoordinates(int x, int y, int z) const {
    // Same hash as C# GalaxyGenerator.HashCoordinates (unchecked multiply-xor).
    int hash = _seed;
    hash = hash * 397 ^ x;
    hash = hash * 397 ^ y;
    hash = hash * 397 ^ z;
    return hash;
}

ResourceType GalaxyGenerator::GetRandomResourceType(std::mt19937& rng) const {
    static const ResourceType types[] = {
        ResourceType::Iron,
        ResourceType::Titanium,
        ResourceType::Naonite,
        ResourceType::Trinium,
        ResourceType::Xanion,
        ResourceType::Ogonite,
        ResourceType::Avorion
    };
    std::uniform_int_distribution<int> dist(0, 6);
    return types[dist(rng)];
}

std::string GalaxyGenerator::GetRandomStationType(std::mt19937& rng) const {
    static const char* types[] = {
        "Trading", "Military", "Mining", "Shipyard", "Research", "Refinery"
    };
    std::uniform_int_distribution<int> dist(0, 5);
    return types[dist(rng)];
}

std::string GalaxyGenerator::GenerateStationName(std::mt19937& rng) const {
    static const char* prefixes[] = {
        "Alpha", "Beta", "Gamma", "Delta", "Epsilon",
        "Zeta", "Sigma", "Omega", "Nova", "Stellar"
    };
    static const char* suffixes[] = {
        "Outpost", "Station", "Base", "Hub",
        "Terminal", "Complex", "Nexus", "Citadel"
    };
    std::uniform_int_distribution<int> prefDist(0, 9);
    std::uniform_int_distribution<int> sufDist(0, 7);
    return std::string(prefixes[prefDist(rng)]) + " " + suffixes[sufDist(rng)];
}

std::string GalaxyGenerator::GenerateWormholeDesignation(std::mt19937& rng) const {
    std::uniform_int_distribution<int> letterDist(0, 25);
    std::uniform_int_distribution<int> numberDist(100, 999);
    char letter = static_cast<char>('A' + letterDist(rng));
    return std::string(1, letter) + std::to_string(numberDist(rng));
}

GalaxySector GalaxyGenerator::GenerateSector(int x, int y, int z) const {
    int sectorSeed = HashCoordinates(x, y, z);
    std::mt19937 rng(static_cast<unsigned>(sectorSeed));

    GalaxySector sector(x, y, z);

    // --- Asteroids ---
    std::uniform_int_distribution<int> asteroidCountDist(minAsteroids, maxAsteroids);
    int asteroidCount = asteroidCountDist(rng);

    std::uniform_real_distribution<float> posDist(-kSectorPositionRange, kSectorPositionRange);
    std::uniform_real_distribution<float> sizeDist(kAsteroidMinSize, kAsteroidMaxSize);

    for (int i = 0; i < asteroidCount; ++i) {
        AsteroidData ad;
        ad.position = {posDist(rng), posDist(rng), posDist(rng)};
        ad.size = sizeDist(rng);
        ad.resourceType = GetRandomResourceType(rng);
        sector.asteroids.push_back(ad);
    }

    // --- Station (probability-based) ---
    std::uniform_real_distribution<float> prob(0.0f, 1.0f);
    if (prob(rng) < stationProbability) {
        sector.hasStation = true;
        sector.station.position = {0.0f, 0.0f, 0.0f};
        sector.station.stationType = GetRandomStationType(rng);
        sector.station.name = GenerateStationName(rng);
    }

    // --- Wormholes (probability-based) ---
    if (prob(rng) < wormholeProbability) {
        std::uniform_int_distribution<int> classDist(kWormholeMinClass, kWormholeMaxClass);
        std::uniform_int_distribution<int> destDist(-kWormholeDestRange, kWormholeDestRange);

        WormholeData wh;
        wh.position = {posDist(rng), posDist(rng), posDist(rng)};
        wh.designation = GenerateWormholeDesignation(rng);
        wh.wormholeClass = classDist(rng);
        wh.type = "Wandering";
        wh.destinationSector = {
            static_cast<float>(destDist(rng)),
            static_cast<float>(destDist(rng)),
            static_cast<float>(destDist(rng))
        };
        sector.wormholes.push_back(wh);
    }

    // --- Anomaly generation ------------------------------------------------
    std::uniform_real_distribution<float> anomalyDist(0.0f, 1.0f);
    if (anomalyDist(rng) < anomalyProbability) {
        int numAnomalies = 1 + static_cast<int>(anomalyDist(rng) * 2.0f); // 1-2 anomalies
        std::uniform_real_distribution<float> anomalyPosDist(-kAnomalyPositionRange, kAnomalyPositionRange);
        std::uniform_real_distribution<float> radiusDist(kAnomalyMinRadius, kAnomalyMaxRadius);
        std::uniform_real_distribution<float> intensityDist(kAnomalyMinIntensity, kAnomalyMaxIntensity);
        for (int i = 0; i < numAnomalies; ++i) {
            AnomalyData anomaly;
            anomaly.position = { anomalyPosDist(rng), anomalyPosDist(rng), anomalyPosDist(rng) };
            anomaly.type = GetRandomAnomalyType(rng);
            anomaly.radius = radiusDist(rng);
            anomaly.intensity = intensityDist(rng);
            anomaly.name = GenerateAnomalyName(rng, anomaly.type);
            sector.anomalies.push_back(anomaly);
        }
    }

    return sector;
}

AnomalyType GalaxyGenerator::GetRandomAnomalyType(std::mt19937& rng) const {
    std::uniform_int_distribution<int> dist(0, 4);
    switch (dist(rng)) {
        case 0: return AnomalyType::Nebula;
        case 1: return AnomalyType::BlackHole;
        case 2: return AnomalyType::RadiationZone;
        case 3: return AnomalyType::IonStorm;
        case 4: return AnomalyType::GravityWell;
        default: return AnomalyType::Nebula;
    }
}

std::string GalaxyGenerator::GenerateAnomalyName(std::mt19937& rng, AnomalyType type) const {
    static const std::vector<std::string> prefixes = {
        "Alpha", "Beta", "Gamma", "Delta", "Epsilon",
        "Omega", "Sigma", "Theta", "Lambda", "Zeta"
    };
    static const std::vector<std::string> nebulaNames = {
        "Nebula", "Cloud", "Veil", "Mist", "Haze"
    };
    static const std::vector<std::string> blackHoleNames = {
        "Singularity", "Void", "Abyss", "Maw", "Rift"
    };
    static const std::vector<std::string> radiationNames = {
        "Radiation Zone", "Hot Zone", "Fallout", "Exposure", "Flux"
    };
    static const std::vector<std::string> stormNames = {
        "Ion Storm", "Tempest", "Maelstrom", "Squall", "Surge"
    };
    static const std::vector<std::string> gravityNames = {
        "Gravity Well", "Anomaly", "Distortion", "Warp", "Sink"
    };

    std::uniform_int_distribution<int> prefixDist(0, static_cast<int>(prefixes.size()) - 1);
    const std::string& prefix = prefixes[prefixDist(rng)];

    const std::vector<std::string>* suffixes = nullptr;
    switch (type) {
        case AnomalyType::Nebula:        suffixes = &nebulaNames; break;
        case AnomalyType::BlackHole:     suffixes = &blackHoleNames; break;
        case AnomalyType::RadiationZone: suffixes = &radiationNames; break;
        case AnomalyType::IonStorm:      suffixes = &stormNames; break;
        case AnomalyType::GravityWell:   suffixes = &gravityNames; break;
    }

    std::uniform_int_distribution<int> suffixDist(0, static_cast<int>(suffixes->size()) - 1);
    return prefix + " " + (*suffixes)[suffixDist(rng)];
}

} // namespace subspace
