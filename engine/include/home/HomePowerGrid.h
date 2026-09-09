#pragma once

#include "home/HomeSolarSystem.h"

#include <string>

namespace subspace {

struct HomePowerGridReport {
    int generation = 0;
    int structureLoad = 0;
    int stored = 0;
    int surplus = 0;
    int dysonNodes = 0;
    int solarCollectors = 0;
    float solarEfficiency = 1.0f;
    bool stable = true;
};

HomePowerGridReport AnalyzeHomePowerGrid(const HomeSolarSystemState& home, float stellarIntensity = 1.0f);
int EstimateHomeStructurePowerLoad(const HomeSolarSystemState& home);
std::string HomePowerGridSummary(const HomePowerGridReport& report);

} // namespace subspace
