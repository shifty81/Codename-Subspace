#include "home/HomeSolarSystem.h"

#include <algorithm>
#include <sstream>

namespace subspace {
namespace {
CelestialBodyDefinition MakeHomeBody(const std::string& id,
                                     const std::string& name,
                                     CelestialBodyType type,
                                     CelestialOrbitRole role,
                                     std::uint32_t seed,
                                     float orbit,
                                     float radius,
                                     std::initializer_list<std::string> resources) {
    CelestialBodyDefinition body;
    body.id = id;
    body.displayName = name;
    body.type = type;
    body.orbitRole = role;
    body.seed = seed;
    body.orbitRadius = orbit;
    body.visualRadius = radius;
    body.resourceRichness = 3;
    body.resourceTags.assign(resources.begin(), resources.end());
    body.gameplayTags.push_back("home-system");
    body.isHazardous = false;
    return body;
}
}

HomeBuildZone CreateHomeBuildZone(const std::string& id,
                                  const std::string& displayName,
                                  HomeBuildZoneType type,
                                  const std::string& parentBodyId,
                                  int width,
                                  int height) {
    HomeBuildZone zone;
    zone.id = id;
    zone.displayName = displayName;
    zone.type = type;
    zone.parentBodyId = parentBodyId;
    zone.gridWidth = std::max(8, width);
    zone.gridHeight = std::max(8, height);
    zone.safeByDefault = true;
    return zone;
}

HomeStructure CreateHomeStructure(const std::string& id,
                                  HomeStructureType type,
                                  const std::string& zoneId,
                                  int x,
                                  int y,
                                  int tier) {
    HomeStructure structure;
    structure.id = id;
    structure.type = type;
    structure.zoneId = zoneId;
    structure.x = x;
    structure.y = y;
    structure.tier = std::max(1, tier);
    structure.powered = type == HomeStructureType::SolarCollector || type == HomeStructureType::PowerRelay || type == HomeStructureType::LandingPad;
    structure.automated = type == HomeStructureType::DroneDepot || type == HomeStructureType::Extractor || type == HomeStructureType::Refinery;
    return structure;
}

HomeSolarSystemState CreateDefaultHomeSolarSystem(std::uint32_t seed) {
    HomeSolarSystemState home;
    home.seed = seed;
    home.config = HomeWorldConfig{};

    home.systemDefinition.id = "home-sol";
    home.systemDefinition.displayName = "Home Solar System";
    home.systemDefinition.seed = seed;
    home.systemDefinition.tags = {"home", "safe", "persistent", "buildable"};
    home.systemDefinition.primary = MakeHomeBody("home-star", "Home Star", CelestialBodyType::Star, CelestialOrbitRole::Primary, seed, 0.0f, 68.0f, {"solar-energy"});
    home.systemDefinition.primary.gameplayTags.push_back("stellar-power-source");
    home.systemDefinition.primary.resourceRichness = 5;

    home.systemDefinition.bodies.push_back(MakeHomeBody("home-world", "Foundry World", CelestialBodyType::TerranPlanet, CelestialOrbitRole::HabitableBand, seed + 11, 760.0f, 42.0f, {"iron", "copper", "silicates", "water"}));
    home.systemDefinition.bodies.push_back(MakeHomeBody("home-moon", "Cinder Moon", CelestialBodyType::RockyPlanet, CelestialOrbitRole::InnerSystem, seed + 17, 1010.0f, 26.0f, {"titanium", "carbon", "ice"}));
    home.systemDefinition.bodies.push_back(MakeHomeBody("home-belt", "Starter Belt", CelestialBodyType::AsteroidBelt, CelestialOrbitRole::Belt, seed + 29, 1420.0f, 90.0f, {"ore", "scrap", "ice", "silicates"}));

    HomeBuildZone surface = CreateHomeBuildZone("home-world-surface-a", "Starter Surface Zone", HomeBuildZoneType::PlanetSurface, "home-world", 64, 64);
    surface.localResourceTags = {"iron", "copper", "silicates"};
    home.buildZones.push_back(surface);

    HomeBuildZone orbit = CreateHomeBuildZone("home-orbit-yard", "Orbital Shipyard Grid", HomeBuildZoneType::OrbitalPlatform, "home-world", 48, 48);
    orbit.localResourceTags = {"solar-energy", "logistics"};
    home.buildZones.push_back(orbit);

    HomeBuildZone solar = CreateHomeBuildZone("home-solar-orbit", "Solar Collector Orbit", HomeBuildZoneType::SolarOrbit, "home-star", 96, 16);
    solar.localResourceTags = {"solar-energy"};
    home.buildZones.push_back(solar);

    home.structures.push_back(CreateHomeStructure("landing-pad-01", HomeStructureType::LandingPad, "home-world-surface-a", 30, 30, 1));
    home.structures.push_back(CreateHomeStructure("extractor-iron-01", HomeStructureType::Extractor, "home-world-surface-a", 22, 28, 1));
    home.structures.push_back(CreateHomeStructure("refinery-01", HomeStructureType::Refinery, "home-world-surface-a", 34, 30, 1));
    home.structures.push_back(CreateHomeStructure("storage-01", HomeStructureType::StorageDepot, "home-world-surface-a", 38, 30, 1));
    home.structures.push_back(CreateHomeStructure("shipyard-bay-01", HomeStructureType::ShipyardBay, "home-orbit-yard", 24, 24, 1));
    home.structures.push_back(CreateHomeStructure("solar-collector-01", HomeStructureType::SolarCollector, "home-solar-orbit", 8, 8, 1));

    home.storedPower = 0;
    home.automationBandwidthCap = EstimateHomeAutomationCapacity(home);
    home.automationBandwidthUsed = 3;
    return home;
}

bool IsHomeSystemSafe(const HomeSolarSystemState& home) {
    return home.config.safetyMode == HomeSafetyMode::Safe && !home.config.allowHomeRaids && !home.config.allowHomeStructureDamage;
}

int CountHomeStructures(const HomeSolarSystemState& home, HomeStructureType type) {
    return static_cast<int>(std::count_if(home.structures.begin(), home.structures.end(), [type](const HomeStructure& structure) {
        return structure.type == type;
    }));
}

int EstimateHomePowerGeneration(const HomeSolarSystemState& home) {
    int power = 0;
    for (const auto& structure : home.structures) {
        switch (structure.type) {
            case HomeStructureType::SolarCollector: power += 100 * structure.tier; break;
            case HomeStructureType::DysonSwarmNode: power += 750 * structure.tier; break;
            case HomeStructureType::OrbitalRingSegment: power += 250 * structure.tier; break;
            case HomeStructureType::PowerRelay: power += 25 * structure.tier; break;
            default: break;
        }
    }
    return power;
}

int EstimateHomeAutomationCapacity(const HomeSolarSystemState& home) {
    int capacity = home.config.automationLimit;
    for (const auto& structure : home.structures) {
        if (structure.type == HomeStructureType::DroneDepot) capacity += 4 * structure.tier;
        if (structure.type == HomeStructureType::ResearchLab) capacity += structure.tier;
        if (structure.type == HomeStructureType::DysonSwarmNode) capacity += 2 * structure.tier;
    }
    return capacity;
}

std::string HomeSafetyModeName(HomeSafetyMode mode) {
    switch (mode) {
        case HomeSafetyMode::Safe: return "Safe";
        case HomeSafetyMode::CosmeticThreats: return "CosmeticThreats";
        case HomeSafetyMode::RealThreats: return "RealThreats";
        case HomeSafetyMode::Hardcore: return "Hardcore";
    }
    return "Unknown";
}

std::string HomeBuildZoneTypeName(HomeBuildZoneType type) {
    switch (type) {
        case HomeBuildZoneType::PlanetSurface: return "PlanetSurface";
        case HomeBuildZoneType::MoonSurface: return "MoonSurface";
        case HomeBuildZoneType::AsteroidSurface: return "AsteroidSurface";
        case HomeBuildZoneType::OrbitalPlatform: return "OrbitalPlatform";
        case HomeBuildZoneType::SolarOrbit: return "SolarOrbit";
        case HomeBuildZoneType::GasGiantOrbit: return "GasGiantOrbit";
        case HomeBuildZoneType::DeepSpaceAnchor: return "DeepSpaceAnchor";
    }
    return "Unknown";
}

std::string HomeStructureTypeName(HomeStructureType type) {
    switch (type) {
        case HomeStructureType::Unknown: return "Unknown";
        case HomeStructureType::LandingPad: return "LandingPad";
        case HomeStructureType::Extractor: return "Extractor";
        case HomeStructureType::ConveyorHub: return "ConveyorHub";
        case HomeStructureType::StorageDepot: return "StorageDepot";
        case HomeStructureType::Refinery: return "Refinery";
        case HomeStructureType::Assembler: return "Assembler";
        case HomeStructureType::PowerRelay: return "PowerRelay";
        case HomeStructureType::SolarCollector: return "SolarCollector";
        case HomeStructureType::DroneDepot: return "DroneDepot";
        case HomeStructureType::ResearchLab: return "ResearchLab";
        case HomeStructureType::ShipyardBay: return "ShipyardBay";
        case HomeStructureType::DysonSwarmNode: return "DysonSwarmNode";
        case HomeStructureType::OrbitalRingSegment: return "OrbitalRingSegment";
        case HomeStructureType::SubspaceAnchor: return "SubspaceAnchor";
    }
    return "Unknown";
}

std::string HomeSolarSystemSummary(const HomeSolarSystemState& home) {
    std::ostringstream out;
    out << home.systemDefinition.displayName
        << " zones=" << home.buildZones.size()
        << " structures=" << home.structures.size()
        << " safety=" << HomeSafetyModeName(home.config.safetyMode)
        << " power=" << EstimateHomePowerGeneration(home)
        << " automation=" << home.automationBandwidthUsed << "/" << EstimateHomeAutomationCapacity(home);
    return out.str();
}

} // namespace subspace
