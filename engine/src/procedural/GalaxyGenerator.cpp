#include "procedural/GalaxyGenerator.h"

#include <chrono>

namespace subspace {

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

    std::uniform_real_distribution<float> posDist(-5000.0f, 5000.0f);
    std::uniform_real_distribution<float> sizeDist(10.0f, 60.0f);

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
        std::uniform_int_distribution<int> classDist(1, 6);
        std::uniform_int_distribution<int> destDist(-500, 500);

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

    return sector;
}

} // namespace subspace
