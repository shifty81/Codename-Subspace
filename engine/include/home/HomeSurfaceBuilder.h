#pragma once

#include "home/HomeFactoryNetwork.h"
#include "home/HomeSolarSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class HomePlacementStatus {
    Success,
    InvalidZone,
    OutOfBounds,
    Occupied,
    InsufficientResources,
    InvalidStructure,
    ProtectedStarterStructure
};

struct HomeStructureCost {
    HomeStructureType type = HomeStructureType::Unknown;
    int tier = 1;
    std::vector<HomeInventoryStack> inputs;
    int powerDelta = 0;
    int automationDelta = 0;
};

struct HomeBuildPlacementRequest {
    std::string zoneId;
    HomeStructureType type = HomeStructureType::Unknown;
    int x = 0;
    int y = 0;
    int tier = 1;
    bool freeBuild = false;
};

struct HomeBuildPlacementResult {
    bool success = false;
    HomePlacementStatus status = HomePlacementStatus::InvalidStructure;
    std::string message;
    HomeStructure placedStructure;
    std::vector<HomeInventoryStack> consumed;
};

std::vector<HomeStructureType> CreateHomeBuildPalette(HomeBuildZoneType zoneType);
HomeStructureCost GetHomeStructureCost(HomeStructureType type, int tier = 1);
bool IsHomeStructureAllowedInZone(HomeStructureType type, HomeBuildZoneType zoneType);
const HomeBuildZone* FindHomeBuildZone(const HomeSolarSystemState& home, const std::string& zoneId);
const HomeStructure* FindHomeStructureAt(const HomeSolarSystemState& home, const std::string& zoneId, int x, int y);
HomeStructure* FindHomeStructureAt(HomeSolarSystemState& home, const std::string& zoneId, int x, int y);
HomeBuildPlacementResult PlaceHomeStructure(HomeSolarSystemState& home,
                                            HomeFactoryNetworkState& network,
                                            const HomeBuildPlacementRequest& request);
HomeBuildPlacementResult RemoveHomeStructure(HomeSolarSystemState& home,
                                             HomeFactoryNetworkState& network,
                                             const std::string& zoneId,
                                             int x,
                                             int y);
void RecalculateHomeDerivedState(HomeSolarSystemState& home, HomeFactoryNetworkState* network = nullptr);
std::string HomePlacementStatusName(HomePlacementStatus status);
std::string HomeStructureCostSummary(const HomeStructureCost& cost);

} // namespace subspace
