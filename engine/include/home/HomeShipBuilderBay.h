#pragma once

#include "ships/ShipPartCatalog.h"

#include <string>
#include <vector>

namespace subspace {

struct HomeShipBuilderBayState {
    bool atHome = true;
    bool expeditionActive = false;
    int availableCredits = 0;
    ShipLoadout activeLoadout;
    std::vector<ShipPartDefinition> availableParts;
    std::vector<std::string> installLog;
};

struct HomeShipBuilderBayReport {
    bool canEdit = false;
    std::string status;
    ShipPartStats stats;
    std::vector<std::string> compatiblePartIds;
};

HomeShipBuilderBayState CreateStarterHomeShipBuilderBay(int credits = 2500);
HomeShipBuilderBayReport AnalyzeHomeShipBuilderBay(const HomeShipBuilderBayState& bay);
ShipPartInstallResult InstallHomeShipPart(HomeShipBuilderBayState& bay, const std::string& partId);
std::string HomeShipBuilderBaySummary(const HomeShipBuilderBayState& bay);

} // namespace subspace
