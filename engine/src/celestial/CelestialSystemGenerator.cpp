#include "celestial/CelestialSystemGenerator.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {
namespace {

constexpr float kPi = 3.14159265358979323846f;

std::string BodySuffix(CelestialBodyType type) {
    switch (type) {
        case CelestialBodyType::RockyPlanet: return "Rock";
        case CelestialBodyType::DryTerranPlanet: return "Dry Terran";
        case CelestialBodyType::TerranPlanet: return "Terran";
        case CelestialBodyType::RiverWorld: return "River World";
        case CelestialBodyType::IceWorld: return "Ice World";
        case CelestialBodyType::LavaWorld: return "Lava World";
        case CelestialBodyType::GasGiant: return "Gas Giant";
        case CelestialBodyType::RingedGasGiant: return "Ringed Giant";
        case CelestialBodyType::AsteroidBelt: return "Belt";
        case CelestialBodyType::BlackHole: return "Black Hole";
        default: return "Body";
    }
}

CelestialPalette PaletteForType(CelestialBodyType type, std::uint32_t seed) {
    const float variation = CelestialSeededUnit(seed, 91);
    CelestialPalette palette;
    switch (type) {
        case CelestialBodyType::Star:
            palette.primary = variation > 0.65f ? 0xBBD7FF : (variation > 0.3f ? 0xFFE082 : 0xFF9A45);
            palette.secondary = 0xFFF2AA;
            palette.accent = 0xFF6B35;
            palette.atmosphere = 0xFFE8A3;
            palette.shadow = 0x3B230C;
            break;
        case CelestialBodyType::TerranPlanet:
        case CelestialBodyType::RiverWorld:
            palette.primary = 0x2C8F5B;
            palette.secondary = 0x2A5D9F;
            palette.accent = 0xC8B56A;
            palette.atmosphere = 0x9FE8FF;
            palette.shadow = 0x123245;
            break;
        case CelestialBodyType::DryTerranPlanet:
            palette.primary = 0xA77D44;
            palette.secondary = 0x5E7D4E;
            palette.accent = 0xCBA15B;
            palette.atmosphere = 0xD6C38B;
            palette.shadow = 0x2C251B;
            break;
        case CelestialBodyType::IceWorld:
            palette.primary = 0xC8E7FF;
            palette.secondary = 0x7BA6D8;
            palette.accent = 0xF5FCFF;
            palette.atmosphere = 0xDAF7FF;
            palette.shadow = 0x2E4A69;
            break;
        case CelestialBodyType::LavaWorld:
            palette.primary = 0x3A2420;
            palette.secondary = 0xA53A22;
            palette.accent = 0xFFB347;
            palette.atmosphere = 0xD84B2A;
            palette.shadow = 0x130A08;
            break;
        case CelestialBodyType::GasGiant:
        case CelestialBodyType::RingedGasGiant:
            palette.primary = 0xD6A66F;
            palette.secondary = 0x875C92;
            palette.accent = 0xF2D7A6;
            palette.atmosphere = 0xE9B884;
            palette.shadow = 0x382442;
            break;
        case CelestialBodyType::AsteroidBelt:
            palette.primary = 0x625B52;
            palette.secondary = 0x3E3A35;
            palette.accent = 0x8F8378;
            palette.shadow = 0x1A1D22;
            break;
        case CelestialBodyType::BlackHole:
            palette.primary = 0x0B0E18;
            palette.secondary = 0x3E4C89;
            palette.accent = 0xD9B4FF;
            palette.atmosphere = 0x6B55C8;
            palette.shadow = 0x02030A;
            break;
        default:
            palette.primary = 0x8899AA;
            palette.secondary = 0x667788;
            palette.accent = 0xAABBCC;
            break;
    }
    return palette;
}

void AddTags(CelestialBodyDefinition& body) {
    body.gameplayTags.push_back(CelestialBodyTypeName(body.type));
    body.gameplayTags.push_back(CelestialOrbitRoleName(body.orbitRole));

    switch (body.type) {
        case CelestialBodyType::RockyPlanet:
            body.resourceTags = {"ore", "silicates", "rare-metals"};
            body.resourceRichness = 4;
            break;
        case CelestialBodyType::DryTerranPlanet:
            body.resourceTags = {"organics", "silicates", "surface-salvage"};
            body.resourceRichness = 5;
            body.hasAtmosphere = true;
            break;
        case CelestialBodyType::TerranPlanet:
            body.resourceTags = {"water", "organics", "settlements"};
            body.resourceRichness = 6;
            body.hasAtmosphere = true;
            body.hasClouds = true;
            break;
        case CelestialBodyType::RiverWorld:
            body.resourceTags = {"water", "organics", "river-deltas", "settlements"};
            body.resourceRichness = 7;
            body.hasAtmosphere = true;
            body.hasClouds = true;
            break;
        case CelestialBodyType::IceWorld:
            body.resourceTags = {"ice", "volatiles", "cryo-minerals"};
            body.resourceRichness = 5;
            body.hasAtmosphere = true;
            break;
        case CelestialBodyType::LavaWorld:
            body.resourceTags = {"rare-metals", "thermal", "hazard-mining"};
            body.resourceRichness = 8;
            body.isHazardous = true;
            break;
        case CelestialBodyType::GasGiant:
        case CelestialBodyType::RingedGasGiant:
            body.resourceTags = {"hydrogen", "helium", "fuel", "storm-harvest"};
            body.resourceRichness = 7;
            body.hasAtmosphere = true;
            body.isHazardous = true;
            break;
        case CelestialBodyType::AsteroidBelt:
            body.resourceTags = {"ore", "salvage", "pirate-risk", "hidden-sites"};
            body.resourceRichness = 8;
            break;
        case CelestialBodyType::BlackHole:
            body.resourceTags = {"gravity-hazard", "research", "rare-anomaly"};
            body.resourceRichness = 9;
            body.isHazardous = true;
            break;
        default:
            body.resourceTags = {"survey"};
            body.resourceRichness = 1;
            break;
    }
}

} // namespace

float CelestialSeededUnit(std::uint32_t seed, int index) {
    std::uint32_t value = seed + static_cast<std::uint32_t>(index) * 747796405u + 2891336453u;
    value = ((value >> ((value >> 28u) + 4u)) ^ value) * 277803737u;
    value = (value >> 22u) ^ value;
    return static_cast<float>(value & 0xffffu) / 65535.0f;
}

std::uint32_t CelestialHashSeed(const std::string& text, std::uint32_t salt) {
    std::uint32_t hash = 2166136261u ^ salt;
    for (unsigned char c : text) {
        hash ^= static_cast<std::uint32_t>(c);
        hash *= 16777619u;
    }
    return hash;
}

CelestialSystemGenerator::CelestialSystemGenerator(CelestialSystemGeneratorOptions options)
    : options_(options) {}

StarSystemDefinition CelestialSystemGenerator::GenerateSystem(const std::string& sectorId, std::uint32_t seed) const {
    StarSystemDefinition system;
    system.id = sectorId;
    system.displayName = "Sector " + sectorId + " Solar System";
    system.seed = seed == 0 ? CelestialHashSeed(sectorId, 0x53554253u) : seed;
    system.tags = {"sector-system", "procedural", "celestial"};

    system.primary.id = sectorId + ":primary";
    system.primary.displayName = sectorId + " Primary";
    system.primary.type = (options_.allowBlackHolePrimary && CelestialSeededUnit(system.seed, 2) > 0.94f)
        ? CelestialBodyType::BlackHole
        : CelestialBodyType::Star;
    system.primary.orbitRole = CelestialOrbitRole::Primary;
    system.primary.seed = CelestialHashSeed(system.primary.id, system.seed);
    system.primary.visualRadius = system.primary.type == CelestialBodyType::BlackHole ? 80.0f : 96.0f;
    system.primary.palette = PaletteForType(system.primary.type, system.primary.seed);
    AddTags(system.primary);

    const int minPlanets = std::max(0, options_.minPlanets);
    const int maxPlanets = std::max(minPlanets, options_.maxPlanets);
    const int span = std::max(1, maxPlanets - minPlanets + 1);
    const int planetCount = minPlanets + static_cast<int>(CelestialSeededUnit(system.seed, 8) * static_cast<float>(span)) % span;

    for (int i = 0; i < planetCount; ++i) {
        CelestialOrbitRole role = CelestialOrbitRole::OuterSystem;
        if (i <= 1) {
            role = CelestialOrbitRole::InnerSystem;
        } else if (i <= 3) {
            role = CelestialOrbitRole::HabitableBand;
        }
        system.bodies.push_back(GenerateBody(system.id, i, role, CelestialHashSeed(system.id, system.seed + static_cast<std::uint32_t>(i * 31))));
    }

    if (options_.includeAsteroidBelt) {
        CelestialBodyDefinition belt = GenerateBody(system.id, planetCount, CelestialOrbitRole::Belt, CelestialHashSeed(system.id + ":belt", system.seed));
        belt.type = CelestialBodyType::AsteroidBelt;
        belt.displayName = sectorId + " Belt";
        belt.visualRadius = 34.0f;
        belt.palette = PaletteForType(belt.type, belt.seed);
        belt.resourceTags.clear();
        belt.gameplayTags.clear();
        AddTags(belt);
        system.bodies.push_back(belt);
    }

    return system;
}

CelestialBodyDefinition CelestialSystemGenerator::GenerateBody(const std::string& systemId,
                                                               int orbitIndex,
                                                               CelestialOrbitRole role,
                                                               std::uint32_t seed) const {
    const float roll = CelestialSeededUnit(seed, orbitIndex + 10);
    CelestialBodyType type = CelestialBodyType::RockyPlanet;

    if (role == CelestialOrbitRole::InnerSystem) {
        type = roll > 0.68f ? CelestialBodyType::LavaWorld : (roll > 0.36f ? CelestialBodyType::DryTerranPlanet : CelestialBodyType::RockyPlanet);
    } else if (role == CelestialOrbitRole::HabitableBand) {
        if (roll > 0.76f) type = CelestialBodyType::RiverWorld;
        else if (roll > 0.46f) type = CelestialBodyType::TerranPlanet;
        else if (roll > 0.22f) type = CelestialBodyType::DryTerranPlanet;
        else type = CelestialBodyType::RockyPlanet;
    } else if (role == CelestialOrbitRole::Belt) {
        type = CelestialBodyType::AsteroidBelt;
    } else {
        if (roll > 0.72f) type = CelestialBodyType::RingedGasGiant;
        else if (roll > 0.42f) type = CelestialBodyType::GasGiant;
        else if (roll > 0.18f) type = CelestialBodyType::IceWorld;
        else type = CelestialBodyType::RockyPlanet;
    }

    CelestialBodyDefinition body;
    body.id = systemId + ":body-" + std::to_string(orbitIndex);
    body.type = type;
    body.orbitRole = role;
    body.seed = seed;
    body.orbitRadius = 260.0f + static_cast<float>(orbitIndex) * 170.0f + CelestialSeededUnit(seed, 50) * 80.0f;
    body.orbitAngleRadians = CelestialSeededUnit(seed, 51) * kPi * 2.0f;
    body.visualRadius = 28.0f + CelestialSeededUnit(seed, 52) * 34.0f;
    if (type == CelestialBodyType::GasGiant || type == CelestialBodyType::RingedGasGiant) {
        body.visualRadius += 28.0f;
    }
    body.hasRings = type == CelestialBodyType::RingedGasGiant || (type == CelestialBodyType::GasGiant && CelestialSeededUnit(seed, 53) > 0.75f);
    body.palette = PaletteForType(type, seed);
    body.displayName = "Orbit " + std::to_string(orbitIndex + 1) + " " + BodySuffix(type);
    if (options_.includeGameplayTags) {
        AddTags(body);
    }
    return body;
}

} // namespace subspace
