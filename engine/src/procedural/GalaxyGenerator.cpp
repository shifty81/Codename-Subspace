#include "procedural/GalaxyGenerator.h"
#include "procedural/SolarSystemPlacementSystem.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// Sector generation constants
// ---------------------------------------------------------------------------
static constexpr float kSectorPositionRange  = 5000.0f;   // object position spread within sector
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
static constexpr float kPlanetPositionRange  = 3200.0f;
static constexpr float kPlanetMinRadius      = 240.0f;
static constexpr float kPlanetMaxRadius      = 980.0f;

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

    // --- Pass201 solar-light authority ------------------------------------
    // Every generated sector owns one star. The star is deterministic data,
    // not a renderer-created backdrop, so lighting, navigation and later
    // gameplay can all agree on the same solar source.
    std::uniform_int_distribution<int> starClassDist(0, 4);
    const int starRoll = starClassDist(rng);
    sector.hasStar = true;
    sector.star.starId = "star_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
    sector.star.name = "Subspace " + std::to_string(std::abs(x * 31 + y * 17 + z * 7) % 997) + " Primary";
    sector.star.position = {0.0f, 0.0f, 0.0f};
    sector.star.starClass = static_cast<StarClass>(starRoll);
    switch (sector.star.starClass) {
        case StarClass::RedDwarf:
            sector.star.radius=900.0f; sector.star.luminosity=0.62f; sector.star.colorR=1.0f; sector.star.colorG=0.38f; sector.star.colorB=0.24f; break;
        case StarClass::Orange:
            sector.star.radius=1080.0f; sector.star.luminosity=0.82f; sector.star.colorR=1.0f; sector.star.colorG=0.62f; sector.star.colorB=0.34f; break;
        case StarClass::Yellow:
            sector.star.radius=1240.0f; sector.star.luminosity=1.0f; sector.star.colorR=1.0f; sector.star.colorG=0.86f; sector.star.colorB=0.58f; break;
        case StarClass::White:
            sector.star.radius=1380.0f; sector.star.luminosity=1.20f; sector.star.colorR=0.88f; sector.star.colorG=0.92f; sector.star.colorB=1.0f; break;
        case StarClass::BlueWhite:
            sector.star.radius=1520.0f; sector.star.luminosity=1.42f; sector.star.colorR=0.62f; sector.star.colorG=0.78f; sector.star.colorB=1.0f; break;
    }

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
        std::uniform_real_distribution<float> stationAngle(0.0f, 6.28318530718f);
        std::uniform_real_distribution<float> stationRadius(2600.0f, 5200.0f);
        const float sa=stationAngle(rng), sr=stationRadius(rng);
        sector.station.position = {std::cos(sa)*sr, std::sin(sa)*sr, 0.0f};
        sector.station.stationType = GetRandomStationType(rng);
        sector.station.name = GenerateStationName(rng);
        sector.station.stationId = "station_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z) + "_primary";
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

    // --- Pass217+ orbital planetary regions -----------------------------------
    // Event-stage sectors intentionally do not expose the normal surface
    // industry loop. They may still render decorative planets later.
    bool isEventStage = prob(rng) < eventStageProbability;
    if (isEventStage) {
        sector.purpose = SectorPurpose::EventStage;
    } else if (prob(rng) < planetProbability) {
        const int cappedMax = std::max(1, maxPlanets);
        const int preferredMin = std::clamp(preferredMinPlanets, 1, cappedMax);
        std::uniform_int_distribution<int> countDist(preferredMin, cappedMax);
        std::uniform_real_distribution<float> orbitStartDist(180000.0f, 300000.0f);
        std::uniform_real_distribution<float> orbitStepDist(150000.0f, 260000.0f);
        std::uniform_real_distribution<float> orbitJitterDist(-22000.0f, 22000.0f);
        std::uniform_real_distribution<float> orbitPhaseDist(0.0f, 6.28318530718f);
        std::uniform_real_distribution<float> radiusDist(kPlanetMinRadius, kPlanetMaxRadius);
        std::uniform_real_distribution<float> richnessDist(0.20f, 1.0f);
        std::uniform_real_distribution<float> hazardDist(0.05f, 0.95f);
        const int count = countDist(rng);
        bool hasIndustrialCandidate = false;
        float nextOrbitRadius = orbitStartDist(rng);

        for (int i = 0; i < count; ++i) {
            PlanetData planet;
            planet.planetId = "planet_" + std::to_string(x) + "_" +
                              std::to_string(y) + "_" + std::to_string(z) + "_" +
                              std::to_string(i);
            planet.name = GeneratePlanetName(rng, i);
            const float phase = orbitPhaseDist(rng) + static_cast<float>(i) * (6.28318530718f / static_cast<float>(std::max(1, count)));
            const float orbitRadius = std::max(160000.0f, nextOrbitRadius + orbitJitterDist(rng));
            nextOrbitRadius += orbitStepDist(rng);
            planet.position = {sector.star.position.x + std::cos(phase) * orbitRadius,
                               sector.star.position.y + std::sin(phase) * orbitRadius, 0.0f};
            planet.radius = radiusDist(rng);
            planet.type = GetRandomPlanetType(rng);
            if (planet.type == PlanetType::GasGiant) planet.radius *= 1.85f;
            else if (planet.type == PlanetType::Oceanic || planet.type == PlanetType::Ice) planet.radius *= 1.18f;
            planet.resourceRichness = richnessDist(rng);
            planet.hazardLevel = hazardDist(rng);
            planet.surfaceSeed = HashCoordinates(x * 31 + i, y * 17 - i, z * 13 + i * 7);

            // Pass217+ correction: ships never land. Planetary access is
            // orbital survey -> tether seed -> space elevator -> Planetary
            // Manufacturing. Gas giants do not accept a conventional surface
            // elevator anchor; their future industry path is an orbital/
            // atmospheric collector ring and remains harvestable by contract.
            planet.landable = false;
            planet.surveyable = true;
            planet.harvestable = true;
            planet.industryRepresentation = planet.type == PlanetType::GasGiant
                ? PlanetIndustryRepresentation::AtmosphericCollectorRing
                : PlanetIndustryRepresentation::SurfaceHexGrid;
            planet.elevatorCandidate = planet.type != PlanetType::GasGiant &&
                                       planet.hazardLevel < 0.90f;
            planet.hasRings = (planet.type == PlanetType::GasGiant && prob(rng) > 0.28f) ||
                              (planet.type != PlanetType::GasGiant && prob(rng) > 0.92f);
            planet.supportsIndustry = planet.elevatorCandidate &&
                                      planet.resourceRichness >= 0.28f &&
                                      planet.hazardLevel <= 0.82f;
            planet.orbitalHubBuilt = false;
            hasIndustrialCandidate = hasIndustrialCandidate || planet.elevatorCandidate;
            sector.planets.push_back(std::move(planet));
        }

        sector.purpose = hasIndustrialCandidate ? SectorPurpose::PlanetaryIndustry
                                     : SectorPurpose::OpenSpace;

        // Pass227: normal flight represents one orbital neighborhood, not the
        // entire solar system. Existing stations in populated systems are
        // re-anchored into a stable orbit around a deterministic major planet
        // so undocking immediately reads as local orbital space while other
        // planets remain system-map destinations.
        if (sector.hasStation && !sector.planets.empty()) {
            const PlanetData* anchor = &sector.planets.front();
            for (const auto& candidate : sector.planets) {
                if (candidate.supportsIndustry) { anchor = &candidate; break; }
            }
            const float stationOrbit = anchor->radius * (anchor->hasRings ? 5.2f : 4.1f) + 1800.0f;
            const float stationPhase = orbitPhaseDist(rng);
            sector.station.position = {anchor->position.x + std::cos(stationPhase) * stationOrbit,
                                       anchor->position.y + std::sin(stationPhase) * stationOrbit,
                                       0.0f};
        }
    }

    // --- Pass496 first-class moon authority --------------------------------
    // Moons are generated as persistent system records instead of being only
    // renderer/map-derived decorations. The placement authority below expands
    // their parent-relative orbits beyond atmosphere/ring presentation safely.
    for (std::size_t pi=0; pi<sector.planets.size(); ++pi) {
        const auto& planet=sector.planets[pi];
        const std::uint32_t moonHash=static_cast<std::uint32_t>((planet.surfaceSeed*1664525u)+1013904223u+static_cast<unsigned>(pi)*97u);
        const int moonCount=(planet.type==PlanetType::GasGiant)?(1+int(moonHash%3u)):int((moonHash>>3)%3u);
        for(int mi=0;mi<moonCount;++mi){
            MoonData moon;
            moon.moonId=planet.planetId+"_moon_"+std::to_string(mi+1);
            moon.name=planet.name+" Moon "+std::to_string(mi+1);
            moon.parentPlanetId=planet.planetId;
            moon.radius=std::max(24.0f,planet.radius*(.10f+.035f*mi));
            moon.orbitalRadius=std::max(480.0f,planet.radius*(2.5f+1.35f*mi));
            moon.orbitalPhaseRadians=std::fmod((73.0f*mi+float((moonHash>>12)%360u))*3.14159265358979323846f/180.0f,6.28318530718f);
            moon.surfaceSeed=HashCoordinates(x*53+int(pi)*7+mi,y*47-int(pi)*11-mi,z*41+mi*13);
            moon.resourceRichness=0.20f+0.65f*(float((moonHash>>(mi*4))&255u)/255.0f);
            moon.hazardLevel=0.05f+0.55f*(float((moonHash>>(mi*5+3))&255u)/255.0f);
            moon.position={planet.position.x+std::cos(moon.orbitalPhaseRadians)*moon.orbitalRadius,planet.position.y+std::sin(moon.orbitalPhaseRadians)*moon.orbitalRadius,0.0f};
            sector.moons.push_back(std::move(moon));
        }
    }

    // --- Pass410R3 regenerated asteroid-belt topology ----------------------
    // A mining POI is not a solar-system belt. Build persistent macro belts
    // that can be drawn on the live System Map and streamed into dense local
    // asteroid cells only when the player travels into them.
    if (!isEventStage) {
        std::uniform_real_distribution<float> beltPhaseDist(0.0f, 6.28318530718f);
        std::uniform_real_distribution<float> beltRichnessDist(0.35f, 1.0f);
        std::uniform_real_distribution<float> beltDensityDist(0.32f, 0.92f);

        const bool makePrimaryBelt = prob(rng) < asteroidBeltProbability || sector.planets.size() >= 3;
        if (makePrimaryBelt) {
            float innerOrbit = 390000.0f;
            float outerOrbit = 520000.0f;
            if (sector.planets.size() >= 2) {
                std::vector<float> radii;
                radii.reserve(sector.planets.size());
                for (const auto& p : sector.planets) {
                    const float dx=p.position.x-sector.star.position.x,dy=p.position.y-sector.star.position.y;
                    radii.push_back(std::sqrt(dx*dx+dy*dy));
                }
                std::sort(radii.begin(),radii.end());
                const std::size_t split=std::min<std::size_t>(radii.size()-1,std::max<std::size_t>(1,radii.size()/2));
                const float midpoint=(radii[split-1]+radii[split])*0.5f;
                const float gap=std::max(50000.0f,radii[split]-radii[split-1]);
                innerOrbit=std::max(120000.0f,midpoint-gap*0.28f);
                outerOrbit=midpoint+gap*0.28f;
            }
            AsteroidBeltData belt;
            belt.beltId="belt_"+std::to_string(x)+"_"+std::to_string(y)+"_"+std::to_string(z)+"_primary";
            belt.name="Primary Asteroid Belt";
            belt.beltClass=AsteroidBeltClass::Circumstellar;
            belt.innerRadius=innerOrbit;
            belt.outerRadius=std::max(innerOrbit+42000.0f,outerOrbit);
            belt.orbitalRadius=(belt.innerRadius+belt.outerRadius)*0.5f;
            belt.orbitalPhaseRadians=beltPhaseDist(rng);
            belt.resourceRichness=beltRichnessDist(rng);
            belt.density=beltDensityDist(rng);
            belt.localAsteroidBudget=36+static_cast<int>(belt.density*28.0f);
            sector.asteroidBelts.push_back(std::move(belt));
        }

        // Gas giants receive their own planet-relative debris/minor-body belt
        // at high frequency. This is distinct from the visual dust-ring shader:
        // it is a mineable orbital region that follows the giant's reference frame.
        for (std::size_t i=0;i<sector.planets.size();++i) {
            auto& planet=sector.planets[i];
            if (planet.type!=PlanetType::GasGiant || prob(rng)>=gasGiantBeltProbability) continue;
            planet.hasRings = true;
            AsteroidBeltData belt;
            belt.beltId="belt_"+planet.planetId+"_debris";
            belt.name=planet.name+" Debris Belt";
            belt.beltClass=AsteroidBeltClass::PlanetaryDebris;
            belt.parentPlanetId=planet.planetId;
            belt.innerRadius=std::max(1200.0f,planet.radius*5.6f);
            belt.outerRadius=std::max(belt.innerRadius+900.0f,planet.radius*8.8f);
            belt.orbitalRadius=(belt.innerRadius+belt.outerRadius)*0.5f;
            belt.orbitalPhaseRadians=beltPhaseDist(rng);
            belt.resourceRichness=std::clamp(beltRichnessDist(rng)+0.08f,0.0f,1.0f);
            belt.density=std::clamp(beltDensityDist(rng)+0.06f,0.0f,1.0f);
            belt.localAsteroidBudget=44+static_cast<int>(belt.density*34.0f);
            sector.asteroidBelts.push_back(std::move(belt));
        }
    }

    // --- Pass180 native traffic / salvage / industrial encounter layer ----
    // This absorbs the useful deterministic intent from the legacy C#
    // procedural experiments while keeping every gameplay position on X/Y.
    const int minShips = std::max(0, std::min(minAmbientShips, maxAmbientShips));
    const int maxShips = std::max(minShips, maxAmbientShips);
    std::uniform_int_distribution<int> shipCountDist(minShips, maxShips);
    std::uniform_real_distribution<float> trafficPos(-kSectorPositionRange * 0.72f,
                                                      kSectorPositionRange * 0.72f);
    std::uniform_real_distribution<float> headingDist(0.0f, 6.28318530718f);
    std::uniform_int_distribution<int> shipTypeDist(0, 4);
    const char* shipTypes[] = {"Industrial", "Escort", "Hauler", "Salvager", "Patrol"};
    const int ambientShipCount = shipCountDist(rng);
    for (int i = 0; i < ambientShipCount; ++i) {
        ShipData ship;
        ship.shipId = "ship_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z) + "_" + std::to_string(i);
        ship.visualSeed = static_cast<std::uint32_t>(HashCoordinates(x * 97 + i * 13, y * 89 - i * 7, z * 83 + i * 5));
        if (ship.visualSeed == 0) ship.visualSeed = 1;
        ship.position = {trafficPos(rng), trafficPos(rng), 0.0f};
        ship.shipType = shipTypes[shipTypeDist(rng)];
        ship.heading = headingDist(rng);
        ship.hostile = sector.purpose == SectorPurpose::EventStage ? (i % 2 == 0) : (prob(rng) < 0.18f);
        ship.faction = ship.hostile ? "Raider" : (ship.shipType == "Industrial" || ship.shipType == "Hauler" ? "Independent Industry" : "Neutral Patrol");
        sector.ships.push_back(std::move(ship));
    }

    if (prob(rng) < derelictProbability) {
        DerelictData d;
        d.derelictId = "derelict_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
        d.position = {trafficPos(rng), trafficPos(rng), 0.0f};
        std::uniform_real_distribution<float> derelictSize(55.0f, 180.0f);
        std::uniform_real_distribution<float> salvageValue(900.0f, 9500.0f);
        std::uniform_real_distribution<float> danger(0.0f, 1.0f);
        d.size = derelictSize(rng);
        d.salvageValue = salvageValue(rng);
        d.danger = danger(rng);
        d.orientation = headingDist(rng);
        sector.derelicts.push_back(d);
    }

    if (prob(rng) < debrisFieldProbability) {
        DebrisFieldData field;
        field.position = {trafficPos(rng), trafficPos(rng), 0.0f};
        std::uniform_real_distribution<float> debrisRadius(220.0f, 760.0f);
        std::uniform_real_distribution<float> debrisDensity(0.20f, 0.95f);
        field.radius = debrisRadius(rng);
        field.density = debrisDensity(rng);
        sector.debrisFields.push_back(field);
    }

    EncounterData encounter;
    encounter.encounterId = "encounter_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
    encounter.position = {trafficPos(rng), trafficPos(rng), 0.0f};
    encounter.shipCount = std::max(1, ambientShipCount / 2);
    encounter.strength = 0.65f + static_cast<float>(ambientShipCount) * 0.22f;
    if (sector.purpose == SectorPurpose::EventStage) {
        encounter.archetype = EncounterArchetype::PirateRaiders;
        encounter.hostile = true;
    } else if (!sector.derelicts.empty()) {
        encounter.archetype = EncounterArchetype::SalvageClaim;
        encounter.hostile = prob(rng) < 0.35f;
    } else if (sector.purpose == SectorPurpose::PlanetaryIndustry) {
        encounter.archetype = EncounterArchetype::IndustrialConvoy;
    } else {
        encounter.archetype = prob(rng) < 0.5f ? EncounterArchetype::Trader : EncounterArchetype::Patrol;
    }
    sector.encounters.push_back(encounter);

    for (const auto& planet : sector.planets) {
        if (!planet.supportsIndustry || prob(rng) >= orbitalHubProbability) continue;
        OrbitalHubData hub;
        hub.hubId = "hub_" + planet.planetId;
        hub.planetId = planet.planetId;
        hub.position = {planet.position.x + planet.radius * (planet.hasRings ? 4.8f : 3.6f),
                        planet.position.y + planet.radius * 0.65f,
                        0.0f};
        hub.automatedLogistics = false;
        sector.orbitalHubs.push_back(std::move(hub));
    }

    // --- Pass202/203 sector composition + first-class POIs ----------------
    const int minPoi = std::max(2, minPointsOfInterest);
    const int maxPoi = std::max(minPoi, maxPointsOfInterest);
    const int lowPlanetBonus = sector.planets.size() < 2 ? 2 : 0;
    std::uniform_int_distribution<int> poiCountDist(minPoi, maxPoi);
    const int poiCount = std::min(maxPoi, poiCountDist(rng) + lowPlanetBonus);
    std::uniform_real_distribution<float> siteRadiusDist(2200.0f, 7200.0f);
    std::uniform_real_distribution<float> siteAngleDist(0.0f, 6.28318530718f);
    std::uniform_real_distribution<float> siteExtentDist(180.0f, 620.0f);
    std::uniform_real_distribution<float> siteRichnessDist(0.25f, 1.0f);
    std::uniform_real_distribution<float> siteDangerDist(0.04f, 0.95f);
    std::uniform_int_distribution<int> genericSiteDist(0, 8);

    for (int i = 0; i < poiCount; ++i) {
        SectorSiteData site;
        site.siteId = "site_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z) + "_" + std::to_string(i);
        // Guarantee one mining field and one salvage site in each sector; the
        // remaining activity budget is deterministic variety.
        site.type = i == 0 ? SectorSiteType::MiningField :
                    (i == 1 ? SectorSiteType::SalvageSite : static_cast<SectorSiteType>(genericSiteDist(rng)));
        const float angle = siteAngleDist(rng);
        const float sampledDistance = siteRadiusDist(rng);
        // Mining belts stay inside the established asteroid navigation bounds;
        // other POIs can sit farther out in the larger strategic system.
        const float distance = site.type==SectorSiteType::MiningField ? std::min(sampledDistance,4200.0f) : sampledDistance;
        site.position = {std::cos(angle)*distance,std::sin(angle)*distance,0.0f};
        site.radius = siteExtentDist(rng);
        site.resourceRichness = siteRichnessDist(rng);
        site.danger = siteDangerDist(rng);
        site.salvageValue = 500.0f + site.resourceRichness * 6500.0f;
        switch (site.type) {
            case SectorSiteType::MiningField: site.name="Prospector Belt " + std::to_string(i+1); break;
            case SectorSiteType::SalvageSite: site.name="Salvage Claim " + std::to_string(i+1); break;
            case SectorSiteType::DerelictYard: site.name="Derelict Yard " + std::to_string(i+1); break;
            case SectorSiteType::OrbitalRelay: site.name="Orbital Relay " + std::to_string(i+1); break;
            case SectorSiteType::ResearchOutpost: site.name="Survey Outpost " + std::to_string(i+1); break;
            case SectorSiteType::TradeLane: site.name="Trade Lane " + std::to_string(i+1); break;
            case SectorSiteType::DistressWreck: site.name="Distress Wreck " + std::to_string(i+1); break;
            case SectorSiteType::AnomalyBeacon: site.name="Anomaly Beacon " + std::to_string(i+1); break;
            case SectorSiteType::IndustrialDepot: site.name="Industrial Depot " + std::to_string(i+1); break;
        }
        sector.pointsOfInterest.push_back(site);

        if (site.type == SectorSiteType::MiningField) {
            // A mining POI must contain real rocks, but it must also respect
            // the caller's asteroid population contract.  Add only up to
            // maxAsteroids, then cluster the tail of the existing population
            // around the site.  This preserves custom min=max fixtures while
            // still making the POI spatially meaningful.
            std::uniform_int_distribution<int> clusterCount(5, 10);
            std::uniform_real_distribution<float> clusterAngle(0.0f, 6.28318530718f);
            std::uniform_real_distribution<float> clusterRadius(40.0f, std::max(80.0f, site.radius));
            const int desiredCluster=clusterCount(rng);
            while (static_cast<int>(sector.asteroids.size()) < maxAsteroids &&
                   static_cast<int>(sector.asteroids.size()) < std::max(minAsteroids, desiredCluster)) {
                AsteroidData ad;
                ad.size=sizeDist(rng)*(0.8f+site.resourceRichness*0.45f);
                ad.resourceType=GetRandomResourceType(rng);
                sector.asteroids.push_back(ad);
            }
            const int clustered=std::min(desiredCluster,static_cast<int>(sector.asteroids.size()));
            const std::size_t first=sector.asteroids.size()-static_cast<std::size_t>(clustered);
            for (std::size_t a=first; a<sector.asteroids.size(); ++a) {
                const float aa=clusterAngle(rng), rr=clusterRadius(rng);
                auto& ad=sector.asteroids[a];
                ad.position={site.position.x+std::cos(aa)*rr,site.position.y+std::sin(aa)*rr,0.0f};
            }
        } else if (site.type == SectorSiteType::SalvageSite || site.type == SectorSiteType::DerelictYard || site.type == SectorSiteType::DistressWreck) {
            DerelictData d;
            d.derelictId="poi_"+site.siteId+"_wreck"; d.position=site.position; d.size=70.0f+site.resourceRichness*120.0f;
            d.salvageValue=site.salvageValue; d.danger=site.danger; d.orientation=headingDist(rng);
            sector.derelicts.push_back(d);
            DebrisFieldData field; field.position=site.position; field.radius=site.radius; field.density=0.30f+site.resourceRichness*0.55f;
            sector.debrisFields.push_back(field);
        }
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

    // Pass496: all generated content is normalized through one solar-system
    // placement authority before the sector can escape generation. This closes
    // legacy origin-centered spawns inside the star and establishes the grammar
    // every procedural system follows.
    SolarSystemPlacementSystem placement;
    placement.Normalize(sector);
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


PlanetType GalaxyGenerator::GetRandomPlanetType(std::mt19937& rng) const {
    // Weighted toward solid landable worlds; gas giants remain uncommon but
    // visually important strategic backdrops.
    std::uniform_int_distribution<int> dist(0, 99);
    int roll = dist(rng);
    if (roll < 24) return PlanetType::Rocky;
    if (roll < 40) return PlanetType::Desert;
    if (roll < 54) return PlanetType::Ice;
    if (roll < 66) return PlanetType::Oceanic;
    if (roll < 77) return PlanetType::Volcanic;
    if (roll < 90) return PlanetType::Barren;
    return PlanetType::GasGiant;
}

std::string GalaxyGenerator::GeneratePlanetName(std::mt19937& rng, int index) const {
    static const char* prefixes[] = {
        "Aster", "Cinder", "Helios", "Morrow", "Orison",
        "Pioneer", "Rime", "Sable", "Vesper", "Wayfarer"
    };
    static const char* suffixes[] = {
        "Prime", "Reach", "Haven", "Basin", "Crown",
        "Drift", "Field", "March", "Vale", "World"
    };
    std::uniform_int_distribution<int> prefDist(0, 9);
    std::uniform_int_distribution<int> sufDist(0, 9);
    return std::string(prefixes[prefDist(rng)]) + " " + suffixes[sufDist(rng)] +
           " " + std::to_string(index + 1);
}

} // namespace subspace
