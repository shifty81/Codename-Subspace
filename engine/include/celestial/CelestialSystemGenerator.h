#pragma once

#include "celestial/CelestialTypes.h"

#include <cstdint>
#include <string>

namespace subspace {

struct CelestialSystemGeneratorOptions {
    int minPlanets = 3;
    int maxPlanets = 8;
    bool includeAsteroidBelt = true;
    bool allowBlackHolePrimary = false;
    bool includeGameplayTags = true;
};

class CelestialSystemGenerator {
public:
    explicit CelestialSystemGenerator(CelestialSystemGeneratorOptions options = {});

    StarSystemDefinition GenerateSystem(const std::string& sectorId, std::uint32_t seed) const;
    CelestialBodyDefinition GenerateBody(const std::string& systemId,
                                         int orbitIndex,
                                         CelestialOrbitRole role,
                                         std::uint32_t seed) const;

private:
    CelestialSystemGeneratorOptions options_;
};

float CelestialSeededUnit(std::uint32_t seed, int index);
std::uint32_t CelestialHashSeed(const std::string& text, std::uint32_t salt = 0);

} // namespace subspace
