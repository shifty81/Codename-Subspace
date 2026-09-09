#include "travel/RailRouteFittingAdvisor.h"

#include <algorithm>
#include <sstream>

namespace subspace {

RailRouteFitAdvice AdviseRailRouteFit(const RailRouteFitInputs& inputs) {
    RailRouteFitAdvice advice;
    advice.safetyScore = 0.0f;
    if (inputs.routeFuelNeed > 0.0f) advice.safetyScore += std::min(1.0f, inputs.fuel / inputs.routeFuelNeed) * 0.35f;
    advice.safetyScore += std::min(1.0f, inputs.defense / std::max(1.0f, inputs.routeHazard)) * 0.35f;
    advice.safetyScore += std::min(1.0f, inputs.scanner / std::max(1.0f, inputs.routeScannerNeed)) * 0.30f;
    advice.yieldScore = std::min(inputs.cargoCapacity, inputs.routeCargoOpportunity) / std::max(1.0f, inputs.routeCargoOpportunity);
    if (inputs.fuel + 0.01f < inputs.routeFuelNeed) advice.missingUpgrades.push_back("fuel capacity");
    if (inputs.defense + 0.01f < inputs.routeHazard) advice.missingUpgrades.push_back("defense rating");
    if (inputs.scanner + 0.01f < inputs.routeScannerNeed) advice.missingUpgrades.push_back("scanner rating");
    if (inputs.cargoCapacity + 0.01f < inputs.routeCargoOpportunity) advice.missingUpgrades.push_back("cargo capacity");
    advice.canLaunch = advice.safetyScore >= 0.65f && inputs.fuel >= inputs.routeFuelNeed;
    advice.lucrative = advice.canLaunch && advice.yieldScore >= 0.70f;
    return advice;
}

std::string RailRouteFitAdviceSummary(const RailRouteFitAdvice& advice) {
    std::ostringstream out;
    out << (advice.canLaunch ? "Launchable" : "Not launchable") << " safety=" << advice.safetyScore << " yield=" << advice.yieldScore;
    if (!advice.missingUpgrades.empty()) {
        out << " missing=";
        for (std::size_t i = 0; i < advice.missingUpgrades.size(); ++i) {
            if (i) out << ",";
            out << advice.missingUpgrades[i];
        }
    }
    return out.str();
}

} // namespace subspace
