#pragma once

#include "celestial/CelestialTypes.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class HomeSafetyMode {
    Safe,
    CosmeticThreats,
    RealThreats,
    Hardcore
};

enum class HomeBuildZoneType {
    PlanetSurface,
    MoonSurface,
    AsteroidSurface,
    OrbitalPlatform,
    SolarOrbit,
    GasGiantOrbit,
    DeepSpaceAnchor
};

enum class HomeStructureType {
    Unknown,
    LandingPad,
    Extractor,
    ConveyorHub,
    StorageDepot,
    Refinery,
    Assembler,
    PowerRelay,
    SolarCollector,
    DroneDepot,
    ResearchLab,
    ShipyardBay,
    DysonSwarmNode,
    OrbitalRingSegment,
    SubspaceAnchor
};

struct HomeWorldConfig {
    HomeSafetyMode safetyMode = HomeSafetyMode::Safe;
    bool persistHomeSystem = true;
    bool allowHomeRaids = false;
    bool allowHomeStructureDamage = false;
    bool allowOfflineProduction = false;
    int automationLimit = 12;
};

struct HomeBuildZone {
    std::string id;
    std::string displayName;
    HomeBuildZoneType type = HomeBuildZoneType::PlanetSurface;
    std::string parentBodyId;
    int gridWidth = 64;
    int gridHeight = 64;
    bool safeByDefault = true;
    std::vector<std::string> localResourceTags;
};

struct HomeStructure {
    std::string id;
    HomeStructureType type = HomeStructureType::Unknown;
    std::string zoneId;
    int x = 0;
    int y = 0;
    int tier = 1;
    bool powered = false;
    bool automated = false;
    float buildProgress = 1.0f;
};

struct HomeSolarSystemState {
    std::string id = "home-sol";
    std::uint32_t seed = 0x51B5ACEu;
    HomeWorldConfig config;
    StarSystemDefinition systemDefinition;
    std::vector<HomeBuildZone> buildZones;
    std::vector<HomeStructure> structures;
    int storedPower = 0;
    int automationBandwidthUsed = 0;
    int automationBandwidthCap = 12;
};

HomeSolarSystemState CreateDefaultHomeSolarSystem(std::uint32_t seed = 0x51B5ACEu);
HomeBuildZone CreateHomeBuildZone(const std::string& id,
                                  const std::string& displayName,
                                  HomeBuildZoneType type,
                                  const std::string& parentBodyId,
                                  int width,
                                  int height);
HomeStructure CreateHomeStructure(const std::string& id,
                                  HomeStructureType type,
                                  const std::string& zoneId,
                                  int x,
                                  int y,
                                  int tier = 1);
bool IsHomeSystemSafe(const HomeSolarSystemState& home);
int CountHomeStructures(const HomeSolarSystemState& home, HomeStructureType type);
int EstimateHomePowerGeneration(const HomeSolarSystemState& home);
int EstimateHomeAutomationCapacity(const HomeSolarSystemState& home);
std::string HomeSafetyModeName(HomeSafetyMode mode);
std::string HomeBuildZoneTypeName(HomeBuildZoneType type);
std::string HomeStructureTypeName(HomeStructureType type);
std::string HomeSolarSystemSummary(const HomeSolarSystemState& home);

} // namespace subspace
