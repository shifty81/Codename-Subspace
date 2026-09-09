// Production stabilization acceptance suite beginning with Pass316-320.
#include <iostream>
#include <string>
#include <vector>

#include "debug_tools/ProductionPerformanceSystem.h"
#include "procedural/RegionStreamingScheduler.h"
#include "content/ThirdPartyAssetIntakeSystem.h"
#include "developer/provenance/AssetProvenanceManifest.h"
#include "interior/ModularInteriorKitSystem.h"
#include "interior/InteriorConstructionSystem.h"
#include "interior/ShipInteriorSystem.h"
#include "rendering/ForwardSpacePresentationSystem.h"
#include "rendering/CelestialEnvironmentSystem.h"
#include "navigation/TravelArrivalSystem.h"
#include "hangar/DockingExperienceSystem.h"
#include "station/StationActivityPresentationSystem.h"
#include "ui/ProductionInterfaceSystem.h"
#include "navigation/SystemMapSystem.h"
#include "interior/InteriorInteractionSystem.h"
#include "audio/ProductionAudioCueSystem.h"
#include "input/InputBindingProfile.h"
#include "core/persistence/SaveRecoverySystem.h"
#include "core/persistence/RuntimeSaveGame.h"
#include "procedural/GalaxyGenerator.h"

using namespace subspace;
static int testsPassed=0; static int testsFailed=0;
#define TEST(name,expr) do { if(expr){++testsPassed;std::cout<<"  PASS: "<<name<<"\n";}else{++testsFailed;std::cout<<"  FAIL: "<<name<<" ("<<__FILE__<<":"<<__LINE__<<")\n";} } while(0)

static InteriorModuleKit MakeKit() {
    InteriorModuleKit kit;
    kit.kitId = "subspace_modular_interior_v1";
    kit.cellSizeMeters = 2.0;
    kit.deckHeightMeters = 3.0;
    kit.modules = {
        {"floor_2m","space_colony_reference",InteriorModuleKind::Floor,1,1,2.0,3.0,true,false,{}},
        {"wall_2m","space_colony_reference",InteriorModuleKind::Wall,1,1,2.0,3.0,true,false,{}},
        {"door_2m","space_colony_reference",InteriorModuleKind::Door,1,1,2.0,3.0,true,false,{{"door_a",InteriorSocketDirection::East,1,0,0,"interior"}}},
        {"machine_mount","space_colony_reference",InteriorModuleKind::MachineryMount,1,1,2.0,3.0,true,false,{}}
    };
    return kit;
}

static void TestPass316ProductionProfiling() {
    std::cout << "[Pass316ProductionProfiling]\n";
    ProductionPerformanceSystem perf;
    TEST("Pass316 developer overlay stays off by default", !perf.DeveloperOverlayVisible());
    perf.RecordFrame({15.9,7.2,4.1,1.1,1024*1024,850,420,3});
    perf.RecordFrame({18.2,7.8,4.4,1.5,2*1024*1024,870,438,3});
    auto healthy = perf.Snapshot();
    TEST("Pass316 records frame samples", healthy.sampleCount==2);
    TEST("Pass316 captures render/simulation-facing counters", healthy.entityCount==870&&healthy.drawCalls==438&&healthy.gpuSubmissions==3);
    TEST("Pass316 healthy sequence has no hitch", healthy.hitchCount==0);
    perf.RecordFrame({41.0,10.2,6.4,4.2,6*1024*1024,910,470,4});
    auto hitch = perf.Snapshot();
    TEST("Pass316 critical frame becomes a hitch", hitch.hitchCount==1&&hitch.health==FrameHealth::Critical);
    TEST("Pass316 component budget violations are classified", hitch.renderBudgetViolations==1&&hitch.simulationBudgetViolations==1&&hitch.streamingBudgetViolations==1&&hitch.allocationBudgetViolations==1);
    TEST("Pass316 normal HUD telemetry remains explicitly opt-in", !perf.DeveloperOverlayVisible());
}

static void TestPass317BudgetedStreaming() {
    std::cout << "[Pass317BudgetedStreaming]\n";
    BeltMacroRegion region; region.seed=317; region.id="belt_317"; region.localAsteroidBudget=36;
    RegionStreamingSystem streaming;
    RegionStreamingScheduler scheduler({2,96,2.0});
    auto plan = scheduler.BuildFramePlan(region,{0,0},2,{});
    TEST("Pass317 full radius-two window requests 25 cells", plan.requestedCells==25);
    TEST("Pass317 frame plan admits at most two cells", plan.scheduled.size()<=2);
    TEST("Pass317 excess streaming work is deferred", plan.deferredCells>=23);
    TEST("Pass317 nearest center cell receives first priority", !plan.scheduled.empty()&&plan.scheduled.front().key.x==0&&plan.scheduled.front().key.y==0);
    TEST("Pass317 estimated asteroid work stays budgeted", plan.estimatedAsteroids<=96);
    TEST("Pass317 estimated streaming time stays budgeted", plan.estimatedWorkMs<=2.0);
    auto cells = scheduler.MaterializePlan(region,plan,streaming);
    TEST("Pass317 scheduler delegates deterministic materialization", cells.size()==plan.scheduled.size()&&!cells.empty()&&!cells.front().asteroids.empty());
    std::vector<RegionCellKey> loaded{{0,0}};
    auto next = scheduler.BuildFramePlan(region,{0,0},1,loaded);
    TEST("Pass317 already-loaded center is not re-requested", !next.scheduled.empty() && !(next.scheduled.front().key.x==0&&next.scheduled.front().key.y==0));
}

static void TestPass318GovernedThirdPartyIntake() {
    std::cout << "[Pass318GovernedThirdPartyIntake]\n";
    AssetProvenanceManifest manifest;
    ThirdPartyAssetIntakeSystem intake;
    ThirdPartyAssetDescriptor d;
    d.assetId="Space Colony Modular Kit Bash";
    d.title="Space Colony Modular Kit Bash";
    d.author="R-LAB";
    d.sourceUrl="https://sketchfab.com/3d-models/space-colony-modular-kit-bash-1ac694729ad04977ba93dafa66914aff";
    d.licenseId="CC-BY";
    d.licenseUrl="https://creativecommons.org/licenses/by/4.0/";
    d.attributionText="Space Colony Modular Kit Bash by R-LAB";
    d.checksumSha256=std::string(64,'a');
    d.upstreamVersion="source-capture";
    d.sourcePath="incoming/space_colony_modular_kit/";
    auto result=intake.ValidateAndRegister(d,manifest);
    TEST("Pass318 complete provenance is accepted",result.accepted&&result.errors.empty());
    TEST("Pass318 canonical source is isolated from derivatives",result.canonicalSourcePath!=result.derivedPath&&result.canonicalSourcePath.find("third_party")!=std::string::npos&&result.derivedPath.find("derived")!=std::string::npos);
    const auto* record=manifest.Find("space_colony_modular_kit_bash");
    TEST("Pass318 manifest preserves author/source/license",record&&record->author=="R-LAB"&&record->license=="CC-BY"&&!record->sourceUrl.empty());
    TEST("Pass318 manifest preserves checksum and attribution",record&&record->checksumSha256.size()==64&&!record->attributionText.empty());
    TEST("Pass318 imported source remains marked unmodified",record&&record->modificationStatus=="UNMODIFIED_SOURCE");
    d.checksumSha256="bad";
    auto rejected=intake.ValidateAndRegister(d,manifest);
    TEST("Pass318 missing valid checksum fails closed",!rejected.accepted&&!rejected.errors.empty());
    d.checksumSha256=std::string(64,'b'); d.sourceReadOnly=false;
    rejected=intake.ValidateAndRegister(d,manifest);
    TEST("Pass318 writable third-party source fails closed",!rejected.accepted);
}

static void TestPass319ModularInteriorNormalization() {
    std::cout << "[Pass319ModularInteriorNormalization]\n";
    auto kit=MakeKit(); ModularInteriorKitSystem system;
    auto validation=system.Validate(kit);
    TEST("Pass319 complete floor/wall/traversal kit validates",validation.valid);
    auto machine=system.Normalize(kit.modules.back(),kit);
    TEST("Pass319 machinery module receives gameplay mount semantics",machine.gameplayMount);
    auto door=system.Normalize(kit.modules[2],kit);
    TEST("Pass319 imported module inherits canonical cell size",door.cellSizeMeters==2.0&&door.deckHeightMeters==3.0);
    TEST("Pass319 traversal sockets retain compatibility contract",!door.sockets.empty()&&door.sockets.front().compatibility=="interior");
    auto broken=kit; broken.modules.erase(broken.modules.begin()+1);
    TEST("Pass319 kit without walls fails closed",!system.Validate(broken).valid);
    TEST("Pass319 kind lookup resolves canonical module",system.FindByKind(kit,InteriorModuleKind::Door)!=nullptr);
}

static void TestPass320InteriorConstruction() {
    std::cout << "[Pass320InteriorConstruction]\n";
    auto kit=MakeKit();
    std::vector<InteriorRoomRequest> rooms={
        {InteriorRoomType::Cockpit,3,2,2,2.5},
        {InteriorRoomType::Engineering,4,3,3,8.0},
        {InteriorRoomType::Cargo,4,3,1,1.0},
        {InteriorRoomType::Airlock,2,2,1,0.5}
    };
    InteriorConstructionSystem construction;
    auto plan=construction.BuildLinearPlan(42,kit,rooms,0);
    TEST("Pass320 visual construction plan carries all gameplay rooms",plan.rooms.size()==4);
    TEST("Pass320 layout is explicitly connected",plan.connected);
    TEST("Pass320 layout produces visual placements",plan.placements.size()>30&&plan.totalWidthCells>13);
    int connectors=0; for(const auto& p:plan.placements) if(p.moduleKind==InteriorModuleKind::Door) ++connectors;
    TEST("Pass320 adjacent rooms receive explicit traversal connectors",connectors==3);
    ShipInteriorSystem interiors;
    auto applied=construction.ApplyToGameplay(plan,interiors);
    TEST("Pass320 construction applies to existing gameplay interior authority",applied.applied&&applied.gameplayRoomIds.size()==4);
    const auto* layout=interiors.GetLayout(42);
    TEST("Pass320 gameplay layout remains owned by ShipInteriorSystem",layout&&layout->rooms.size()==4);
    TEST("Pass320 room power/crew semantics survive visual construction",interiors.GetCrewCapacity(42)==7&&layout->rooms[1].powerDemand==8.0);
}


static GalaxySector MakeStabilizationSector(){GalaxyGenerator g(335);g.planetProbability=1.0f;g.minPointsOfInterest=5;g.maxPointsOfInterest=5;auto s=g.GenerateSector(2,3,0);if(!s.planets.empty()){s.planets.front().hasRings=true;s.planets.front().type=PlanetType::GasGiant;s.planets.front().elevatorCandidate=true;}return s;}
static void TestPass321ShipCohesion(){std::cout<<"[Pass321ShipCohesion]\n";ForwardSpacePresentationSystem p;auto player=p.ForShip("Industrial Hauler",true,.22f);auto npc=p.ForShip("Industrial Hauler",false,.22f);TEST("Pass321 player hull cohesion is production grade",player.hullCohesion>.9f);TEST("Pass321 role material hierarchy is explicit",player.materialRoughness>.65f&&player.metallicResponse>.7f);TEST("Pass321 fitted player carries emissive identity",player.emissiveAccent>npc.emissiveAccent);}
static void TestPass322ShipMaterialLod(){std::cout<<"[Pass322ShipMaterialLod]\n";ForwardSpacePresentationSystem p;TEST("Pass322 LOD0 retains full detail scale",p.ForShip("Combat",false,.2f).lodDetailScale==1.0f);TEST("Pass322 fleet LOD reduces detail strongly",p.ForShip("Combat",false,.02f).lodDetailScale<.5f);TEST("Pass322 combat finish differs from industrial",p.ForShip("Combat",false,.2f).materialRoughness<p.ForShip("Industrial",false,.2f).materialRoughness);}
static void TestPass323FunctionalShipFx(){std::cout<<"[Pass323FunctionalShipFx]\n";ForwardSpacePresentationSystem p;auto c=p.ForShip("Combat Frigate",false,.2f);auto h=p.ForShip("Heavy Hauler",false,.2f);TEST("Pass323 combat retros are visibly authoritative",c.retroThrusterIntensity>=1.0f);TEST("Pass323 combat maneuvering arrays exceed hauler response",c.maneuverThrusterIntensity>h.maneuverThrusterIntensity);TEST("Pass323 functional glow remains role differentiated",c.thrusterGlow>h.thrusterGlow);}
static void TestPass324CelestialPolish(){std::cout<<"[Pass324CelestialPolish]\n";auto s=MakeStabilizationSector();CelestialEnvironmentSystem c;auto p=c.ProfileFor(s.planets.front());TEST("Pass324 gas giant receives high surface detail",p.surfaceDetail>.8f);TEST("Pass324 ringed world casts strong ring shadow",p.ringShadowStrength>.5f);TEST("Pass324 populated elevator candidate can show night lights",p.nightLightStrength>0.0f);TEST("Pass324 atmosphere remains layered",p.atmosphereGlow>0&&p.cloudOpacity>0);}
static void TestPass325VectorPolish(){std::cout<<"[Pass325VectorPolish]\n";ForwardSpacePresentationSystem p;auto charge=p.VectorVisual(VectorTravelStage::Charging,.8);auto cruise=p.VectorVisual(VectorTravelStage::Cruise,.5);auto exit=p.VectorVisual(VectorTravelStage::Decelerating,.75);TEST("Pass325 charge has entry flash and audio ramp",charge.entryFlash>.5f&&charge.audioIntensity>.7f);TEST("Pass325 cruise has tunnel flow",cruise.tunnelFlow>.8f&&cruise.tunnelOpacity>.6f);TEST("Pass325 exit reveals destination progressively",exit.destinationReveal>.7f&&exit.exitReveal>.7f);}
static void TestPass326PoiContinuity(){std::cout<<"[Pass326PoiContinuity]\n";auto s=MakeStabilizationSector();SystemDestination d;d.id=326;d.name="Known Wreck";d.type=SystemDestinationType::SalvageSite;d.discovered=true;d.position.localX=2e6;d.position.localY=3e6;TravelArrivalSystem a;auto e=a.Resolve(d,s,18);TEST("Pass326 scene is ready before exit completes",e.localSceneReadyAtExit);TEST("Pass326 known target is visible at exit",e.targetVisibleAtExit);TEST("Pass326 sensor contact precedes full approach",e.sensorContactSeconds>0&&e.sensorContactSeconds<e.finalApproachSeconds);TEST("Pass326 streaming lead is bounded",e.streamLeadSeconds>=1.25&&e.streamLeadSeconds<=8.0);}
static void TestPass327DockingPolish(){std::cout<<"[Pass327DockingPolish]\n";DockingExperienceSystem d;DockingExperienceState s;TEST("Pass327 request assigns real berth",d.Request(s,77,{0,0,0},{0,-12,0},true)&&!s.berthId.empty()&&s.trafficClearance);for(int i=0;i<800&&s.stage!=DockingExperienceStage::Docked;++i){auto pos=i==0?Vector3{0,-12,0}:s.approachWorld;d.Update(s,pos,.1);}TEST("Pass327 docked state prepares hangar",s.stage==DockingExperienceStage::Docked&&s.hangarReady);TEST("Pass327 capture clamps reach full strength",s.captureStrength>=.99);}
static void TestPass328StationActivity(){std::cout<<"[Pass328StationActivity]\n";StationActivityPresentationSystem a;auto small=a.Build(500,false,DockingExperienceStage::Undocked);auto yard=a.Build(4000,true,DockingExperienceStage::Docked);TEST("Pass328 large shipyard has denser traffic",yard.cargoCraft>small.cargoCraft&&yard.trafficDensity>small.trafficDensity);TEST("Pass328 shipyard exposes construction activity",yard.constructionActivity);TEST("Pass328 docked station brightens dock guidance",yard.activeDockLights>small.activeDockLights);}
static void TestPass329HudLayout(){std::cout<<"[Pass329HudLayout]\n";ProductionInterfaceSystem ui;auto m=ui.Build(ProductionContextKind::Station,"Axiom",ShipEmbodimentMode::CockpitControl,DockingExperienceStage::Undocked,false);TEST("Pass329 HUD has top-bar hierarchy",m.topBar.size()>=2&&m.topBar[0]=="FLIGHT");TEST("Pass329 HUD has bottom interaction hints",m.bottomHints.size()>=3);TEST("Pass329 developer telemetry remains hidden",!m.compactDeveloperTelemetry);}
static void TestPass330ContextUx(){std::cout<<"[Pass330ContextUx]\n";ProductionInterfaceSystem ui;auto normal=ui.Build(ProductionContextKind::Station,"Axiom",ShipEmbodimentMode::CockpitControl,DockingExperienceStage::Undocked,false);auto transit=ui.Build(ProductionContextKind::Station,"Axiom",ShipEmbodimentMode::CockpitControl,DockingExperienceStage::Undocked,true);TEST("Pass330 actions are structured",normal.actions.size()==normal.contextActions.size()&&!normal.actions.empty());bool disabled=false,reason=false;for(const auto&a:transit.actions)if(!a.enabled){disabled=true;if(!a.disabledReason.empty())reason=true;}TEST("Pass330 invalid transit actions are disabled",disabled);TEST("Pass330 disabled actions explain why",reason);}
static void TestPass331SystemMapUx(){std::cout<<"[Pass331SystemMapUx]\n";auto s=MakeStabilizationSector();SystemMapSystem maps;auto m=maps.Build(s);auto known=maps.WarpableKnown(m);TEST("Pass331 map derives region labels",!m.nodes.empty()&&!m.nodes.back().regionLabel.empty());if(!known.empty())maps.SelectById(m,known.front().id);auto summary=maps.SelectedSummary(m);TEST("Pass331 selected destination has readable summary",summary.size()>=4);TEST("Pass331 map tracks physical distance metadata",!m.nodes.empty()&&m.nodes.front().distanceFromOrigin>=0);}
static void TestPass332InteriorEngineeringUx(){std::cout<<"[Pass332InteriorEngineeringUx]\n";InteriorInteractionSystem ux;InteriorRoom eng;eng.type=InteriorRoomType::Engineering;eng.pressurized=true;auto damaged=ux.ActionsFor(eng,true,true);auto sound=ux.ActionsFor(eng,false,true);TEST("Pass332 engineering exposes subsystem actions",damaged.size()>=3&&damaged[0].label=="INSPECT ENGINES");TEST("Pass332 repair activates only when damaged",damaged[1].enabled&&!sound[1].enabled);InteriorRoom air;air.type=InteriorRoomType::Airlock;air.pressurized=false;TEST("Pass332 depressurized airlock fails closed",!ux.ActionsFor(air,false,true)[0].enabled);}
static void TestPass333ProductionAudio(){std::cout<<"[Pass333ProductionAudio]\n";ProductionAudioCueSystem a;auto v=a.ForVector(VectorTravelStage::Cruise,.5);auto d=a.ForDocking(DockingExperienceStage::Capture);auto t=a.ForThruster(1.0f,true);TEST("Pass333 Vector cruise has dedicated loop",v.cueId=="vector_cruise"&&v.loop);TEST("Pass333 docking capture has dedicated cue",d.cueId=="dock_capture");TEST("Pass333 reverse thrust has distinct feedback",t.cueId=="retro_thruster"&&t.volume>.8f);}
static void TestPass334RecoveryAndInput(){std::cout<<"[Pass334RecoveryAndInput]\n";InputBindingProfile b=InputBindingProfile::Defaults();TEST("Pass334 defaults expose core flight/navigation bindings",b.KeyFor(InputAction::ThrustForward)=="W"&&b.KeyFor(InputAction::OpenGalaxyMap)=="M");TEST("Pass334 clean defaults have no conflicts",!b.HasConflicts());b.Rebind(InputAction::RequestDock,"M");TEST("Pass334 conflicting rebinding is detectable",b.HasConflicts());RuntimeSaveGameSnapshot snap;snap.saveId="rc0";snap.sectorId="2:3";snap.credits=99;auto payload=SerializeRuntimeSaveGameSnapshot(snap);SaveRecoverySystem r;auto e=r.Wrap(payload,"autosave-a");TEST("Pass334 save recovery envelope validates",r.Validate(e));e.payload+="corrupt";TEST("Pass334 corrupted recovery payload fails closed",!r.Validate(e));}
static void TestPass335Rc0Acceptance(){std::cout<<"[Pass335Rc0Acceptance]\n";ProductionPerformanceSystem perf;perf.RecordFrame({16,7,4,1,1024,500,250,2});ForwardSpacePresentationSystem visual;auto ship=visual.ForShip("Industrial",true,.2f);auto sector=MakeStabilizationSector();SystemMapSystem maps;auto map=maps.Build(sector);StationActivityPresentationSystem activity;auto station=activity.Build(2500,true,DockingExperienceStage::Docked);ProductionAudioCueSystem audio;auto cue=audio.ForVector(VectorTravelStage::Cruise,.5);TEST("Pass335 RC0 performance authority is healthy",perf.Snapshot().health==FrameHealth::Good);TEST("Pass335 RC0 ship presentation is production refined",ship.hullCohesion>.9f&&ship.lod==0);TEST("Pass335 RC0 navigation has real destinations",!maps.WarpableKnown(map).empty());TEST("Pass335 RC0 station presentation is alive",station.cargoCraft>0&&station.serviceDrones>0);TEST("Pass335 RC0 feedback layer is available",!cue.cueId.empty());}

int main(){
    TestPass316ProductionProfiling();
    TestPass317BudgetedStreaming();
    TestPass318GovernedThirdPartyIntake();
    TestPass319ModularInteriorNormalization();
    TestPass320InteriorConstruction();
    TestPass321ShipCohesion();TestPass322ShipMaterialLod();TestPass323FunctionalShipFx();TestPass324CelestialPolish();TestPass325VectorPolish();
    TestPass326PoiContinuity();TestPass327DockingPolish();TestPass328StationActivity();TestPass329HudLayout();TestPass330ContextUx();
    TestPass331SystemMapUx();TestPass332InteriorEngineeringUx();TestPass333ProductionAudio();TestPass334RecoveryAndInput();TestPass335Rc0Acceptance();
    std::cout << "\n=== Pass316-335 Stabilization Summary: " << testsPassed << " passed, " << testsFailed << " failed ===\n";
    return testsFailed ? 1 : 0;
}
