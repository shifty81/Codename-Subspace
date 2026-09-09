#include "core/persistence/HomeSystemSaveGame.h"
#include "expedition/ExpeditionRun.h"
#include "home/HomeFactoryNetwork.h"
#include "home/HomeShipyardProgression.h"
#include "home/HomeSolarSystem.h"
#include "roguelite/RogueliteDirector.h"

#include <iostream>
#include <stdexcept>

using namespace subspace;

static void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    HomeSolarSystemState home = CreateDefaultHomeSolarSystem(1234);
    Require(IsHomeSystemSafe(home), "default home should be safe");
    Require(!home.buildZones.empty(), "home should have build zones");
    Require(EstimateHomePowerGeneration(home) > 0, "home should generate starter power");

    HomeFactoryNetworkState factory = CreateStarterHomeFactoryNetwork(home);
    Require(GetHomeInventoryUnits(factory, "ore") > 0, "starter factory should have ore");
    HomeFactoryTickReport tick = TickHomeFactoryNetwork(factory, 30.0f);
    Require(tick.cyclesCompleted > 0, "factory should complete at least one cycle");

    ShipyardProgressionState shipyard = CreateStarterShipyardProgression();
    Require(EstimateShipyardBuildCapacity(shipyard) >= 1, "shipyard should have build capacity");

    ExpeditionRunConfig config;
    config.runId = "smoke-run";
    config.seed = 42;
    config.objective = ExpeditionObjectiveType::MiningSurvey;
    ExpeditionRunStateSnapshot run = CreateExpeditionRun(config);
    StartExpeditionRun(run);
    TickExpeditionRun(run, 1000.0f);
    AddExpeditionCargoReward(run, {"ore", 5, 10});
    MarkExpeditionExtracted(run);
    Require(run.state == ExpeditionRunState::Extracted, "run should extract");

    HomeSystemSaveSnapshot save = CreateDefaultHomeSystemSave(5678);
    const std::string serialized = SerializeHomeSystemSaveSnapshot(save);
    HomeSystemSaveSnapshot parsed = DeserializeHomeSystemSaveSnapshot(serialized);
    Require(parsed.saveVersion == save.saveVersion, "home save version should round-trip");

    RogueliteDirectorState director = CreateRogueliteDirector(999);
    Require(!director.availableRuns.empty(), "director should create run offers");
    ApplyExtractedRunRewards(director, run);
    Require(director.completedRuns == 1, "director should count extracted runs");
    Require(GetHomeInventoryUnits(director.save.factory, "ore") >= 5, "run rewards should feed home inventory");

    std::cout << "Pass67-75 home/roguelite smoke passed\n";
    return 0;
}
