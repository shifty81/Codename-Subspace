#include "home/HomeProductionPlanner.h"

#include <algorithm>
#include <sstream>

namespace subspace {
namespace {
int CountPowered(const HomeSolarSystemState& home, HomeStructureType type) {
    return static_cast<int>(std::count_if(home.structures.begin(), home.structures.end(), [type](const HomeStructure& structure) {
        return structure.type == type && structure.powered;
    }));
}

HomeProductionRoute MakeRoute(const std::string& id,
                              HomeStructureType source,
                              HomeStructureType destination,
                              const std::string& commodity,
                              float rate,
                              bool active) {
    HomeProductionRoute route;
    route.id = id;
    route.source = source;
    route.destination = destination;
    route.commodity = commodity;
    route.unitsPerMinute = rate;
    route.active = active;
    return route;
}
}

HomeProductionPlan BuildHomeProductionPlan(const HomeSolarSystemState& home,
                                           const HomeFactoryNetworkState& factory) {
    HomeProductionPlan plan;
    plan.extractorCount = CountPowered(home, HomeStructureType::Extractor);
    plan.refineryCount = CountPowered(home, HomeStructureType::Refinery);
    plan.assemblerCount = CountPowered(home, HomeStructureType::Assembler);
    plan.storageCount = CountPowered(home, HomeStructureType::StorageDepot);
    plan.shipyardCount = CountPowered(home, HomeStructureType::ShipyardBay);

    const bool hasLogistics = CountHomeStructures(home, HomeStructureType::ConveyorHub) > 0 ||
                              CountHomeStructures(home, HomeStructureType::DroneDepot) > 0;
    const bool hasPower = EstimateHomePowerGeneration(home) > 0 || factory.availablePower > 0;

    plan.rawOrePerMinute = hasPower ? static_cast<float>(plan.extractorCount) * 6.0f : 0.0f;
    plan.refinedMaterialPerMinute = std::min(plan.rawOrePerMinute * 0.5f,
                                             static_cast<float>(plan.refineryCount) * 4.0f);
    plan.moduleComponentPerMinute = std::min(plan.refinedMaterialPerMinute * 0.35f,
                                             static_cast<float>(plan.assemblerCount) * 2.0f);

    plan.routes.push_back(MakeRoute("ore-to-refinery", HomeStructureType::Extractor, HomeStructureType::Refinery,
                                    "ore", plan.rawOrePerMinute, plan.extractorCount > 0 && plan.refineryCount > 0 && hasLogistics));
    plan.routes.push_back(MakeRoute("refined-to-storage", HomeStructureType::Refinery, HomeStructureType::StorageDepot,
                                    "refined-material", plan.refinedMaterialPerMinute, plan.refineryCount > 0 && plan.storageCount > 0 && hasLogistics));
    plan.routes.push_back(MakeRoute("components-to-shipyard", HomeStructureType::Assembler, HomeStructureType::ShipyardBay,
                                    "module-components", plan.moduleComponentPerMinute, plan.assemblerCount > 0 && plan.shipyardCount > 0 && hasLogistics));

    if (!hasPower) {
        plan.bottleneck = "power";
    }
    else if (!hasLogistics) {
        plan.bottleneck = "logistics";
    }
    else if (plan.extractorCount == 0) {
        plan.bottleneck = "raw extraction";
    }
    else if (plan.refineryCount == 0) {
        plan.bottleneck = "refining";
    }
    else if (plan.storageCount == 0) {
        plan.bottleneck = "storage";
    }
    else {
        plan.bottleneck = "balanced starter chain";
    }
    return plan;
}

std::string HomeProductionRouteSummary(const HomeProductionRoute& route) {
    std::ostringstream stream;
    stream << HomeStructureTypeName(route.source) << " -> " << HomeStructureTypeName(route.destination)
           << " " << route.commodity << " " << route.unitsPerMinute << "/min "
           << (route.active ? "active" : "blocked");
    return stream.str();
}

std::string HomeProductionPlanSummary(const HomeProductionPlan& plan) {
    std::ostringstream stream;
    stream << "Production ore=" << plan.rawOrePerMinute << "/min refined=" << plan.refinedMaterialPerMinute
           << "/min components=" << plan.moduleComponentPerMinute << "/min bottleneck=" << plan.bottleneck;
    return stream.str();
}

} // namespace subspace
