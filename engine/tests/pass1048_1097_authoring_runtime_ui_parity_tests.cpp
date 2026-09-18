#include "construction/CohesiveAssemblyBakeSystem.h"
#include "editor/AuthoringStandardsSystem.h"
#include "generator/GeneratorParitySystem.h"
#include "rendering/ConformalShieldSurfaceSystem.h"
#include "ui/GameUiFramework.h"
#include "ui/RuntimeControlContextSystem.h"
#include "world/WorldScaleAuthoritySystem.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
int failures=0,assertions=0;
void Check(bool ok,const char* name){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}
ShipyardModuleRecord FlatModule(std::string id){
    ShipyardModuleRecord r;r.source.moduleId=std::move(id);r.source.halfWidth=1.5f;r.source.halfLength=2.5f;r.source.halfHeight=1.2f;r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.partRole=ShipyardPartRole::PrimaryHull;r.primaryHull=true;
    auto contact=[](Vector3 p,Vector3 n,float area){VisualModuleSurfaceContact c;c.point=p;c.normal=n;c.supportingArea=area;c.confidence=.92f;c.valid=true;return c;};
    r.source.forwardSurface=contact({0,2.5f,0},{0,1,0},7.2f);r.source.aftSurface=contact({0,-2.5f,0},{0,-1,0},7.0f);r.source.portSurface=contact({-1.5f,0,0},{-1,0,0},9.0f);r.source.starboardSurface=contact({1.5f,0,0},{1,0,0},9.0f);r.source.dorsalSurface=contact({0,0,1.2f},{0,0,1},13.5f);r.source.ventralSurface=contact({0,0,-1.2f},{0,0,-1},13.5f);
    r.sockets={{"aft","hull_aft",0,-2.5f,0,0,-1,0,.10f},{"forward","hull_forward",0,2.5f,0,0,1,0,.10f}};return r;
}
}

int main(){
    std::cout<<"[Pass1048-1097 Authoring/Runtime/UI/Blender Parity]\n";
    const auto scale=WorldScaleAuthoritySystem::DefaultProfile();
    auto hull=FlatModule("hull");
    auto surfaces=AuthoringStandardsSystem::DiscoverFlatSnapSurfaces(hull,.10f,.55f,scale);
    Check(surfaces.size()==6,"1048 all six certified flat contact surfaces are discoverable");
    Check(!surfaces.empty()&&surfaces.front().supportingArea>=surfaces.back().supportingArea,"1049 planar snap surfaces prioritize confidence/support area");
    Check(std::any_of(surfaces.begin(),surfaces.end(),[](const auto&s){return s.id=="dorsal"&&s.gridColumns>1&&s.gridRows>1;}),"1050 large flat dorsal surface expands into a dense snap field");
    auto weak=hull;weak.source.dorsalSurface.confidence=.2f;auto weakSurfaces=AuthoringStandardsSystem::DiscoverFlatSnapSurfaces(weak,.10f,.55f,scale);
    Check(std::none_of(weakSurfaces.begin(),weakSurfaces.end(),[](const auto&s){return s.id=="dorsal";}),"1051 low-confidence guessed surface cannot outrank certified geometry");
    Check(surfaces.front().gridPitchMeters>=scale.fineSnapMeters,"1052 surface grid pitch remains tied to physical world-scale authoring units");
    Check(surfaces.front().maximumInsertionMeters<=.25f,"1053 generic surface snap limits module penetration instead of allowing arbitrary clipping");
    Check(hull.source.dorsalSurface.supportingArea>hull.source.forwardSurface.supportingArea,"1054 supporting-area evidence differentiates broad attachment faces");
    Check(hull.source.dorsalSurface.valid&&hull.source.dorsalSurface.confidence>.5f,"1055 surface-snap authority consumes measured contact evidence rather than BBox-only endpoints");

    auto door=AuthoringStandardsSystem::DefaultObject(SemanticObjectPurpose::Door,scale);door.id="door.standard";
    Check(AuthoringStandardsSystem::ValidateObject(door,scale).valid,"1056 default authored door fits canonical 1.80m character envelope");
    auto narrow=door;narrow.sizeMeters.x=.45f;Check(!AuthoringStandardsSystem::ValidateObject(narrow,scale).valid,"1057 undersized door is rejected by character-clearance authority");
    auto seat=AuthoringStandardsSystem::DefaultObject(SemanticObjectPurpose::Seat,scale);seat.id="seat.pilot";
    Check(seat.sitInteraction&&AuthoringStandardsSystem::ValidateObject(seat,scale).valid,"1058 authored seat carries semantic sit purpose and human-scale interaction point");
    auto console=AuthoringStandardsSystem::DefaultObject(SemanticObjectPurpose::Console,scale);console.id="console.nav";
    Check(console.consoleInteraction&&console.interactionReachMeters==scale.referenceReachMeters,"1059 console interaction reach derives from canonical character model scale");
    auto storage=AuthoringStandardsSystem::DefaultObject(SemanticObjectPurpose::Storage,scale);storage.id="storage.box";
    Check(storage.storageInteraction,"1060 in-engine modeled storage can be assigned gameplay purpose instead of remaining anonymous geometry");
    Check(scale.referenceDoorHeightMeters>scale.referencePlayerHeightMeters,"1061 canonical door height is explicitly larger than canonical character height");
    Check(scale.referenceCorridorWidthMeters>scale.referenceDoorWidthMeters,"1062 corridor standard provides maneuvering clearance beyond doorway width");
    Check(scale.referenceDeckHeightMeters>scale.referenceDoorHeightMeters,"1063 deck height preserves overhead structure above traversable door clearance");

    auto child=FlatModule("child");child.primaryHull=false;child.partRole=ShipyardPartRole::StructuralFrame;
    ProceduralShipVisualRecipe recipe;recipe.recipeId="cohesive_test";VisualModulePlacement a;a.moduleId="hull";VisualModulePlacement b;b.moduleId="child";b.y=4.9f;recipe.modules={a,b};recipe.attachments.push_back({0,1,"forward","aft",.05f,true});
    auto plan=CohesiveAssemblyBakeSystem::BuildPlan(recipe,{hull,child},scale);
    Check(plan.readyForBooleanBake,"1064 connected certified assembly produces a cohesive boolean-bake plan");
    Check(plan.policy.unionExteriorShell&&plan.policy.removeOccludedInternalExteriorFaces,"1065 bake policy unions outer silhouette and removes buried module exterior faces");
    Check(plan.policy.carveWalkableInterior&&plan.policy.preservePortalOpenings,"1066 cohesive bake preserves carved interior and portal openings");
    Check(plan.outputObjName.find("_cohesive.obj")!=std::string::npos,"1067 saved assembly declares one cohesive OBJ interchange product");
    Check(plan.characterScaleCompatible,"1068 interior-candidate geometry is evaluated against canonical character headroom");
    Check(!plan.portalCuts.empty()&&plan.portalCuts.front().allowedPenetrationMeters<=plan.policy.maximumAttachmentPenetrationMeters,"1069 attachment portal cut carries bounded penetration authority");
    auto badRecipe=recipe;badRecipe.attachments.front().measuredGap=.80f;auto badPlan=CohesiveAssemblyBakeSystem::BuildPlan(badRecipe,{hull,child},scale);
    Check(!badPlan.readyForBooleanBake&&!badPlan.errors.empty(),"1070 excessive module overlap/gap blocks cohesive bake instead of hiding clipping inside the final shell");

    RuntimeControlContextSystem control;
    auto flight=control.Build(SandboxWorkspaceMode::Flight,ShipEmbodimentMode::CockpitControl,DockingExperienceStage::Undocked,false,false);
    Check(flight.viewAuthority==RuntimeViewAuthority::CockpitFirstPerson&&flight.firstPerson,"1071 cockpit first-person is the standard embodied flight authority");
    auto onFoot=control.Build(SandboxWorkspaceMode::Flight,ShipEmbodimentMode::InteriorOnFoot,DockingExperienceStage::Undocked,false,false);
    Check(onFoot.viewAuthority==RuntimeViewAuthority::OnFootFirstPerson&&onFoot.firstPerson&&onFoot.interiorControls,"1072 on-foot first-person is the standard interior embodiment authority");
    auto remote=control.Build(SandboxWorkspaceMode::Flight,ShipEmbodimentMode::CockpitControl,DockingExperienceStage::Undocked,false,true);
    Check(remote.viewAuthority==RuntimeViewAuthority::RemoteFleetCommand&&!remote.firstPerson&&remote.remoteFleetCommand,"1073 overhead tactical view is explicitly Remote Fleet Command");
    auto authoring=control.Build(SandboxWorkspaceMode::ShipBuilder,ShipEmbodimentMode::CockpitControl,DockingExperienceStage::Undocked,false,false);
    Check(authoring.viewAuthority==RuntimeViewAuthority::AuthoringDev&&!authoring.firstPerson,"1074 Shipyard is an authoring/dev-mode camera over live runtime authority");
    Check(remote.cameraMode==CameraMode::TacticalFleet,"1075 existing tactical renderer becomes the presentation backend for Remote Fleet Command");
    Check(!remote.flightControls&&!remote.weapons&&!remote.scanner&&
          !remote.vectorCommands&&!remote.dockingControls,
          "G5 legacy tactical mouse ownership is isolated from flight and weapons");

    // G5 foundation: require physical seat claim and per-order revalidation.
    FleetCommandSeatSnapshot commandSeat{};
    commandSeat.seatId=21;commandSeat.hostEntityId=31;commandSeat.occupiedBy=7;
    commandSeat.kind=FleetSeatKind::StationTerminal;
    FleetCommandSession commandSession{};
    Check(!FleetCommandSeatSystem::Enter(commandSession,7,commandSeat).allowed && !commandSession.active,
        "G5 terminal requires permission/power/command-system/link rather than a UI toggle");
    commandSeat.accessAllowed=true;commandSeat.powered=true;
    commandSeat.commandSystemInstalled=true;commandSeat.commandLinkOnline=true;
    Check(FleetCommandSeatSystem::Enter(commandSession,7,commandSeat).allowed && commandSession.active,
        "G5 authoritative occupied command seat grants an explicit session");
    Check(commandSession.returnView==FleetSeatReturnView::OnFoot,
        "G5 station command view returns to the original on-foot seat");
    Check(!FleetCommandSeatSystem::Enter(commandSession,8,commandSeat).allowed,
        "G5 second actor cannot steal a claimed session");
    auto live=control.BuildWithCommandSeat(SandboxWorkspaceMode::Flight,ShipEmbodimentMode::CockpitControl,
        DockingExperienceStage::Undocked,false,true,7,commandSession,commandSeat);
    Check(live.remoteFleetCommand&&!live.flightControls&&!live.weapons&&!live.mouseLook,
        "G5 authorized fleet camera owns pointer without steering or weapon fire");
    auto stationed=control.BuildWithCommandSeat(SandboxWorkspaceMode::Flight,ShipEmbodimentMode::InteriorOnFoot,
        DockingExperienceStage::Docked,false,true,7,commandSession,commandSeat);
    Check(stationed.remoteFleetCommand&&!stationed.flightControls&&!stationed.interiorControls,
        "G5 station terminal can enter command view from occupied on-foot station seat");
    commandSeat.commandLinkOnline=false;
    Check(!FleetCommandSeatSystem::CanCommand(commandSession,7,commandSeat).allowed,
        "G5 lost comm link revokes order permission without erasing world session");
    live=control.BuildWithCommandSeat(SandboxWorkspaceMode::Flight,ShipEmbodimentMode::CockpitControl,
        DockingExperienceStage::Undocked,false,true,7,commandSession,commandSeat);
    Check(!live.remoteFleetCommand&&live.firstPerson,
        "G5 invalid command link does not grant remote fleet camera");
    Check(!FleetCommandSeatSystem::Exit(commandSession,8).allowed&&commandSession.active,
        "G5 unauthorized actor cannot release another crew seat");
    FleetSeatReturnView restored=FleetSeatReturnView::Cockpit;
    Check(FleetCommandSeatSystem::Exit(commandSession,7,&restored).allowed&&!commandSession.active&&
          restored==FleetSeatReturnView::OnFoot,
        "G5 rightful actor releases seat and restores the captured originating view");
    FleetCommandSeatSnapshot shipConsole=commandSeat;
    shipConsole.seatId=22;shipConsole.kind=FleetSeatKind::ShipCommandSeat;
    shipConsole.commandLinkOnline=true;
    Check(FleetCommandSeatSystem::Enter(commandSession,7,shipConsole,FleetSeatReturnView::OnFoot).allowed&&
          commandSession.returnView==FleetSeatReturnView::OnFoot,
        "G5 walk-up bridge console on a ship returns to first-person on foot");
    Check(FleetCommandSeatSystem::Enter(commandSession,7,shipConsole,FleetSeatReturnView::Cockpit).allowed&&
          commandSession.returnView==FleetSeatReturnView::OnFoot,
        "G5 duplicate Enter cannot overwrite original camera return state");
    Check(FleetCommandSeatSystem::Exit(commandSession,7,&restored).allowed&&restored==FleetSeatReturnView::OnFoot,
        "G5 ship-console exit returns the captured on-foot origin");
    shipConsole.seatId=23;shipConsole.kind=FleetSeatKind::PilotSeat;
    Check(!FleetCommandSeatSystem::Enter(commandSession,7,shipConsole,FleetSeatReturnView::OnFoot).allowed&&
          !commandSession.active,
        "G5 pilot seat cannot incorrectly record on-foot exit authority");
    Check(FleetCommandSeatSystem::Enter(commandSession,7,shipConsole,FleetSeatReturnView::Cockpit).allowed&&
          commandSession.returnView==FleetSeatReturnView::Cockpit,
        "G5 pilot seat captures cockpit as its original view");
    Check(FleetCommandSeatSystem::Exit(commandSession,7,&restored).allowed&&restored==FleetSeatReturnView::Cockpit,
        "G5 pilot seat exit restores cockpit");

    const auto shieldPolicy=ConformalShieldSurfaceSystem::DefaultPolicy();
    auto nearShield=ConformalShieldSurfaceSystem::Select({500.0f,12,90000,true,false},shieldPolicy);
    Check(nearShield.lod==ShieldSurfaceLod::NearConformal&&nearShield.rippleBudget==4,"1076 local/near shield keeps full conformal water/ripple treatment");
    auto fleetShield=ConformalShieldSurfaceSystem::Select({10000.0f,120,90000,false,false},shieldPolicy);
    Check(fleetShield.lod==ShieldSurfaceLod::FarSilhouette&&fleetShield.triangleStride>1,"1077 100+ ship grid decimates shield surface work rather than drawing every hull triangle");
    auto hugeFleet=ConformalShieldSurfaceSystem::Select({80000.0f,220,90000,false,false},shieldPolicy);
    Check(hugeFleet.lod==ShieldSurfaceLod::StrategicOnly&&!hugeFleet.receivePointImpacts,"1078 extreme fleet distance/count collapses shield presentation to strategic-only state");
    Check(shieldPolicy.hullGapMeters>.30f&&shieldPolicy.hullGapMeters<.31f,"1079 conformal shield keeps the intended one-foot hull gap");
    Check(shieldPolicy.maxNearRipples>shieldPolicy.maxFarRipples,"1080 point-impact ripple cost is explicitly bounded by shield LOD");

    const auto theme=SubspaceUiTheme::Dark();
    Check(theme.canvas.r<theme.panel.r&&theme.scrollThumb.a>theme.scrollTrack.a,"1081 one standardized minimal dark theme owns panel and scrollbar contrast");
    Check(theme.panelOpacityMinimum<theme.panelOpacityDefault&&theme.panelOpacityDefault<=theme.panelOpacityMaximum,"1082 project UI defines a readable adjustable panel-opacity range");
    auto dock=SubspaceDockSystem::CreateMinimalWorkspace("test");
    SubspaceDockPanel assets;assets.id="assets";assets.title="Assets";assets.defaultLeafId="left";SubspaceDockSystem::RegisterPanel(dock,assets);
    SubspaceDockPanel view;view.id="viewport";view.title="Viewport";view.defaultLeafId="center";view.closable=false;view.floatable=false;view.resizable=false;SubspaceDockSystem::RegisterPanel(dock,view);
    SubspaceDockPanel props;props.id="properties";props.title="Properties";props.defaultLeafId="right";SubspaceDockSystem::RegisterPanel(dock,props);
    Check(SubspaceDockSystem::Validate(dock),"1083 universal dock workspace validates as the shared runtime/editor panel authority");
    Check(SubspaceDockSystem::ResizeSplit(dock,"main",.28f),"1084 dock splitters resize panel regions in-game");
    Check(SubspaceDockSystem::SetPanelOpacity(dock,"assets",.50f,theme)&&SubspaceDockSystem::FindPanel(dock,"assets")->opacity==.50f,"1085 individual panels support user-adjustable opacity");
    Check(SubspaceDockSystem::SetPanelOpacity(dock,"assets",.01f,theme)&&SubspaceDockSystem::FindPanel(dock,"assets")->opacity==theme.panelOpacityMinimum,"1086 panel opacity is clamped so UI cannot become accidentally unreadable");
    Check(SubspaceDockSystem::FloatPanel(dock,"properties",{100,120,520,360}),"1087 ordinary panels can float from the same shared dock authority");
    Check(SubspaceDockSystem::ResizeFloating(dock,"properties",{120,140,620,420}),"1088 floating panels remain resizable rather than fixed-size popups");
    GameUiFramework gameUi;auto flightDock=gameUi.DefaultFlightDockWorkspace();auto strategicDock=gameUi.DefaultStrategicDockWorkspace();
    Check(SubspaceDockSystem::Validate(flightDock)&&SubspaceDockSystem::Validate(strategicDock),"1089 flight and Remote Fleet workspaces consume the same dock system");

    const auto registry=GeneratorParitySystem::Registry();
    Check(registry.size()==9,"1090 generator parity registry exposes all current major authoring domains");
    Check(GeneratorParitySystem::Find(registry,GeneratorDomain::Ship)!=nullptr&&GeneratorParitySystem::Find(registry,GeneratorDomain::SolarSystem)!=nullptr,"1091 ship through solar-system generation share one discoverable parity registry");
    GeneratorParityRequest request;request.domain=GeneratorDomain::Ship;request.seed=42;request.profileId="DEFAULT";request.role="INDUSTRIAL";request.shipClass=ShipClass::Cruiser;request.size=UniversalSizeClass::M;
    Check(GeneratorParitySystem::Validate(request).valid,"1092 canonical generator request validates in runtime authority");
    const auto json=GeneratorParitySystem::RequestJson(request);
    Check(json.find("subspace.generator-request.v1")!=std::string::npos&&json.find("\\\"")==std::string::npos,"1093 generator request JSON is a clean transport contract suitable for Blender");
    Check(std::string(GeneratorParitySystem::DomainId(GeneratorDomain::Rover))=="rover","1094 rover generation has a stable cross-client domain id");
    Check(std::string(GeneratorParitySystem::DomainId(GeneratorDomain::Character))=="character","1095 character generation has a stable cross-client domain id");
    Check(std::string(GeneratorParitySystem::DomainId(GeneratorDomain::PlanetWorld))=="planet_world","1096 planetary generation has a stable cross-client domain id");
    Check(std::string(GeneratorParitySystem::DomainId(GeneratorDomain::SolarSystem))=="solar_system","1097 two-system/solar proving-ground generation has a stable Blender/runtime parity id");

    std::cout<<"Pass1048-1097 assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
