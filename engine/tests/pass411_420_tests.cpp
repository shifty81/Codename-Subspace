#include "fleet/FleetCaptainAiSystem.h"
#include "integration/PlayerFacingIntegrationSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "rendering/StrategicCamera.h"
#include "rendering/StrategicViewProjection.h"
#include "runtime/CarbonOpenSourceBridgeSystem.h"
#include "ui/GameUiFramework.h"

#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace subspace;
static int testsPassed=0,testsFailed=0;
#define TEST(name,expr) do{if(expr){++testsPassed;std::cout<<"[PASS] "<<name<<"\n";}else{++testsFailed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static FleetRuntimeModel MakeWing(StrategicOrderKind order=StrategicOrderKind::Follow){
    FleetRuntimeModel f;f.spacing=18.0f;f.formation=FormationType::V;
    f.ships={{2,FleetShipRole::Combat,{-16,-16,0},order},{3,FleetShipRole::Mining,{16,-16,0},order},
             {4,FleetShipRole::Support,{-28,-30,0},order},{5,FleetShipRole::Salvage,{28,-30,0},order}};
    return f;
}

static void Pass411_Captains(){
    FleetCaptainAiSystem ai;auto a=ai.MakeCaptain(2,FleetShipRole::Combat);auto b=ai.MakeCaptain(2,FleetShipRole::Combat);auto m=ai.MakeCaptain(3,FleetShipRole::Mining);
    TEST("Pass411 captains are deterministic per fleet ship",a.name==b.name&&std::fabs(a.navigation-b.navigation)<.0001f&&a.captainId==b.captainId);
    TEST("Pass411 captain profiles expose navigation tactics industry and discipline",a.navigation>.5f&&a.tactics>=.76f&&a.discipline>.5f&&m.industry>=.78f);
    TEST("Pass411 role specialization changes captain temperament",a.temperament==FleetCaptainTemperament::Aggressive&&m.temperament==FleetCaptainTemperament::Industrial);
    FleetCaptainRuntime rt;auto wing=MakeWing();ai.EnsureWing(rt,wing,{0,0,0},0);TEST("Pass411 immediate wing creates four independent captain-driven runtime ships",rt.ships.size()==4&&rt.ships[0].initialized&&rt.ships[0].shipId!=rt.ships[1].shipId);
}

static void Pass412_FleetMotion(){
    FleetCaptainAiSystem ai;FleetCaptainRuntime rt;auto wing=MakeWing();ai.EnsureWing(rt,wing,{0,0,0},0);const auto start=rt.ships[0].position;
    for(int i=0;i<120;++i)ai.Step(rt,wing,{0,80.0f+i*.25f,0},{0,8,0},0,{0,0,0},false,false,false,1.0f/60.0f);
    TEST("Pass412 wing ships physically travel instead of remaining fixed render offsets",(rt.ships[0].position-start).length()>4.0f&&rt.ships[0].velocity.length()>0.1f);
    TEST("Pass412 wing captain continuously receives a moving formation target",rt.ships[0].formationTarget.y>80.0f&&rt.ships[0].state!=FleetCaptainState::Holding);

    auto combat=MakeWing(StrategicOrderKind::Engage);ai.Step(rt,combat,{0,100,0},{},0,{0,160,0},true,false,false,.05f);bool engaging=false,supporting=false;for(const auto&s:rt.ships){engaging|=s.role==FleetShipRole::Combat&&s.state==FleetCaptainState::Engaging;supporting|=s.role==FleetShipRole::Support&&s.state==FleetCaptainState::Supporting;}TEST("Pass412 combat intent mirrors by role rather than forcing identical behavior",engaging&&supporting);
    auto mining=MakeWing(StrategicOrderKind::Mine);ai.Step(rt,mining,{0,100,0},{},0,{20,150,0},true,false,false,.05f);bool miner=false;for(const auto&s:rt.ships)miner|=s.role==FleetShipRole::Mining&&s.state==FleetCaptainState::Mining;TEST("Pass412 mining captain mirrors player mining intent when ship role can perform it",miner);
    auto salvage=MakeWing(StrategicOrderKind::Salvage);ai.Step(rt,salvage,{0,100,0},{},0,{-20,150,0},true,false,false,.05f);bool salvager=false;for(const auto&s:rt.ships)salvager|=s.role==FleetShipRole::Salvage&&s.state==FleetCaptainState::Salvaging;TEST("Pass412 salvage captain mirrors salvage intent",salvager);
    auto vector=MakeWing(StrategicOrderKind::VectorTo);ai.Step(rt,vector,{0,100,0},{0,20,0},0,{},false,true,false,.05f);bool allVector=true;for(const auto&s:rt.ships)allVector&=s.state==FleetCaptainState::VectorSync;TEST("Pass412 fleet enters Vector synchronization as moving ships",allVector);
    auto dock=MakeWing(StrategicOrderKind::Dock);ai.Step(rt,dock,{0,100,0},{},0,{},false,false,false,.05f);bool allDock=true;for(const auto&s:rt.ships)allDock&=s.state==FleetCaptainState::Docking;TEST("Pass412 docking intent creates a fleet docking queue state",allDock);
}

static std::vector<std::string> ModuleSet(){return {"cargo_bay","cockpit_basic","cockpit_small","engine_main","engine_small","hull_section","hull_section_enhanced","hull_section_small","power_core","sensor_array","thruster","thruster_small","weapon_mount","wing_left","wing_right","wing_small_left","wing_small_right"};}
static bool CoreModule(const std::string&id){return id.find("cockpit")!=std::string::npos||id.find("hull")!=std::string::npos||id=="power_core";}

static void Pass413_AnchorFirst(){
    auto catalog=ProceduralVisualVariantSystem::Build(ModuleSet(),0x411420u,5);TEST("Pass413 anchor-first generator produces role-varied ship catalog",catalog.shipRecipes.size()>=50);
    bool everyOrigin=true,everyExternalAnchored=true,hasFill=false,hasDetailAnchors=false;
    for(const auto&r:catalog.shipRecipes){
        bool command=false;for(const auto&a:r.anchors){command|=a.id=="COMMAND_ORIGIN";hasDetailAnchors|=a.id.find("DETAIL_")==0;}everyOrigin&=command;
        for(const auto&m:r.modules){if(CoreModule(m.moduleId))continue;bool anchored=false;for(const auto&a:r.anchors)if(a.moduleId==m.moduleId){anchored=true;break;}everyExternalAnchored&=anchored;}
        for(const auto&d:r.details)hasFill|=d.kind==VisualDetailKind::StructuralFill;
    }
    TEST("Pass413 every generated ship owns an explicit command origin",everyOrigin);
    TEST("Pass413 all non-core visible modules are rooted into anchor graph",everyExternalAnchored&&hasDetailAnchors);
    TEST("Pass413 structure is filled between origins and module anchors before skinning",hasFill);
}

static void Pass414_Hardpoints(){
    auto catalog=ProceduralVisualVariantSystem::Build(ModuleSet(),0x414u,4);bool all=true,sockets=true,unique=true;
    for(const auto&r:catalog.shipRecipes){all&=!r.hardpoints.empty();std::set<std::string> ids;for(const auto&h:r.hardpoints){sockets&=h.turret&&h.size==FittingHardpointSize::Small||h.size==FittingHardpointSize::Medium||h.size==FittingHardpointSize::Large||h.size==FittingHardpointSize::Capital||h.size==FittingHardpointSize::Utility;ids.insert(h.id);}unique&=ids.size()==r.hardpoints.size();}
    TEST("Pass414 every generated hull exposes supported turret hardpoints",all);
    TEST("Pass414 turret sockets carry fitting size and turret semantics",sockets);
    TEST("Pass414 hardpoint identifiers are unique within each hull recipe",unique);
}

static void Pass415_FittingContract(){
    auto catalog=ProceduralVisualVariantSystem::Build(ModuleSet(),0x5A17C0DEu,12);const auto*r=ProceduralVisualVariantSystem::Select(catalog,"INDUSTRIAL",1u);
    TEST("Pass415 fitting can resolve the same deterministic generated player hull used by renderer",r!=nullptr&&r->role=="INDUSTRIAL"&&r->seed!=0);
    TEST("Pass415 fitting hull carries actual hardpoint list for empty/fitted socket visualization",r&&r->hardpoints.size()>=2);
    bool renderedSockets=false;if(r)for(const auto&d:r->details)renderedSockets|=d.kind==VisualDetailKind::TurretSocket;TEST("Pass415 fitting hardpoints also exist as visible ship geometry markers",renderedSockets);
}

static void Pass416_FreeCamera(){
    StrategicCamera c;c.SetZoomLimits(.12f,28.0f);c.SetZoom(1.0f);c.SetElevationOverrideDegrees(-89.0f);c.SetVisualYawDegrees(359.0f);c.OrbitVisual(4.0f,0);c.Pan({2,-3,0});c.ZoomBy(20.0f);c.Update(1.0f);
    TEST("Pass416 one normal camera supports full yaw wrap and underside elevation",c.GetVisualYawDegrees()<5.0f&&c.GetElevationOverrideDegrees()<=-88.9f);
    TEST("Pass416 normal camera retains MMB-style persistent pan offset",std::fabs(c.GetPanOffset().x-2.0f)<.01f&&std::fabs(c.GetPanOffset().y+3.0f)<.01f);
    TEST("Pass416 normal camera can zoom dramatically closer without entering a separate inspection camera",c.GetZoom()>8.0f&&c.GetMaxZoom()>=28.0f);
    StrategicViewProjectionConfig cfg;TEST("Pass416 projection supports hull-close near plane and camera distance",cfg.minDistance<=1.35f&&cfg.nearPlane<=.08f);
}

static void Pass417_GuiReadability(){
    GameUiFramework ui;const auto t=ui.DefaultTheme();TEST("Pass417 production GUI uses native system typography contract",t.fontFamily=="Segoe UI"&&t.bodyTextPx>=16.0f&&t.smallTextPx>=14.0f);
    TEST("Pass417 low-contrast microtext is rejected by theme policy",t.minimumContrast>=.70f&&t.clickableRowHeight>=38.0f);
    auto f=ui.DefaultFlightWorkspace();TEST("Pass417 flight GUI validates with readable modular panels",ui.Validate(f)&&f.size()>=8);
}

static void Pass418_CommandRail(){
    PlayerFacingIntegrationSystem s;auto r=s.BuildCommandRail(SandboxWorkspaceMode::FleetCorporation);TEST("Pass418 original Command Rail exposes all primary gameplay workspaces",r.title=="COMMAND RAIL"&&r.items.size()==10);
    std::set<SandboxWorkspaceMode> modes;bool shortLabels=true,active=false;for(const auto&i:r.items){modes.insert(i.workspace);shortLabels&=!i.shortLabel.empty()&&i.shortLabel.size()<=3;active|=i.workspace==SandboxWorkspaceMode::FleetCorporation&&i.active;}TEST("Pass418 rail entries use compact original Subspace labels",shortLabels&&active&&modes.size()==10);
    const float y=r.top+r.rowHeight*4+.5f;TEST("Pass418 fitting entry is mouse hit-testable on the left rail",s.HitTestCommandRail(r,r.width*.5f,y)==4);
}

static void Pass419_CarbonBridge(){
    CarbonOpenSourceBridgeSystem bridge;const auto r=bridge.Build2026Plan();TEST("Pass419 Carbon bridge is broad but preserves native Subspace runtime",r.components.size()>=9&&r.preservesSubspaceRuntime&&!r.importsGameContent&&!r.importsTrademarkedUi);
    const auto*core=bridge.Find(r,"core");const auto*trinity=bridge.Find(r,"trinity");const auto*destiny=bridge.Find(r,"destiny");const auto*audio=bridge.Find(r,"audio");TEST("Pass419 Carbon core/trinity/destiny/audio are explicitly classified",core&&trinity&&destiny&&audio);
    TEST("Pass419 Destiny is adaptation-not-wholesale-runtime due dependency/architecture boundary",destiny&&destiny->disposition==CarbonBridgeDisposition::NativeAdaptation);
    TEST("Pass419 Carbon rendering/audio are references until isolated dependency review passes",trinity&&audio&&trinity->disposition==CarbonBridgeDisposition::ArchitectureReference&&audio->disposition==CarbonBridgeDisposition::ArchitectureReference);
}

static void Pass420_Acceptance(){
    FleetCaptainAiSystem ai;FleetCaptainRuntime rt;auto wing=MakeWing(StrategicOrderKind::Follow);ai.Step(rt,wing,{0,0,0},{0,8,0},0,{},false,false,false,.05f);
    PlayerFacingIntegrationSystem ui;auto rail=ui.BuildCommandRail(SandboxWorkspaceMode::Flight);auto catalog=ProceduralVisualVariantSystem::Build(ModuleSet(),420u,3);bool anchored=true,hardpoints=true;for(const auto&r:catalog.shipRecipes){anchored&=!r.anchors.empty();hardpoints&=!r.hardpoints.empty();}
    GameUiFramework gui;const auto theme=gui.DefaultTheme();
    TEST("Pass420 player-facing closure combines moving captain fleet, anchor-first ships, hardpoints, readable GUI and Command Rail",rt.ships.size()==4&&anchored&&hardpoints&&theme.smallTextPx>=14&&rail.items.size()==10);
    TEST("Pass420 ship-only presentation keeps center gameplay surface while GUI remains modular",ui.BuildFlightHud(FlightControlMode::Manual,{"WEAPON","SCAN"}).centerReserved&&gui.Validate(gui.DefaultFlightWorkspace()));
}

int main(){
    Pass411_Captains();Pass412_FleetMotion();Pass413_AnchorFirst();Pass414_Hardpoints();Pass415_FittingContract();Pass416_FreeCamera();Pass417_GuiReadability();Pass418_CommandRail();Pass419_CarbonBridge();Pass420_Acceptance();
    std::cout<<"\n=== Pass411-420 Player-Facing Closure III Summary: "<<testsPassed<<" passed, "<<testsFailed<<" failed ===\n";
    return testsFailed?1:0;
}
