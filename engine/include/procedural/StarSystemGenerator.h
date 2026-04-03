#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// Enumerations
// ---------------------------------------------------------------------------

enum class StarType {
    MainSequence,   ///< Yellow dwarf (most common)
    RedDwarf,       ///< Small, cool, common
    BlueGiant,      ///< Rare, massive, hot
    NeutronStar,    ///< Compact remnant
    WhiteDwarf,     ///< End-stage remnant
    BinaryPair      ///< Two stars orbiting each other
};

enum class PlanetType {
    Terrestrial,
    GasGiant,
    IceGiant,
    Desert,
    Ocean,
    Volcanic,
    Barren,
    Asteroid       ///< Not a true planet; belt/dwarf
};

// ---------------------------------------------------------------------------
// Data structures
// ---------------------------------------------------------------------------

struct PlanetData {
    std::string name;
    PlanetType  type       = PlanetType::Barren;
    float       orbitRadius = 1.0f;   ///< AU
    float       radius      = 1.0f;   ///< Earth radii
    float       mass        = 1.0f;   ///< Earth masses
    bool        habitable   = false;
    bool        hasMoons    = false;
    int         moonCount   = 0;
    std::string dominantResource;
};

struct AsteroidBeltData {
    float innerRadiusAU = 2.0f;
    float outerRadiusAU = 3.5f;
    std::string dominantResource = "Iron";
    int approximateCount = 500;
};

struct StarSystemData {
    int sectorX = 0, sectorY = 0, sectorZ = 0;
    std::string  systemName;
    StarType     starType       = StarType::MainSequence;
    float        starMass       = 1.0f;   ///< Solar masses
    float        starLuminosity = 1.0f;   ///< Solar luminosities
    uint32_t     starColorRGB   = 0xFFFF80;

    std::vector<PlanetData>       planets;
    std::vector<AsteroidBeltData> asteroidBelts;
    bool  hasJumpGate = false;
    float jumpGateOrbitRadius = 0.0f;

    int   seed = 0;
};

// ---------------------------------------------------------------------------
// StarSystemGenerator
// ---------------------------------------------------------------------------

/// Deterministically generates complete solar systems from sector coordinates.
class StarSystemGenerator {
public:
    explicit StarSystemGenerator(int galaxySeed = 0);

    /// Generate a solar system at the given sector coordinates.
    StarSystemData GenerateSystem(int sectorX, int sectorY, int sectorZ) const;

    /// Probability that a system contains a jump gate (default 0.15).
    float jumpGateProbability = 0.15f;

    /// Jump gate spawn probability increases for well-connected routes.
    float tradeRouteProbability = 0.30f;

    int galaxySeed() const { return _galaxySeed; }

private:
    int _galaxySeed;

    int  HashSector(int x, int y, int z) const;
    StarType PickStarType(std::mt19937& rng) const;
    std::string GenerateSystemName(std::mt19937& rng, int x, int y, int z) const;
    PlanetData  GeneratePlanet(std::mt19937& rng, int planetIndex,
                               StarType starType) const;
    AsteroidBeltData GenerateBelt(std::mt19937& rng) const;
    uint32_t StarColor(StarType type) const;
};

} // namespace subspace
