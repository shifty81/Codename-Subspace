#include "station/StationDockingGeometrySystem.h"
#include "station/StationNavigationLightSystem.h"
#include "station/StationHangarPresentationSystem.h"
#include "station/StationServiceEnvelopeSystem.h"
#include "combat/FireControlSystem.h"
#include "combat/TacticalTargetingSystem.h"
#include "navigation/ObservableWarpSystem.h"
#include "rendering/PlanetWeatherSystem.h"
#include "rendering/GasGiantWeatherSystem.h"
#include "rendering/PlanetAtmospherePresentationSystem.h"
#include "rendering/StrategicCamera.h"
#include "ship_editor/ShipyardBuildSafetySystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardTransformSystem.h"
#include "ship_editor/ShipyardDragDropSystem.h"
#include "ship_editor/ShipyardOrientationConstraintSystem.h"
#include "ships/ShipProgressionSystem.h"
#include "ui/TacticalContactsSystem.h"
#include "ui/RuntimeWindowLayoutSystem.h"
#include "ui/ShipCommandHudSystem.h"
#include "effects/PropulsionVisualSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
static int failures=0;
static int assertions=0;
static void Check(bool ok,const char* name){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}
static bool Near(float a,float b,float e=.001f){return std::fabs(a-b)<=e;}

int main(){
    std::cout<<"[Pass476To495Systems]\n";

    // 476-478: station geometry, visual guidance, hangar/service infrastructure.
    StationDockingGeometrySystem docking;
    const auto dock=docking.Build(42,{100,50,0},{100,-100,0},StationBerthSize::Heavy);
    Check(!dock.berthId.empty()&&dock.corridor.size()==4,"station berth has persistent id and four-stage approach corridor");
    Check((dock.apertureWorld-dock.stationWorld).length()>1.0f,"station dock aperture is physical exterior geometry, not station center");
    Check((dock.undockWorld-dock.stationWorld).length()>(dock.apertureWorld-dock.stationWorld).length(),"undock point sits outside aperture");
    const Vector3 desired=(dock.apertureWorld-dock.captureWorld).normalized();
    const auto capture=docking.Evaluate(dock,dock.captureWorld,{0,0,0},desired);
    Check(capture.capturable&&capture.alignment>.99f,"aligned slow ship can be physically captured at berth");
    const auto tooFast=docking.Evaluate(dock,dock.captureWorld,{50,0,0},desired);
    Check(!tooFast.capturable&&!tooFast.speedAcceptable,"capture rejects excessive docking speed");

    StationNavigationLightSystem lights;
    const auto clearLights=lights.Build(dock,true,false);
    Check(clearLights.size()>20,"station approach builds readable nav-light corridor and envelope");
    Check(std::any_of(clearLights.begin(),clearLights.end(),[](const auto& l){return l.kind==DockNavLightKind::Aperture;}),"station nav lights identify docking aperture");
    const auto closedLights=lights.Build(dock,true,true);
    Check(std::any_of(closedLights.begin(),closedLights.end(),[](const auto& l){return l.kind==DockNavLightKind::Closed;}),"occupied berth changes guidance to closed state");

    StationHangarPresentationSystem hangars;
    const auto construction=hangars.Build(StationArchetype::Shipyard,StationBerthSize::Capital);
    Check(construction.constructionFrame&&construction.externalClamp,"capital shipyard berth exposes construction frame/external clamp presentation");
    Check(construction.width>80.0f&&construction.serviceArms>=8,"capital hangar scales beyond normal berth");
    const auto service=StationServiceEnvelopeSystem::Build(StationArchetype::Shipyard,true);
    Check(service.radius>70.0f&&service.repairPerSecond>0&&service.defenseCoverage,"authorized shipyard exposes useful service envelope");
    Check(StationServiceEnvelopeSystem::Build(StationArchetype::Shipyard,false).radius==0.0f,"unauthorized ship receives no station service envelope");

    // 479-480: lock-first targeting with manual mouse precision bias.
    TacticalTargetingState targets; targets.maxTargets=2;
    TacticalTargetReference hostile{1,3,"hostile-3"};
    Check(TacticalTargetingSystem::Request(targets,hostile),"Ctrl-style target request creates target acquisition slot");
    TacticalTargetingSystem::Tick(targets,1.3f,1.25f);
    Check(TacticalTargetingSystem::IsLocked(targets,hostile),"target acquisition reaches locked state");

    FireControlRequest fire;fire.mode=FireControlMode::LockedManualAim;fire.targetLocked=false;fire.triggerHeld=true;
    fire.muzzleWorld={0,0,0};fire.lockedTargetWorld={0,100,0};fire.pointerWorld={12,100,0};fire.projectileSpeed=100;
    const auto blocked=FireControlSystem::Solve(WeaponType::BeamArray,fire);
    Check(!blocked.valid&&!blocked.mayFire&&blocked.lockRequired,"normal directed weapon refuses manual fire without target lock");
    fire.targetLocked=true;
    const auto aimed=FireControlSystem::Solve(WeaponType::BeamArray,fire);
    Check(aimed.valid&&aimed.mayFire&&aimed.aimPoint.x>0.1f,"locked manual fire biases weapon aim toward mouse pointer");
    FireControlRequest freeReq=fire;freeReq.targetLocked=false;freeReq.mode=FireControlMode::FreeFire;
    Check(!FireControlSystem::Solve(WeaponType::BeamArray,freeReq).valid,"weapon policy blocks unsupported free fire");
    Check(FireControlSystem::PolicyFor(WeaponType::InwardFlak).allowsFreeFire,"explicit free-fire weapon policy remains available");

    // 481-482: normalized size progression and license/training gate.
    const auto progression=ShipProgressionSystem::CoreSizeProgression();
    Check(std::find(progression.begin(),progression.end(),ShipClass::Shuttle)!=progression.end(),"core ship progression includes Shuttle");
    Check(std::find(progression.begin(),progression.end(),ShipClass::Battlecruiser)!=progression.end(),"core ship progression closes Cruiser-to-Battleship gap with Battlecruiser");
    Check(std::find(progression.begin(),progression.end(),ShipClass::Dreadnought)!=progression.end(),"core ship progression includes Dreadnought capital");
    CharacterShipTraining rookie;rookie.trainingRank=2;rookie.navigationRank=2;rookie.licenses={"FRIGATE"};
    Check(ShipProgressionSystem::CanPilot(rookie,ShipClass::Frigate).allowed,"trained/licensed pilot can fly matching hull");
    Check(!ShipProgressionSystem::CanPilot(rookie,ShipClass::Battlecruiser).allowed,"high-class hull remains gated by training/license");

    // 483: camera-relative Shipyard pan.
    StrategicCamera cam;cam.SetVisualYawDegrees(0);const auto right0=cam.ViewRightPlanar();cam.SetVisualYawDegrees(90);const auto right90=cam.ViewRightPlanar();
    Check(std::fabs(right0.x-right90.x)>.5f||std::fabs(right0.y-right90.y)>.5f,"Shipyard view-right basis follows camera yaw");
    cam.ClearPanOffset();cam.PanViewRelative(2.0f,0.0f);const auto pan90=cam.GetPanOffset();
    Check(std::fabs(pan90.y)>1.0f&&std::fabs(pan90.x)<.2f,"MMB pan moves in camera-relative viewing plane after orbit");

    // 484-487: transform/rotate/drag/socket/orientation authoring authority.
    VisualModulePlacement place;place.moduleId="wing";place.x=.03f;place.rollDegrees=90.0f;
    ShipyardTransformTransaction tx;
    Check(ShipyardTransformSystem::Begin(tx,0,place,ShipyardTransformTool::Move),"Shipyard transform transaction begins on selected module");
    ShipyardTransformSystem::Translate(tx,{.12f,.03f,0},false);const auto moved=ShipyardTransformSystem::Commit(tx);
    Check(Near(std::fmod(std::fabs(moved.x),.10f),0.0f,.001f)||Near(std::fmod(std::fabs(moved.x),.10f),.10f,.001f),"Shipyard move transaction applies translation snapping");
    ShipyardTransformSystem::Begin(tx,0,moved,ShipyardTransformTool::Rotate);
    ShipyardTransformSystem::Rotate(tx,{0,0,-80},false);const auto rotated=ShipyardTransformSystem::Commit(tx);
    Check(Near(rotated.rollDegrees,15.0f),"Shipyard Rotate tool commits snapped module rotation");
    ShipyardTransformSystem::Begin(tx,0,rotated,ShipyardTransformTool::Move);ShipyardTransformSystem::Translate(tx,{4,4,4},false);const auto cancelled=ShipyardTransformSystem::Cancel(tx);
    Check(Near(cancelled.x,rotated.x)&&Near(cancelled.y,rotated.y),"Shipyard transform cancel restores pre-drag placement");

    ShipyardModuleRecord parent;parent.source.moduleId="parent";parent.source.halfWidth=1;parent.source.halfLength=1;parent.source.halfHeight=1;
    parent.sockets.push_back({"starboard-port","lateral_surface",1,0,0,1,0,0,.05f});
    ShipyardModuleRecord child;child.source.moduleId="wing_child";child.source.halfWidth=.5f;child.source.halfLength=.7f;child.source.halfHeight=.2f;child.moduleClass=ShipyardModuleClass::Wing;child.semantic=ShipyardModuleSemantic::Wing;
    child.sockets.push_back({"root","lateral_mount",-.5f,0,0,-1,0,0,.05f});
    ProceduralShipVisualRecipe recipe;VisualModulePlacement pp;pp.moduleId="parent";recipe.modules.push_back(pp);
    auto preview=ShipyardDragDropSystem::Begin(child,{parent,child},recipe);
    Check(preview.active&&preview.valid&&!preview.candidates.empty(),"catalog drag computes compatible socket placement preview");
    Check(preview.selectedCandidate==0&&preview.status=="SNAP READY","drag/drop selects best compatible socket candidate");
    VisualModulePlacement badWing;badWing.moduleId="wing_child";badWing.rollDegrees=90.0f;
    Check(!ShipyardOrientationConstraintSystem::Validate(child,badWing).valid,"wing orientation rules reject accidental vertical mounting");
    ShipyardOrientationConstraintSystem::Normalize(child,badWing);
    Check(std::fabs(badWing.rollDegrees)<=32.01f,"wing normalization clamps invalid roll while preserving manual correction path");
    child.source.moduleId="vertical_fin";const auto finRule=ShipyardOrientationConstraintSystem::RuleFor(child);
    Check(finRule.allowVertical,"FIN/KEEL-classified surface may intentionally mount vertically");

    // 488: build mode is a hard propulsion/weapon interlock.
    InputState input;input.SetAction(InputAction::ThrustForward,true);input.SetAction(InputAction::Boost,true);input.SetAction(InputAction::FirePrimary,true);input.SetAction(InputAction::FireMiningMissile,true);
    ShipyardBuildSafetySystem::SuppressFlightAndWeapons(input);
    Check(!input.IsDown(InputAction::ThrustForward)&&!input.IsDown(InputAction::Boost),"Shipyard build mode hard-disables thrust and boost");
    Check(!input.IsDown(InputAction::FirePrimary)&&!input.IsDown(InputAction::FireMiningMissile),"Shipyard build mode hard-disables weapon fire");

    // 489: propulsion VFX is activity/role/LOD driven rather than a static line.
    const auto nearPlume=PropulsionVisualSystem::Evaluate(ShipyardModuleSemantic::MainEngine,.8f,PropulsionVisualLod::Near,false,false);
    const auto farPlume=PropulsionVisualSystem::Evaluate(ShipyardModuleSemantic::MainEngine,.8f,PropulsionVisualLod::Far,false,false);
    Check(nearPlume.sparkBudget>farPlume.sparkBudget&&nearPlume.ribbon,"near main-engine VFX has richer multi-layer plume budget");
    const auto warpPlume=PropulsionVisualSystem::Evaluate(ShipyardModuleSemantic::MainEngine,1.0f,PropulsionVisualLod::Near,true,true);
    Check(warpPlume.distortion&&warpPlume.ribbon&&warpPlume.bodyLength>nearPlume.bodyLength,"boost/vector-drive state expands cinematic propulsion profile");

    // 490-491: warp produces world-observable evidence with lingering collapse.
    std::vector<ObservableWarpEvent> events;
    ObservableWarpSystem::EmitStageTransition(events,77,VectorTravelStage::Charging,VectorTravelStage::Cruise,{0,0,0},{0,1,0});
    Check(events.size()==3,"warp departure emits streak, transit wake and collapse-ring evidence");
    Check(std::any_of(events.begin(),events.end(),[](const auto&e){return e.kind==WarpEvidenceKind::CollapseRing;}),"warp tunnel collapse remains an explicit observer-visible event");
    ObservableWarpSystem::Tick(events,3.0f);
    Check(events.size()<3&&!events.empty(),"warp evidence persists by lifetime instead of vanishing with player transition");
    ObservableWarpSystem::EmitStageTransition(events,77,VectorTravelStage::Cruise,VectorTravelStage::Decelerating,{0,100,0},{0,1,0});
    Check(std::any_of(events.begin(),events.end(),[](const auto&e){return e.kind==WarpEvidenceKind::ArrivalFlare;}),"warp arrival emits observer-visible emergence evidence");

    // 492-494: atmosphere integration and evolving terrestrial/gas-giant weather.
    const auto atmosphere=PlanetAtmospherePresentationSystem::Default();
    Check(atmosphere.cloudRadiusMultiplier<1.01f&&atmosphere.atmosphereRadiusMultiplier<1.025f,"cloud/atmosphere shells stay tight to planet surface");
    const float centerAlpha=PlanetAtmospherePresentationSystem::Alpha(1.0f,atmosphere,1.0f);
    const float limbAlpha=PlanetAtmospherePresentationSystem::Alpha(0.0f,atmosphere,1.0f);
    Check(limbAlpha>centerAlpha*10.0f,"atmosphere is strongly limb-biased instead of a visible bubble across disc");

    PlanetData ocean;ocean.type=PlanetType::Oceanic;ocean.hazardLevel=.75f;ocean.name="Stormworld";
    auto weather=PlanetWeatherSystem::Initialize(ocean,1234);
    Check(!weather.storms.empty()&&weather.volatility>.4f,"hazardous ocean world initializes active weather cells");
    const float firstLon=weather.storms.front().longitude;PlanetWeatherSystem::Advance(weather,ocean,20.0);
    Check(!weather.storms.empty()&&!Near(weather.storms.front().longitude,firstLon,.00001f),"planet weather cells evolve/move independently over time");

    PlanetData giant;giant.type=PlanetType::GasGiant;giant.hazardLevel=.9f;
    const auto gas=GasGiantWeatherSystem::Build(giant,4321);
    Check(gas.volatileAtmosphere&&gas.bands.size()>=10,"volatile gas giant builds many active atmospheric bands");
    bool opposing=false;for(std::size_t i=1;i<gas.bands.size();++i)if(gas.bands[i-1].angularVelocity*gas.bands[i].angularVelocity<0)opposing=true;
    Check(opposing&&gas.giantVortexProbability>.5f,"gas giant uses counter-rotating shear bands with high vortex potential");

    // 495: EVE-familiar Contacts/window/command-HUD foundations.
    TacticalContactsModel contacts;contacts.preset=TacticalContactPreset::Combat;contacts.rows={{1,TacticalContactKind::HostileShip,"Raider","FRIGATE",12,300,.1f,5,false,false},{2,TacticalContactKind::Station,"Hub","STATION",2,0,0,0,false,false},{3,TacticalContactKind::FriendlyShip,"Wing","FRIGATE",8,220,.02f,0,false,false}};
    Check(TacticalContactsSystem::VisibleInPreset(TacticalContactKind::HostileShip,TacticalContactPreset::Combat),"Contacts Combat preset includes hostile ships");
    Check(!TacticalContactsSystem::VisibleInPreset(TacticalContactKind::Station,TacticalContactPreset::Combat),"Contacts Combat preset filters unrelated station rows");
    TacticalContactsSystem::Sort(contacts);Check(contacts.rows.front().range==2,"Contacts rows sort by range through one tactical model");
    contacts.freezeSort=true;const auto firstId=contacts.rows.front().id;contacts.rows.front().range=99;TacticalContactsSystem::Sort(contacts);Check(contacts.rows.front().id==firstId,"Contacts sorting can freeze during command interaction");

    auto layout=RuntimeWindowLayoutSystem::DefaultFlightLayout(1920,1080);
    Check(layout.windows.size()>=4&&std::any_of(layout.windows.begin(),layout.windows.end(),[](const auto&w){return w.id=="contacts";}),"default flight layout provides persistent Contacts window");
    Check(RuntimeWindowLayoutSystem::Move(layout,"contacts",100,120),"runtime Contacts window can be moved into custom layout");
    Check(RuntimeWindowLayoutSystem::Resize(layout,"contacts",500,600),"runtime Contacts window can be resized");
    Check(RuntimeWindowLayoutSystem::SetMode(layout,"contacts",RuntimeWindowMode::Compact),"runtime Contacts window supports compact mode");

    const auto hud=ShipCommandHudSystem::Build(42.0f,true,true,{"PRIMARY","MINING","SCAN","REPAIR"});
    Check(hud.modules.size()==4&&hud.modules[0].shortcut=="F1"&&hud.modules[3].shortcut=="F4","ship command HUD maps fitted modules to familiar function-key rack");
    Check(hud.modules[0].requiresTarget&&!hud.modules[2].requiresTarget,"ship command HUD differentiates target-required modules from utility modules");
    Check(hud.speed==42.0f&&hud.dampeners&&hud.boost,"ship command HUD carries live speed/dampener/boost state");

    TacticalContactsModel clickable;clickable.maxVisibleRows=10;TacticalContactRow clickableRow;clickableRow.name="Raider";clickableRow.sourceKind=1;clickableRow.sourceIndex=7;clickableRow.sourceId="ship_7";clickable.rows.push_back(clickableRow);
    const auto contactLayout=TacticalContactsSystem::Layout(1600,900,1);
    Check(TacticalContactsSystem::HitTestRow(clickable,1600,900,contactLayout.x+20,contactLayout.RowsY()+8)==0,"Contacts renderer/input share row hit-test geometry");
    Check(TacticalContactsSystem::HitTestPreset(clickable,1600,900,contactLayout.x+5,contactLayout.y+contactLayout.titleHeight+5)==0,"Contacts preset tabs are directly hit-testable");
    Check(TacticalContactsSystem::PresetFromIndex(0)==TacticalContactPreset::Combat&&TacticalContactsSystem::PresetFromIndex(6)==TacticalContactPreset::All,"Contacts preset tab ordering is stable");
    Check(clickable.rows.front().sourceIndex==7&&clickable.rows.front().sourceId=="ship_7","Contacts rows preserve authoritative source identity for selection/locking");

    ShipyardBuilderSystem keyboardBuilder;keyboardBuilder.Initialize({child},ProceduralShipVisualRecipe{});
    // Use a one-module fixture so atomic keyboard/nudge commands can be verified
    // independently from pointer-drag transactions.
    ProceduralShipVisualRecipe keyboardRecipe;VisualModulePlacement keyboardPlacement;keyboardPlacement.moduleId=child.source.moduleId;keyboardRecipe.modules.push_back(keyboardPlacement);keyboardBuilder.Initialize({child},keyboardRecipe);
    Check(keyboardBuilder.Activate(ShipyardBuilderCommand::NudgeStarboard),"Shipyard keyboard nudge executes through transform authority");
    Check(!keyboardBuilder.Model().transform.active&&keyboardBuilder.Recipe().modules.front().x>0.05f,"Shipyard keyboard nudge commits atomically instead of leaving a cancellable drag transaction");

    std::cout<<"Pass476-495 assertions: "<<assertions<<" total / "<<failures<<" failed\n";
    return failures?1:0;
}
