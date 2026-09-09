#include "rendering/VisualPolishProfile.h"
#include "home/HomeSurfacePresentation.h"
#include "home/HomeOutpostExportPlanner.h"
#include "home/HomeFactoryOverlayModel.h"
#include "ships/ModularShipVisualParts.h"
#include "effects/ShipThrusterPortLayout.h"
#include "effects/SpaceVfxProfile.h"
#include "travel/RailTravelPresentation.h"
#include "travel/RailRouteFittingAdvisor.h"
#include "celestial/CelestialInteractionModel.h"
#include "celestial/SolarLightModel.h"
#include "ui/ClientPanelLayoutModel.h"
#include "roguelite/RunPrepChecklist.h"
#include "developer/assets/ConversionNormalizationGate.h"
#include "content/ContentAuthorityRegistry.h"
#include "developer/QualityGateReport.h"
#include "home/HomeDysonProgression.h"
#include "home/HomeHabitableWorldRules.h"
#include "procedural/SectorVisualSeedPlanner.h"
#include "migration/CppConversionRoadmapStatus.h"

#include <cassert>
#include <iostream>

int main() {
    using namespace subspace;
    auto profile = CreateDefaultVisualPolishProfile();
    assert(ResolveLayerIntensity(profile, VisualPolishLayer::ShipThrusters) > 0.0f);
    auto surface = CreateStarterHomeSurfaceFrame(32, 24);
    assert(CountTilesOfKind(surface, HomeSurfaceTileKind::ResourcePatch) > 0);
    auto exports = SummarizeHomeOutpostExports(CreateStarterHomeOutpostNodes());
    assert(exports.totalProductionPerMinute > 0.0f);
    auto overlay = BuildHomeFactoryOverlay(10.0f, 4.0f, 3.0f, 100.0f);
    assert(!overlay.lines.empty());
    auto parts = CreateStarterVisualShipPartCatalog();
    assert(SumVisualPartThrust(parts) > 0.0f);
    auto ports = CreateDefaultThrusterPortLayout(2.0f, 3.0f);
    ThrusterPortFireState fire; fire.forward = true; fire.rotateLeft = true;
    auto firing = ResolveFiringThrusterPorts(ports, fire);
    assert(!firing.empty());
    assert(CreateSpaceVfxProfile(SpaceVfxKind::MiningBeam).glowWidth > 0.0f);
    auto travel = CreateRailTravelPresentationModel("test", 0.5f, 2, 1);
    assert(!ActiveRailTravelSegment(travel).segmentId.empty());
    auto advice = AdviseRailRouteFit({100.0f, 80.0f, 5.0f, 5.0f, 40.0f, 60.0f, 2.0f, 2.0f});
    assert(advice.canLaunch);
    auto zones = CreateDefaultCelestialInteractionZones("planet", 64.0f);
    auto interaction = EvaluateCelestialInteractions(zones, "planet", 50.0f, 5.0f);
    assert(!interaction.availableInteractions.empty());
    auto light = SampleSolarLight({"star", 0.0f, 0.0f, 1.0f}, 100.0f, 0.0f);
    assert(light.intensity > 0.0f);
    auto panels = CreateDefaultClientPanelLayout(true, false, true);
    assert(CountVisibleClientPanels(panels) > 0);
    auto checklist = BuildRunPrepChecklist({100.0f, 20.0f, 80.0f, 25.0f, 2.0f, 2.0f, true, true, true});
    assert(IsRunPrepLaunchAllowed(checklist));
    auto gate = BuildConversionGateReport({{"AvorionLike/Program.cs", "runtime", ConversionGateStatus::Pending, "monolith"}});
    assert(gate.pending == 1);
    auto registry = CreateDefaultContentAuthorityRegistry();
    assert(ResolveContentAuthorityRole(registry, "content/assets/test.png") == ContentAuthorityRole::ActiveRuntime);
    auto report = CreateSubspaceBuildGateReport(true, true, true, true, true);
    assert(QualityGatePassed(report));
    auto dyson = AdvanceHomeDysonProgression({}, 250.0f, 250.0f);
    assert(dyson.energyOutput > 0.0f);
    auto rules = EvaluateHomeHabitableWorldRules({true, false, false, true});
    assert(rules.constructionAllowed && !rules.structureDamageAllowed);
    auto visualSeed = CreateSectorVisualSeedPlan(42u, 2, false);
    assert(!visualSeed.paletteName.empty());
    auto roadmap = CreateCurrentCppConversionRoadmapStatus();
    assert(EstimateOverallCppConversionPercent(roadmap) > 0);
    std::cout << "Pass131-150 bulk systems smoke passed\n";
    return 0;
}
