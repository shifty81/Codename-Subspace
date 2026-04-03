#include "procedural/StarSystemGenerator.h"

#include <cmath>
#include <sstream>

namespace subspace {

StarSystemGenerator::StarSystemGenerator(int galaxySeed)
    : _galaxySeed(galaxySeed == 0 ? 1337 : galaxySeed) {}

StarSystemData StarSystemGenerator::GenerateSystem(int sx, int sy, int sz) const {
    int seed = HashSector(sx, sy, sz);
    std::mt19937 rng(static_cast<uint32_t>(seed));

    StarSystemData sys;
    sys.sectorX  = sx; sys.sectorY = sy; sys.sectorZ = sz;
    sys.seed     = seed;
    sys.starType = PickStarType(rng);
    sys.starColorRGB  = StarColor(sys.starType);
    sys.systemName    = GenerateSystemName(rng, sx, sy, sz);

    // Star mass / luminosity
    std::uniform_real_distribution<float> massVar(0.8f, 1.2f);
    switch (sys.starType) {
        case StarType::RedDwarf:
            sys.starMass = 0.3f * massVar(rng);
            sys.starLuminosity = 0.01f;
            break;
        case StarType::BlueGiant:
            sys.starMass = 20.0f * massVar(rng);
            sys.starLuminosity = 50000.0f;
            break;
        case StarType::NeutronStar:
            sys.starMass = 1.4f;
            sys.starLuminosity = 0.001f;
            break;
        case StarType::WhiteDwarf:
            sys.starMass = 0.6f;
            sys.starLuminosity = 0.002f;
            break;
        case StarType::BinaryPair:
            sys.starMass = 2.5f * massVar(rng);
            sys.starLuminosity = 3.0f;
            break;
        default: // MainSequence
            sys.starMass = massVar(rng);
            sys.starLuminosity = massVar(rng);
            break;
    }

    // Generate planets
    std::uniform_int_distribution<int> numPlanets(1, 8);
    int planetCount = numPlanets(rng);
    for (int i = 0; i < planetCount; ++i)
        sys.planets.push_back(GeneratePlanet(rng, i, sys.starType));

    // Asteroid belt (40% chance)
    std::uniform_real_distribution<float> prob(0.0f, 1.0f);
    if (prob(rng) < 0.4f)
        sys.asteroidBelts.push_back(GenerateBelt(rng));

    // Jump gate
    float jgProb = jumpGateProbability;
    if (std::abs(sx) + std::abs(sy) + std::abs(sz) < 5) jgProb = tradeRouteProbability;
    sys.hasJumpGate = (prob(rng) < jgProb);
    if (sys.hasJumpGate) {
        std::uniform_real_distribution<float> orb(0.8f, 2.0f);
        sys.jumpGateOrbitRadius = orb(rng);
    }

    return sys;
}

int StarSystemGenerator::HashSector(int x, int y, int z) const {
    int h = _galaxySeed;
    h = h * 31 + x;
    h = h * 31 + y;
    h = h * 31 + z;
    return h;
}

StarType StarSystemGenerator::PickStarType(std::mt19937& rng) const {
    std::uniform_int_distribution<int> d(0, 99);
    int v = d(rng);
    if (v < 60) return StarType::MainSequence;
    if (v < 85) return StarType::RedDwarf;
    if (v < 90) return StarType::BlueGiant;
    if (v < 93) return StarType::BinaryPair;
    if (v < 97) return StarType::WhiteDwarf;
    return StarType::NeutronStar;
}

std::string StarSystemGenerator::GenerateSystemName(std::mt19937& rng,
                                                     int x, int y, int z) const {
    static const char* greek[] = { "Alpha","Beta","Gamma","Delta","Epsilon",
                                   "Zeta","Eta","Theta","Iota","Kappa" };
    static const char* suffix[] = { "Prime","Secundus","Tercius","Majoris","Minoris" };
    std::uniform_int_distribution<int> dg(0, 9), ds(0, 4);
    (void)x; (void)y; (void)z;
    return std::string(greek[dg(rng)]) + " " + suffix[ds(rng)];
}

PlanetData StarSystemGenerator::GeneratePlanet(std::mt19937& rng, int idx,
                                                StarType star) const {
    static const char* resources[] = {
        "Iron", "Titanium", "Naonite", "Trinium", "Ice", "Gas", "Carbon"
    };
    std::uniform_int_distribution<int> resD(0, 6);
    std::uniform_real_distribution<float> massD(0.1f, 15.0f);
    std::uniform_real_distribution<float> radD(0.3f, 12.0f);

    PlanetData p;
    p.name           = "Planet " + std::to_string(idx + 1);
    p.orbitRadius    = 0.4f + static_cast<float>(idx) * 1.1f;
    p.mass           = massD(rng);
    p.radius         = radD(rng);
    p.dominantResource = resources[resD(rng)];

    // Choose type by orbit and star
    if (p.orbitRadius < 1.0f) {
        p.type = (star == StarType::BlueGiant) ? PlanetType::Volcanic : PlanetType::Desert;
    } else if (p.orbitRadius < 3.5f) {
        p.type = (p.mass > 8.0f) ? PlanetType::GasGiant : PlanetType::Terrestrial;
    } else {
        p.type = (p.mass > 6.0f) ? PlanetType::IceGiant : PlanetType::Barren;
    }

    // Habitability: terrestrial, in goldilocks zone (0.8–1.6 AU), main sequence
    p.habitable = (p.type == PlanetType::Terrestrial) &&
                  (p.orbitRadius >= 0.8f && p.orbitRadius <= 1.6f) &&
                  (star == StarType::MainSequence);

    std::uniform_int_distribution<int> moonD(0, 5);
    p.moonCount = moonD(rng);
    p.hasMoons  = p.moonCount > 0;
    return p;
}

AsteroidBeltData StarSystemGenerator::GenerateBelt(std::mt19937& rng) const {
    static const char* res[] = { "Iron", "Titanium", "Naonite", "Carbon" };
    std::uniform_int_distribution<int> rd(0, 3), cnt(200, 2000);
    std::uniform_real_distribution<float> inner(2.0f, 3.0f), outer(3.5f, 5.0f);
    AsteroidBeltData b;
    b.innerRadiusAU = inner(rng);
    b.outerRadiusAU = outer(rng);
    b.dominantResource   = res[rd(rng)];
    b.approximateCount   = cnt(rng);
    return b;
}

uint32_t StarSystemGenerator::StarColor(StarType type) const {
    switch (type) {
        case StarType::RedDwarf:   return 0xFF4400;
        case StarType::BlueGiant:  return 0x4488FF;
        case StarType::NeutronStar: return 0xCCCCFF;
        case StarType::WhiteDwarf: return 0xFFFFFF;
        case StarType::BinaryPair: return 0xFFCC44;
        default:                   return 0xFFFF80;
    }
}

} // namespace subspace
