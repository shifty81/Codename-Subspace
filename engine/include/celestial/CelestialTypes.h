#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class CelestialBodyType {
    Unknown,
    Star,
    RockyPlanet,
    DryTerranPlanet,
    TerranPlanet,
    RiverWorld,
    IceWorld,
    LavaWorld,
    GasGiant,
    RingedGasGiant,
    AsteroidBelt,
    BlackHole,
    GalaxyBackdrop
};

enum class CelestialOrbitRole {
    Primary,
    InnerSystem,
    HabitableBand,
    OuterSystem,
    Belt,
    DeepSpace
};

struct CelestialPalette {
    std::uint32_t primary = 0xFFFFFF;
    std::uint32_t secondary = 0x8090A0;
    std::uint32_t accent = 0xFFD166;
    std::uint32_t atmosphere = 0x6EDBFF;
    std::uint32_t shadow = 0x121A24;
};

struct CelestialBodyDefinition {
    std::string id;
    std::string displayName;
    CelestialBodyType type = CelestialBodyType::Unknown;
    CelestialOrbitRole orbitRole = CelestialOrbitRole::DeepSpace;
    std::uint32_t seed = 0;
    float orbitRadius = 0.0f;
    float orbitAngleRadians = 0.0f;
    float visualRadius = 24.0f;
    bool hasAtmosphere = false;
    bool hasRings = false;
    bool hasClouds = false;
    bool isHazardous = false;
    int resourceRichness = 0;
    std::vector<std::string> resourceTags;
    std::vector<std::string> gameplayTags;
    CelestialPalette palette;
};

struct StarSystemDefinition {
    std::string id;
    std::string displayName;
    std::uint32_t seed = 0;
    CelestialBodyDefinition primary;
    std::vector<CelestialBodyDefinition> bodies;
    std::vector<std::string> tags;
};

std::string CelestialBodyTypeName(CelestialBodyType type);
std::string CelestialOrbitRoleName(CelestialOrbitRole role);
std::string CelestialBodySummary(const CelestialBodyDefinition& body);
std::string StarSystemSummary(const StarSystemDefinition& system);

} // namespace subspace
