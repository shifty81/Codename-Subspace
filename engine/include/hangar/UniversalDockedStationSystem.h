#pragma once

#include "station/StationEcologySystem.h"
#include "station/StationServiceSystem.h"
#include <string>
#include <vector>

namespace subspace {

struct DockedStationPresentation {
    StationArchetype type=StationArchetype::TradeHub;
    std::string hangarProfile;
    std::vector<std::string> services;
    bool orbitCamera=true;
    bool showActualFittedShip=true;
    bool onFoot=false;
    float cameraMinDistance=4.0f;
    float cameraMaxDistance=90.0f;
    StationServiceProfile serviceProfile{};
    std::vector<StationServiceType> availableServiceTypes;
};

class UniversalDockedStationSystem {
public:
    DockedStationPresentation Build(const GeneratedStationProfile& station) const;
};

} // namespace subspace
