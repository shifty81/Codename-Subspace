#include "expedition/AdventureLaunchPlanner.h"

#include <sstream>

namespace subspace {

namespace {
ShipRailTravelFit BuildFitFromPrep(const HomeShipPrepState& prep) {
    ShipRailTravelFit fit;
    fit.driveTier = 1;
    fit.defenseRating = prep.defenseRating;
    fit.scannerRating = prep.scannerRating;
    fit.cargoCapacity = prep.cargoReserved;
    fit.fuelAvailable = static_cast<float>(prep.fuelLoaded);
    fit.fuelCapacity = static_cast<float>(prep.fuelLoaded);
    fit.mass = 1.0f;
    fit.thrustRating = 1.0f;
    fit.hasRailDrive = true;
    return fit;
}
} // namespace

AdventureLaunchPlan BuildAdventureLaunchPlan(const HomeShipPrepState& prep,
                                             const HomeFactoryNetworkState& inventory,
                                             const std::vector<ExpeditionContract>& contracts,
                                             const std::vector<RailTravelRouteOption>& routes) {
    AdventureLaunchPlan plan;
    const HomeShipPrepValidation validation = ValidateHomeShipPrepForAdventure(prep, inventory);
    const ShipRailTravelFit fit = BuildFitFromPrep(prep);
    if (!validation.canLaunch) {
        plan.blockers.push_back("Ship prep validation blocks launch: " + HomeShipPrepValidationSummary(validation));
    }

    const std::size_t optionCount = contracts.size() > routes.size() ? contracts.size() : routes.size();
    for (std::size_t i = 0; i < optionCount; ++i) {
        AdventureLaunchOption option;
        option.id = "launch-" + std::to_string(i + 1);
        if (i < contracts.size()) {
            option.displayName = contracts[i].displayName;
            option.expectedRewardSummary = ExpeditionContractSummary(contracts[i]);
            option.riskScore += contracts[i].config.depth * 10;
        } else {
            option.displayName = "Unassigned Expedition";
        }
        if (i < routes.size()) {
            const RailTravelFitReport routeFit = EvaluateRailTravelFit(routes[i], fit);
            option.destinationSystemId = routes[i].destinationSystemId;
            option.requiredFitSummary = RailTravelFitSummary(routeFit);
            option.riskScore += static_cast<int>(routes[i].hazardPressure * 10.0f);
            option.available = validation.canLaunch && routeFit.canLaunch;
        } else {
            option.destinationSystemId = "unknown";
            option.available = validation.canLaunch;
        }
        if (option.available && plan.recommendedOptionId.empty()) {
            plan.recommendedOptionId = option.id;
        }
        plan.options.push_back(option);
    }
    if (plan.options.empty()) {
        plan.blockers.push_back("No expedition contracts or interstellar routes are available.");
    }
    return plan;
}

std::string AdventureLaunchPlanSummary(const AdventureLaunchPlan& plan) {
    int available = 0;
    for (const auto& option : plan.options) {
        if (option.available) ++available;
    }
    std::ostringstream out;
    out << "adventureLaunch options=" << plan.options.size()
        << " available=" << available
        << " recommended=" << plan.recommendedOptionId
        << " blockers=" << plan.blockers.size();
    return out.str();
}

} // namespace subspace
