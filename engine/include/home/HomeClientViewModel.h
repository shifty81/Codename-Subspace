#pragma once

#include "roguelite/RogueliteDirector.h"
#include "home/HomeProductionPlanner.h"
#include "home/HomePowerGrid.h"
#include "home/HomeLogisticsNetwork.h"
#include "home/HomeShipyardBuildQueue.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct HomeBodyView {
    std::string id;
    std::string displayName;
    std::string typeName;
    float orbitRadius = 0.0f;
    float orbitAngleRadians = 0.0f;
    float visualRadius = 12.0f;
    std::uint32_t color = 0xFFFFFFu;
    bool primary = false;
    bool interactive = true;
};

struct HomeBuildZoneView {
    std::string id;
    std::string displayName;
    std::string typeName;
    std::string resourceSummary;
    int gridWidth = 0;
    int gridHeight = 0;
    int structureCount = 0;
};

struct HomeStructureView {
    std::string id;
    std::string typeName;
    std::string zoneId;
    int x = 0;
    int y = 0;
    int tier = 1;
    bool powered = false;
    bool automated = false;
};

struct HomeRunOfferView {
    int index = 0;
    std::string displayName;
    std::string riskLabel;
    std::string rewardLabel;
    std::string summary;
};

struct HomeClientViewModel {
    std::string title;
    std::string safetyText;
    std::string summaryText;
    std::string powerText;
    std::string automationText;
    std::string inventoryText;
    std::string shipyardText;
    std::string productionText;
    std::string powerGridText;
    std::string logisticsText;
    std::string shipyardQueueText;
    std::vector<HomeBodyView> bodies;
    std::vector<HomeBuildZoneView> buildZones;
    std::vector<HomeStructureView> structures;
    std::vector<HomeRunOfferView> runOffers;
};

HomeClientViewModel BuildHomeClientViewModel(const RogueliteDirectorState& director,
                                             float elapsedSeconds = 0.0f);
std::string HomeClientViewSummary(const HomeClientViewModel& view);

} // namespace subspace
