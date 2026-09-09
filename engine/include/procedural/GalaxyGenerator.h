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
    // Pass496: every materialized rock belongs to a certified local mining
    // region/belt rather than existing as a free-floating system-origin spawn.
    std::string regionId;
};

enum class AsteroidBeltClass { Circumstellar, PlanetaryDebris };

/// Macro-scale asteroid/debris belt authority. Belts are represented as
/// streamed regions rather than materializing thousands of rocks at system
/// load. PlanetaryDebris belts remain parent-relative to gas giants.
struct AsteroidBeltData {
    std::string beltId;
    std::string name;
    AsteroidBeltClass beltClass = AsteroidBeltClass::Circumstellar;
    std::string parentPlanetId;
    float orbitalRadius = 0.0f;
    float innerRadius = 0.0f;
    float outerRadius = 0.0f;
    float orbitalPhaseRadians = 0.0f;
    float resourceRichness = 0.5f;
    float density = 0.5f;
    int localAsteroidBudget = 36;
};

/// Data for a station within a sector.
struct StationData {
    SectorPosition position;
    std::string stationType = "Trading";
    std::string name = "Unknown Station";
    std::string stationId;
    std::string parentObjectId;
    std::string placementClass = "UNASSIGNED";
};

/// Data for a ship within a sector.
struct ShipData {
    std::string shipId;
    std::uint32_t visualSeed = 1;
    SectorPosition position;
    std::string shipType = "Fighter";
    std::string faction = "Neutral";
    float heading = 0.0f;
    bool hostile = false;
    std::string authoredShipId;
    bool capturable = false;
    bool disabled = false;
    bool claimed = false;
    std::string parentObjectId;
    std::string placementClass = "UNASSIGNED";
};

/// Salvageable generated wreck. This replaces the useful procedural intent
/// from the legacy C# derelict/encounter experiments without preserving CLR
/// runtime ownership.
struct DerelictData {
    std::string derelictId;
    SectorPosition position;
    float size = 75.0f;
    float salvageValue = 1000.0f;
    float danger = 0.0f;
    float orientation = 0.0f;
    std::string parentObjectId;
};

struct DebrisFieldData {
    SectorPosition position;
    float radius = 350.0f;
    float density = 0.5f;
    std::string parentObjectId;
};

enum class EncounterArchetype { Patrol, Trader, PirateRaiders, IndustrialConvoy, SalvageClaim };

struct EncounterData {
    std::string encounterId;
    EncounterArchetype archetype = EncounterArchetype::Patrol;
    SectorPosition position;
    int shipCount = 1;
    float strength = 1.0f;
    bool hostile = false;
    std::string parentObjectId;
};

struct OrbitalHubData {
    std::string hubId;
    std::string planetId;
    SectorPosition position;
    bool automatedLogistics = false;
};

/// Planet types used by the strategic sector generator. Every type is
/// eventually harvestable; gas giants use atmospheric collector-ring
/// infrastructure instead of surface hex construction.
enum class PlanetType { Rocky, Desert, Ice, Oceanic, Volcanic, Barren, GasGiant };

enum class PlanetIndustryRepresentation { SurfaceHexGrid, AtmosphericCollectorRing };

/// High-level sector purpose. Event stages can still contain decorative
/// celestials, but do not expose the normal industrial surface loop.
enum class SectorPurpose { OpenSpace, EventStage, PlanetaryIndustry };


struct MoonData {
    std::string moonId;
    std::string name;
    std::string parentPlanetId;
    SectorPosition position;
    float radius = 48.0f;
    float orbitalRadius = 0.0f;
    float orbitalPhaseRadians = 0.0f;
    int surfaceSeed = 0;
    float resourceRichness = 0.35f;
    float hazardLevel = 0.15f;
};
struct PlanetData {
    std::string planetId;
    std::string name;
    SectorPosition position;
    float radius = 250.0f;
    PlanetType type = PlanetType::Rocky;
    bool landable = false; // legacy compatibility: Subspace ships do not land on planets.
    bool surveyable = true;
    bool elevatorCandidate = false;
    bool hasRings = false;
    bool supportsIndustry = false; // presently eligible for industrial deployment at current tech.
    bool harvestable = true;       // all planet classes have an eventual extraction path.
    PlanetIndustryRepresentation industryRepresentation = PlanetIndustryRepresentation::SurfaceHexGrid;
    bool orbitalHubBuilt = false;
    int surfaceSeed = 0;
    float resourceRichness = 0.5f;
    float hazardLevel = 0.2f;
};

/// Every normal sector has one solar-light authority. Star position is part of
/// generated sector data; presentation may render a corona at visual depth but
/// gameplay remains on X/Y.
enum class StarClass { RedDwarf, Orange, Yellow, White, BlueWhite };

struct StarData {
    std::string starId;
    std::string name;
    SectorPosition position;
    StarClass starClass = StarClass::Yellow;
    float radius = 1200.0f;
    float luminosity = 1.0f;
    float colorR = 1.0f;
    float colorG = 0.86f;
    float colorB = 0.62f;
};

enum class SectorSiteType {
    MiningField,
    SalvageSite,
    DerelictYard,
    OrbitalRelay,
    ResearchOutpost,
    TradeLane,
    DistressWreck,
    AnomalyBeacon,
    IndustrialDepot
};

struct SectorSiteData {
    std::string siteId;
    std::string name;
    SectorSiteType type = SectorSiteType::MiningField;
    SectorPosition position;
    float radius = 300.0f;
    float resourceRichness = 0.5f;
    float salvageValue = 0.0f;
    float danger = 0.2f;
    bool discovered = false;
    std::string parentObjectId;
    std::string placementClass = "UNASSIGNED";
};

/// Data for a wormhole connection within a sector.
struct WormholeData {
    SectorPosition position;
    std::string designation = "Unknown";
    int wormholeClass = 1;     // 1-6
    std::string type = "Wandering";
    SectorPosition destinationSector;
    std::string parentObjectId;
    std::string placementClass = "UNASSIGNED";
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
    std::string parentObjectId;
    std::string placementClass = "UNASSIGNED";
};

/// Represents a generated galaxy sector.
struct GalaxySector {
    int x = 0;
    int y = 0;
    int z = 0;
    std::vector<AsteroidData> asteroids;
    std::vector<AsteroidBeltData> asteroidBelts;
    std::vector<ShipData> ships;
    std::vector<WormholeData> wormholes;
    bool hasStation = false;
    StationData station;
    std::vector<AnomalyData> anomalies;
    std::vector<PlanetData> planets;
    std::vector<MoonData> moons;
    std::vector<DerelictData> derelicts;
    std::vector<DebrisFieldData> debrisFields;
    std::vector<EncounterData> encounters;
    std::vector<OrbitalHubData> orbitalHubs;
    StarData star;
    bool hasStar = true;
    std::vector<SectorSiteData> pointsOfInterest;
    SectorPurpose purpose = SectorPurpose::OpenSpace;

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

    /// Probability that a normal sector contains one or more planets.
    float planetProbability = 0.78f;

    /// Probability a sector is generated as a focused event stage instead of
    /// exposing the normal landable-planet industry loop.
    float eventStageProbability = 0.12f;

    /// Maximum generated planets in one sector.
    int maxPlanets = 6;

    /// Native traffic/encounter generation controls (Pass180).
    int minAmbientShips = 2;
    int maxAmbientShips = 6;
    float derelictProbability = 0.28f;
    float debrisFieldProbability = 0.45f;
    float orbitalHubProbability = 0.38f;

    /// Pass202/203 system-composition controls. Each generated sector owns a
    /// star and a minimum activity budget even when it has few planets.
    int minPointsOfInterest = 3;
    int maxPointsOfInterest = 7;
    int preferredMinPlanets = 2;

    /// Min/max immediately materialized asteroid count per local region.
    int minAsteroids = 5;
    int maxAsteroids = 20;

    /// Pass410R3 system regeneration: most normal systems own a true
    /// circumstellar belt and most gas giants own a parent-relative debris
    /// belt. These are macro regions and do not inflate initial rock counts.
    float asteroidBeltProbability = 0.95f;
    float gasGiantBeltProbability = 1.00f;

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

    PlanetType GetRandomPlanetType(std::mt19937& rng) const;
    std::string GeneratePlanetName(std::mt19937& rng, int index) const;
};

} // namespace subspace
