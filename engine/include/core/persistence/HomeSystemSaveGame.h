#pragma once

#include "expedition/ExpeditionRun.h"
#include "home/HomeFactoryNetwork.h"
#include "home/HomeShipyardProgression.h"
#include "home/HomeSolarSystem.h"

#include <string>

namespace subspace {

struct HomeSystemSaveSnapshot {
    std::string saveId = "home-dev-save";
    int saveVersion = 1;
    HomeSolarSystemState home;
    HomeFactoryNetworkState factory;
    ShipyardProgressionState shipyard;
    ExpeditionRunStateSnapshot lastRun;
};

HomeSystemSaveSnapshot CreateDefaultHomeSystemSave(std::uint32_t seed = 0x51B5ACEu);
std::string SerializeHomeSystemSaveSnapshot(const HomeSystemSaveSnapshot& snapshot);
HomeSystemSaveSnapshot DeserializeHomeSystemSaveSnapshot(const std::string& text);
std::string HomeSystemSaveSummary(const HomeSystemSaveSnapshot& snapshot);

} // namespace subspace
