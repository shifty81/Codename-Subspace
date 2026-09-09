#include "home/HomeShipBuilderBay.h"

#include <algorithm>
#include <sstream>

namespace subspace {

HomeShipBuilderBayState CreateStarterHomeShipBuilderBay(int credits) {
    HomeShipBuilderBayState bay;
    bay.availableCredits = credits;
    bay.activeLoadout = CreateStarterShipLoadout();
    bay.availableParts = CreateStarterShipPartCatalog();
    return bay;
}

HomeShipBuilderBayReport AnalyzeHomeShipBuilderBay(const HomeShipBuilderBayState& bay) {
    HomeShipBuilderBayReport report;
    report.canEdit = bay.atHome && !bay.expeditionActive;
    report.stats = CalculateShipPartStats(bay.activeLoadout, bay.availableParts);
    report.status = report.canEdit ? "home builder bay ready" : "ship locked until home docking";
    for (const auto& part : bay.availableParts) {
        if (part.hotSwappableAtHome && bay.availableCredits >= part.installCostCredits) {
            report.compatiblePartIds.push_back(part.id);
        }
    }
    return report;
}

ShipPartInstallResult InstallHomeShipPart(HomeShipBuilderBayState& bay, const std::string& partId) {
    const ShipPartDefinition* part = FindShipPart(bay.availableParts, partId);
    if (!part) {
        return {false, "unknown part: " + partId, 0};
    }
    ShipPartInstallResult result = InstallShipPart(bay.activeLoadout, *part, bay.availableCredits, bay.atHome, bay.expeditionActive);
    if (result.success) {
        bay.availableCredits -= result.costCredits;
    }
    bay.installLog.push_back(result.message);
    return result;
}

std::string HomeShipBuilderBaySummary(const HomeShipBuilderBayState& bay) {
    const HomeShipBuilderBayReport report = AnalyzeHomeShipBuilderBay(bay);
    std::ostringstream ss;
    ss << report.status << " credits=" << bay.availableCredits
       << " compatibleParts=" << report.compatiblePartIds.size()
       << " cargo=" << report.stats.cargoCapacity
       << " thrust=" << report.stats.thrust;
    return ss.str();
}

} // namespace subspace
