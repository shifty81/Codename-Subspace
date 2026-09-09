#pragma once

#include "home/HomeFactoryNetwork.h"
#include "home/HomeSolarSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct HomeProductionRoute {
    std::string id;
    HomeStructureType source = HomeStructureType::Unknown;
    HomeStructureType destination = HomeStructureType::Unknown;
    std::string commodity;
    float unitsPerMinute = 0.0f;
    bool active = false;
};

struct HomeProductionPlan {
    int extractorCount = 0;
    int refineryCount = 0;
    int assemblerCount = 0;
    int storageCount = 0;
    int shipyardCount = 0;
    float rawOrePerMinute = 0.0f;
    float refinedMaterialPerMinute = 0.0f;
    float moduleComponentPerMinute = 0.0f;
    std::vector<HomeProductionRoute> routes;
    std::string bottleneck;
};

HomeProductionPlan BuildHomeProductionPlan(const HomeSolarSystemState& home,
                                           const HomeFactoryNetworkState& factory);
std::string HomeProductionPlanSummary(const HomeProductionPlan& plan);
std::string HomeProductionRouteSummary(const HomeProductionRoute& route);

} // namespace subspace
