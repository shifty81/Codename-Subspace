#include "home/HomeAutomationScheduler.h"
#include "home/HomeClientViewModel.h"
#include "home/HomeLogisticsNetwork.h"
#include "home/HomePowerGrid.h"
#include "home/HomeProductionPlanner.h"
#include "home/HomeShipyardBuildQueue.h"
#include "home/HomeSurfaceBuilder.h"
#include "home/HomeWorldConfigCatalog.h"
#include "expedition/ExpeditionContractBoard.h"
#include "expedition/ExpeditionRewardResolver.h"
#include "developer/assets/ProjectNormalizationLedger.h"

#include <cassert>
#include <iostream>

int main() {
    using namespace subspace;

    auto director = CreateRogueliteDirector(0x51B5ACEu);
    auto& home = director.save.home;
    auto& factory = director.save.factory;

    AddHomeInventory(factory, "ingot", 100);
    AddHomeInventory(factory, "hull-plate", 100);
    AddHomeInventory(factory, "recovered-parts", 100);
    AddHomeInventory(factory, "module-component", 100);
    AddHomeInventory(factory, "refined-metal", 100);
    AddHomeInventory(factory, "electronics", 25);

    HomeBuildPlacementRequest place;
    place.zoneId = home.buildZones.front().id;
    place.type = HomeStructureType::Extractor;
    place.x = 8;
    place.y = 8;
    place.freeBuild = true;
    auto placed = PlaceHomeStructure(home, factory, place);
    assert(placed.success);
    place.type = HomeStructureType::Refinery;
    place.x = 10;
    assert(PlaceHomeStructure(home, factory, place).success);
    place.type = HomeStructureType::ConveyorHub;
    place.x = 12;
    assert(PlaceHomeStructure(home, factory, place).success);
    place.type = HomeStructureType::StorageDepot;
    place.x = 14;
    assert(PlaceHomeStructure(home, factory, place).success);

    auto production = BuildHomeProductionPlan(home, factory);
    assert(!production.routes.empty());
    auto power = AnalyzeHomePowerGrid(home);
    assert(power.generation >= 0);
    auto logistics = AnalyzeHomeLogisticsNetwork(home, production);
    assert(logistics.totalThroughputPerMinute >= 0.0f);

    auto automation = CreateDefaultHomeAutomationSchedule(home);
    RefreshHomeAutomationSchedule(automation, home, production, logistics);
    TickHomeAutomation(automation, home, factory, 20.0f);
    assert(automation.completedJobs >= 0);

    auto queue = CreateStarterShipyardBuildQueue(director.save.shipyard);
    auto order = CreateShipyardBuildOrder(ShipyardBuildOrderType::Module, "test-module", "Test Module");
    const bool queued = QueueShipyardBuildOrder(queue, factory, order);
    assert(queued);
    TickShipyardBuildQueue(queue, 60.0f);
    assert(!queue.completedOrders.empty());

    auto presets = CreateHomeWorldConfigPresets();
    assert(presets.size() >= 3);

    auto board = GenerateExpeditionContractBoard(home.seed, director.save.shipyard, director.completedRuns);
    assert(!board.contracts.empty());
    auto run = CreateExpeditionRun(board.contracts.front().config);
    StartExpeditionRun(run);
    AddExpeditionCargoReward(run, {"ore", 10, 3});
    auto rewards = ResolveSuccessfulExtraction(run, &board.contracts.front());
    assert(rewards.credits > 0);
    auto failure = ResolveFailedExtraction(run);
    assert(failure.researchData >= 1);

    auto ledger = CreateDefaultSubspaceNormalizationLedger();
    assert(!ledger.entries.empty());
    assert(HasUnreviewedNormalizationEntries(ledger));

    auto view = BuildHomeClientViewModel(director, 30.0f);
    assert(!view.productionText.empty());
    assert(!view.powerGridText.empty());
    assert(!view.logisticsText.empty());
    assert(!view.shipyardQueueText.empty());

    std::cout << "Pass78-87 home systems smoke passed\n";
    return 0;
}
