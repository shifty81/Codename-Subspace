#pragma once

#include "station/StationEcologySystem.h"
#include "station/StationDockingGeometrySystem.h"

#include <string>
#include <vector>

namespace subspace {

struct HangarSceneProfile {
    std::string profileId;
    float width = 44.0f;
    float length = 36.0f;
    float height = 8.0f;
    int serviceArms = 4;
    int cargoLifts = 2;
    int serviceDrones = 4;
    bool externalClamp = false;
    bool excavatedRock = false;
    bool constructionFrame = false;
    bool brightCommercial = false;
    std::vector<std::string> ambientActivities;
};

class StationHangarPresentationSystem {
public:
    HangarSceneProfile Build(StationArchetype archetype,StationBerthSize berth) const;
};

} // namespace subspace
