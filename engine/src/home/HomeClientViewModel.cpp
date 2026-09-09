#include "home/HomeClientViewModel.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {
namespace {

std::string JoinTags(const std::vector<std::string>& tags)
{
    if (tags.empty()) {
        return "none";
    }
    std::string result;
    for (std::size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) {
            result += ", ";
        }
        result += tags[i];
    }
    return result;
}

int CountStructuresInZone(const HomeSolarSystemState& home, const std::string& zoneId)
{
    return static_cast<int>(std::count_if(home.structures.begin(), home.structures.end(), [&](const HomeStructure& structure) {
        return structure.zoneId == zoneId;
    }));
}

HomeBodyView BuildBodyView(const CelestialBodyDefinition& body, bool primary, float elapsedSeconds)
{
    HomeBodyView view;
    view.id = body.id;
    view.displayName = body.displayName;
    view.typeName = CelestialBodyTypeName(body.type);
    view.orbitRadius = primary ? 0.0f : std::max(120.0f, body.orbitRadius * 0.75f);
    view.orbitAngleRadians = body.orbitAngleRadians + elapsedSeconds * (primary ? 0.0f : 0.0007f / std::sqrt(std::max(1.0f, view.orbitRadius / 140.0f)));
    view.visualRadius = std::max(8.0f, primary ? body.visualRadius * 0.45f : body.visualRadius * 0.36f);
    view.color = body.palette.primary;
    view.primary = primary;
    view.interactive = true;
    return view;
}

} // namespace

HomeClientViewModel BuildHomeClientViewModel(const RogueliteDirectorState& director, float elapsedSeconds)
{
    HomeClientViewModel view;
    const auto& save = director.save;
    const auto& home = save.home;
    const auto& factory = save.factory;
    const auto& shipyard = save.shipyard;

    view.title = "Home Solar System";
    view.safetyText = std::string("Safety: ") + HomeSafetyModeName(home.config.safetyMode) +
                      (home.config.persistHomeSystem ? " / persistent" : " / transient");
    view.summaryText = HomeSolarSystemSummary(home);
    view.powerText = "Power generation " + std::to_string(EstimateHomePowerGeneration(home)) +
                     " / stored " + std::to_string(home.storedPower);
    view.automationText = "Automation " + std::to_string(home.automationBandwidthUsed) +
                          "/" + std::to_string(home.automationBandwidthCap) +
                          " cap, configured limit " + std::to_string(home.config.automationLimit);
    view.inventoryText = HomeFactoryInventorySummary(factory);
    view.shipyardText = ShipyardProgressionSummary(shipyard);

    const auto production = BuildHomeProductionPlan(home, factory);
    const auto powerGrid = AnalyzeHomePowerGrid(home);
    const auto logistics = AnalyzeHomeLogisticsNetwork(home, production);
    const auto shipyardQueue = CreateStarterShipyardBuildQueue(shipyard);
    view.productionText = HomeProductionPlanSummary(production);
    view.powerGridText = HomePowerGridSummary(powerGrid);
    view.logisticsText = HomeLogisticsSummary(logistics);
    view.shipyardQueueText = ShipyardBuildQueueSummary(shipyardQueue);

    if (!home.systemDefinition.primary.id.empty()) {
        view.bodies.push_back(BuildBodyView(home.systemDefinition.primary, true, elapsedSeconds));
    }
    for (const auto& body : home.systemDefinition.bodies) {
        view.bodies.push_back(BuildBodyView(body, false, elapsedSeconds));
    }

    for (const auto& zone : home.buildZones) {
        HomeBuildZoneView zoneView;
        zoneView.id = zone.id;
        zoneView.displayName = zone.displayName;
        zoneView.typeName = HomeBuildZoneTypeName(zone.type);
        zoneView.resourceSummary = JoinTags(zone.localResourceTags);
        zoneView.gridWidth = zone.gridWidth;
        zoneView.gridHeight = zone.gridHeight;
        zoneView.structureCount = CountStructuresInZone(home, zone.id);
        view.buildZones.push_back(zoneView);
    }

    for (const auto& structure : home.structures) {
        HomeStructureView structureView;
        structureView.id = structure.id;
        structureView.typeName = HomeStructureTypeName(structure.type);
        structureView.zoneId = structure.zoneId;
        structureView.x = structure.x;
        structureView.y = structure.y;
        structureView.tier = structure.tier;
        structureView.powered = structure.powered;
        structureView.automated = structure.automated;
        view.structures.push_back(structureView);
    }

    for (std::size_t i = 0; i < director.availableRuns.size(); ++i) {
        const auto& offer = director.availableRuns[i];
        HomeRunOfferView offerView;
        offerView.index = static_cast<int>(i);
        offerView.displayName = offer.displayName;
        offerView.riskLabel = offer.riskLabel;
        offerView.rewardLabel = offer.rewardLabel;
        offerView.summary = RogueliteRunOfferSummary(offer);
        view.runOffers.push_back(offerView);
    }

    return view;
}

std::string HomeClientViewSummary(const HomeClientViewModel& view)
{
    std::ostringstream stream;
    stream << view.title << " | " << view.safetyText
           << " | bodies=" << view.bodies.size()
           << " zones=" << view.buildZones.size()
           << " structures=" << view.structures.size()
           << " runs=" << view.runOffers.size();
    return stream.str();
}

} // namespace subspace
