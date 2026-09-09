#pragma once

#include "home/HomeSolarSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct HomeOverviewBodyNode {
    std::string bodyId;
    std::string displayName;
    std::string role;
    float orbitRadius = 0.0f;
    float orbitAngleRadians = 0.0f;
    bool primaryHomeWorld = false;
    bool supportsOutposts = false;
    std::string resourceSummary;
};

struct HomeOverviewRouteNode {
    std::string id;
    std::string fromBodyId;
    std::string toBodyId;
    std::string commodity;
    float unitsPerMinute = 0.0f;
    bool active = false;
};

struct HomeSystemOverviewModel {
    std::string title = "Home Solar System Overview";
    std::vector<HomeOverviewBodyNode> bodies;
    std::vector<HomeOverviewRouteNode> routes;
    std::string summary;
};

HomeSystemOverviewModel BuildHomeSystemOverviewModel(const HomeSolarSystemState& home);
std::string HomeSystemOverviewSummary(const HomeSystemOverviewModel& overview);

} // namespace subspace
