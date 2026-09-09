#pragma once

#include "expedition/ExpeditionContractBoard.h"
#include "home/HomeShipPrep.h"
#include "travel/InterstellarRailTravel.h"

#include <string>
#include <vector>

namespace subspace {

struct AdventureLaunchOption {
    std::string id;
    std::string displayName;
    std::string destinationSystemId;
    std::string requiredFitSummary;
    std::string expectedRewardSummary;
    int riskScore = 0;
    bool available = false;
};

struct AdventureLaunchPlan {
    std::vector<AdventureLaunchOption> options;
    std::string recommendedOptionId;
    std::vector<std::string> blockers;
};

AdventureLaunchPlan BuildAdventureLaunchPlan(const HomeShipPrepState& prep,
                                             const HomeFactoryNetworkState& inventory,
                                             const std::vector<ExpeditionContract>& contracts,
                                             const std::vector<RailTravelRouteOption>& routes);
std::string AdventureLaunchPlanSummary(const AdventureLaunchPlan& plan);

} // namespace subspace
