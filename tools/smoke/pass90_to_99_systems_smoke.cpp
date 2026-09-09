#include "client/ClientAppScaffold.h"
#include "content/ContentManifest.h"
#include "effects/ThrusterParticleSystem.h"
#include "flight/ShipFlightControl.h"
#include "home/HomeShipBuilderBay.h"
#include "migration/CppConversionBacklog.h"
#include "normalization/ProjectNormalizationPlan.h"
#include "roguelite/RogueliteLoopModel.h"
#include "ships/ShipPartGeneration.h"
#include "travel/InterstellarRailTravel.h"
#include "travel/RailRouteEncounterModel.h"

#include <cassert>
#include <iostream>

int main() {
    using namespace subspace;

    ClientAppScaffoldState client;
    SwitchClientMode(client, ClientMajorMode::HomeSolarSystem, "smoke");
    assert(client.activeMode == ClientMajorMode::HomeSolarSystem);
    assert(!GetClientDecompositionTargets().empty());

    ShipFlightInputFrame input;
    input.thrust = 1.0f;
    input.turn = 1.0f;
    auto flight = EvaluateShipFlightControl(input, ShipFlightControlProfile{});
    assert(flight.mainBurning);
    assert(flight.leftRcsBurning);
    assert(flight.fuelBurnPerSecond > 0.0f);

    ThrusterParticleEmitterState particles;
    EmitThrusterParticles(particles, ThrusterParticleSpawnContext{}, flight, 0.2f);
    assert(CountAliveThrusterParticles(particles) > 0);
    TickThrusterParticles(particles, 0.05f);

    ShipPartGenerationRequest gen;
    gen.category = ShipPartCategory::MainThruster;
    gen.seed = 42;
    auto variants = GenerateShipPartVariants(gen, 3);
    assert(variants.size() == 3);

    auto bay = CreateStarterHomeShipBuilderBay(5000);
    auto bayReport = AnalyzeHomeShipBuilderBay(bay);
    assert(bayReport.canEdit);
    assert(!bayReport.compatiblePartIds.empty());

    auto routes = CreateStarterRailTravelRoutes("home", "expedition", 99);
    assert(!routes.empty());
    auto encounters = GenerateRailRouteEncounters(routes.front(), 77);
    assert(!encounters.empty());

    auto loop = BuildDefaultRogueliteLoopSteps();
    assert(loop.size() >= 5);

    auto backlog = BuildHighLevelCppConversionBacklog();
    assert(!FilterConversionBacklog(backlog, ConversionDisposition::NeedsPort).empty());

    auto plan = BuildDefaultSubspaceNormalizationPlan();
    assert(!plan.rootItems.empty());

    auto manifest = BuildDefaultContentManifestSeed();
    assert(!manifest.empty());

    std::cout << "Pass90-99 systems smoke passed\n";
    std::cout << ClientAppScaffoldSummary(client) << "\n";
    std::cout << ShipFlightControlSummary(flight) << "\n";
    std::cout << RailRouteEncounterSummary(encounters) << "\n";
    std::cout << CppConversionBacklogSummary(backlog) << "\n";
    std::cout << ProjectNormalizationSummary(plan) << "\n";
    std::cout << ContentManifestSummary(manifest) << "\n";
    return 0;
}
