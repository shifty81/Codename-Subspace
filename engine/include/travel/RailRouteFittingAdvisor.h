#pragma once

#include <string>
#include <vector>

namespace subspace {

struct RailRouteFitInputs {
    float fuel = 0.0f;
    float cargoCapacity = 0.0f;
    float defense = 0.0f;
    float scanner = 0.0f;
    float routeFuelNeed = 0.0f;
    float routeCargoOpportunity = 0.0f;
    float routeHazard = 0.0f;
    float routeScannerNeed = 0.0f;
};

struct RailRouteFitAdvice {
    bool canLaunch = false;
    bool lucrative = false;
    float safetyScore = 0.0f;
    float yieldScore = 0.0f;
    std::vector<std::string> missingUpgrades;
};

RailRouteFitAdvice AdviseRailRouteFit(const RailRouteFitInputs& inputs);
std::string RailRouteFitAdviceSummary(const RailRouteFitAdvice& advice);

} // namespace subspace
