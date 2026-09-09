#include "integration/PlayerFacingIntegrationSystem.h"
#include "rendering/CelestialEnvironmentSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include <cmath>
#include <iostream>
#include <set>

using namespace subspace;
static int testsPassed=0,testsFailed=0;
#define TEST(name,expr) do{if(expr){++testsPassed;std::cout<<"[PASS] "<<name<<"\n";}else{++testsFailed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static void Pass401_Hud(){
    PlayerFacingIntegrationSystem s;
    auto m=s.BuildFlightHud(FlightControlMode::Manual,{"WEAPON","MINING","SCAN","DRONES","REPAIR"});
    TEST("Pass401 manual HUD exposes normalized mode label",m.modeLabel=="MANUAL FLIGHT");
    TEST("Pass401 chat and module rack occupy requested corners",m.chatBottomLeft&&m.hotbarBottomCenter&&m.centerReserved);
    TEST("Pass401 fitted modules populate runtime hotbar instead of generic fake slots",m.moduleSlots.size()==5&&m.moduleSlots[1]=="MINING");
    TEST("Pass401 strategic HUD updates through the same command surface",s.BuildFlightHud(FlightControlMode::Strategic,{}).modeLabel=="STRATEGIC FLIGHT");
}

static void Pass402_Context(){
    PlayerFacingIntegrationSystem s;InteractionContext c;c.kind=ContextObjectKind::Station;c.dockable=true;c.distance=120;
    auto menu=s.OpenContext(c,420,260);TEST("Pass402 RMB station menu opens at pointer with real actions",menu.open&&menu.screenX==420&&menu.actions.size()>=4);
    bool hasDock=false;for(const auto&a:menu.actions)hasDock|=a.id=="dock"&&a.enabled;TEST("Pass402 dock action is present and enabled for dockable station",hasDock);
    const auto before=menu.selected;s.MoveContextSelection(menu,1);TEST("Pass402 context selection can move without closing menu",menu.selected!=before&&menu.open);
    auto* a=s.ActiveContextAction(menu);TEST("Pass402 active action resolves from runtime menu",a!=nullptr);
}

static void Pass403_Strategic(){
    PlayerFacingIntegrationSystem s;ContextAction a{"approach","Approach",true,{}};auto order=s.ToStrategicOrder(a,7,{0,100,0},25);TEST("Pass403 context action converts to strategic order",order.valid&&order.kind==StrategicOrderKind::Approach&&order.targetId==7);
    StrategicFlightSystem flight;flight.SetMode(FlightControlMode::Strategic);flight.Issue(order);auto c=s.EvaluateAutopilot(flight,{0,0,0},0,{0,100,0});TEST("Pass403 strategic autopilot emits physical throttle and heading",c.valid&&c.throttle>0.5f&&std::fabs(c.desiredYawRadians)<0.01f);
    ContextAction dock{"dock","Dock",true,{}};auto d=s.ToStrategicOrder(dock,9,{50,0,0});flight.Issue(d);auto dc=s.EvaluateAutopilot(flight,{0,0,0},0,{50,0,0});TEST("Pass403 dock command retains docking intent",dc.intent.requestDock);
    flight.SetMode(FlightControlMode::Manual);TEST("Pass403 manual mode prevents strategic autopilot takeover",!s.EvaluateAutopilot(flight,{},{},{10,0,0}).valid);
}

static void Pass404_Stations(){
    PlayerFacingIntegrationSystem s;auto contacts=s.BuildStationContacts(44,{100,200,0},300,true);
    TEST("Pass404 populated local system exposes several visible station contacts",contacts.size()>=4);
    bool asteroid=false,dock=false;std::set<std::string>names;for(const auto&c:contacts){asteroid|=c.asteroidEmbedded;dock|=c.dockable;names.insert(c.name);}TEST("Pass404 station ecology includes dockable asteroid installation",asteroid&&dock);
    TEST("Pass404 generated stations retain distinct player-facing identities",names.size()>=3);
    bool offset=true;for(const auto&c:contacts)offset&=(c.position-Vector3{100,200,0}).length()>500;TEST("Pass404 stations are distributed in orbital space instead of stacked at planet center",offset);
}

static void Pass405_Hangar(){
    PlayerFacingIntegrationSystem s;StationEcologySystem e;auto station=e.BuildStartingSystem(9,4,true).front();auto h=s.BuildHangar(station,true,999);
    TEST("Pass405 docked hangar presents actual fitted ship",h.docked&&h.showActualShip&&h.orbitCamera);
    TEST("Pass405 hangar inherits station-specific services",!h.services.empty()&&!h.hangarProfile.empty());
    TEST("Pass405 hangar camera clamps to station presentation contract",h.cameraDistance<=90.0f&&h.cameraDistance>=4.0f);
}

static void Pass406_SystemMap(){
    PlayerFacingIntegrationSystem s;OrbitalBodyRecord star;star.id=1;star.kind=OrbitalBodyKind::Star;star.name="Sun";OrbitalBodyRecord p;p.id=2;p.parentId=1;p.name="World";p.orbit.semiMajorAxis=12000;p.orbit.orbitalPeriodSeconds=1000;OrbitalBodyRecord st;st.id=3;st.parentId=2;st.kind=OrbitalBodyKind::Station;st.name="Dock";st.dockable=true;st.orbit.semiMajorAxis=600;st.orbit.orbitalPeriodSeconds=90;
    auto m=s.BuildSystemMapRuntime({star,p,st},250,64);TEST("Pass406 live System Map carries ephemeris nodes and sampled orbit tracks",m.snapshot.nodes.size()==3&&m.snapshot.nodes[1].orbitTrack.size()==64);
    TEST("Pass406 dockable orbital infrastructure becomes useful default selection",m.snapshot.nodes[m.selected].dockable);
    const float z=m.zoom;s.ZoomSystemMap(m,2);s.PanSystemMap(m,40,-20);TEST("Pass406 System Map supports independent zoom and pan",m.zoom>z&&std::fabs(m.pan.x)>0.1f&&std::fabs(m.pan.y)>0.1f);
    const auto old=m.selected;s.StepSystemMapSelection(m,1);TEST("Pass406 System Map selection cycles across live orbital nodes",m.selected!=old);
}

static void Pass407_Galaxy(){
    PlayerFacingIntegrationSystem s;auto g=s.BuildGalaxyRuntime(1234,10000);TEST("Pass407 player-facing galaxy runtime initializes full 10k-system catalog",g.initialized&&g.catalog.size()==10000);
    TEST("Pass407 arbitrary generated system can become inspected selection",s.SelectGalaxySystem(g,42)&&g.selectedSystem==42);
    auto small=s.BuildGalaxyRuntime(33,180);TEST("Pass407 route planner is wired into runtime galaxy state",s.PlotGalaxyRoute(small,1,80,1500,GalaxyRouteMode::Safest)&&small.route.systems.front()==1&&small.route.systems.back()==80);
}

static void Pass408_Pi(){
    PlayerFacingIntegrationSystem s;PlanetData p;p.name="Kestrel";p.type=PlanetType::Rocky;p.resourceRichness=.8f;p.hazardLevel=.2f;p.industryRepresentation=PlanetIndustryRepresentation::SurfaceHexGrid;auto m=s.BuildPlanetIndustry(p,3,77);HexCoord origin{0,0};TEST("Pass408 PI runtime exposes deterministic surveyed hex state",m.industry.hexes.size()==37&&m.industry.hexes[origin].surveyed);
    TEST("Pass408 selected hex can accept a functional installation",s.PlaceIndustry(m,PiInstallationKind::Storage));
    m.selectedHex={1,0};TEST("Pass408 logistics installation can unlock tether path",s.PlaceIndustry(m,PiInstallationKind::Logistics)&&m.tetherAvailable);
    TEST("Pass408 produced material moves physically/logically into tether storage",s.TickIndustryToTether(m,60,1)>0&&m.tetherStored>0);
}

static void Pass409_Fleet(){
    PlayerFacingIntegrationSystem s;std::vector<FleetWingShip>w={{1,FleetShipRole::Combat,true},{2,FleetShipRole::Mining,true},{3,FleetShipRole::Support,true},{4,FleetShipRole::Salvage,true},{5,FleetShipRole::Combat,true},{6,FleetShipRole::Combat,true}};FleetFlightConfig cfg;auto f=s.BuildFleet(w,cfg,StrategicOrderKind::Engage);TEST("Pass409 immediate runtime fleet remains capped to four wing ships",f.ships.size()==4);
    TEST("Pass409 combat intent selects combat formation",f.formation==FormationType::Wedge);
    bool separated=true;for(std::size_t i=1;i<f.ships.size();++i)separated&=(f.ships[i].desiredOffset-f.ships[0].desiredOffset).length()>1;TEST("Pass409 wing ships receive distinct formation-relative offsets",separated);
    auto mining=s.BuildFleet(w,cfg,StrategicOrderKind::Mine);TEST("Pass409 mining intent switches the live formation",mining.formation==FormationType::Line);
}

static void Pass410_Acceptance(){
    PlayerFacingIntegrationSystem s;PlayerFacingAcceptanceState a;a.hudIntegrated=a.contextCommands=a.strategicAutopilot=a.stationVisibleAndDockable=a.hangarIntegrated=a.liveSystemMap=a.galaxyRuntime=a.planetaryIndustryRuntime=a.fleetRuntime=a.shipOnly=true;auto r=s.EvaluateAcceptance(a);TEST("Pass410 integrated ship-only player-facing vertical closes at 100",r.pass&&r.score==100&&r.blockers.empty());a.liveSystemMap=false;TEST("Pass410 missing major runtime surface blocks acceptance",!s.EvaluateAcceptance(a).pass);
}

static void Pass410R3_RuntimeCorrections(){
    PlayerFacingIntegrationSystem ui;ContextMenuRuntimeModel menu;menu.open=true;menu.screenX=1500;menu.screenY=800;menu.actions={{"approach","Approach",true,{}},{"dock","Dock",true,{}}};
    const auto layout=ui.LayoutContextMenu(menu,1600,900);
    TEST("Pass410R3 context menu is clamped, enlarged and screen readable",layout.textScale>=1.8f&&layout.rowHeight>=38.0f&&layout.x+layout.width<=1591.0f);
    TEST("Pass410R3 context menu rows are mouse hit-testable",ui.HitTestContextMenu(menu,1600,900,layout.x+30,layout.y+layout.headerHeight+layout.rowHeight*.5f)==0);
    SystemMapRuntimeModel sm;sm.zoom=.10f;TEST("Pass410R3 outward system-map zoom can promote into galaxy hierarchy",ui.ShouldPromoteSystemMapToGalaxy(sm,-1.0f));

    PlanetData giant;giant.type=PlanetType::GasGiant;giant.radius=1500.0f;giant.hasRings=true;CelestialEnvironmentSystem env;const Vector3 center{100,50,0};
    const auto safe=env.ProjectOutsideLocalOrbit(center,center,giant,64.0f);
    TEST("Pass410R3 spawn projection guarantees player is outside giant/ring envelope",(safe-center).length()>=env.SafeLocalOrbitRadius(giant,64.0f)-.5f);

    GalaxyGenerator gen(77123);gen.eventStageProbability=0.0f;gen.planetProbability=1.0f;gen.asteroidBeltProbability=1.0f;
    const auto system=gen.GenerateSector(3,4,0);bool stellar=false;for(const auto&b:system.asteroidBelts)stellar|=b.beltClass==AsteroidBeltClass::Circumstellar;
    TEST("Pass410R3 regenerated normal systems carry first-class circumstellar asteroid belts",stellar);
    OrbitalDynamicsSystem dyn;const auto bodies=dyn.DeriveSystemOrbits(system);bool liveBelt=false;for(const auto&b:bodies)liveBelt|=b.kind==OrbitalBodyKind::BeltObject;
    TEST("Pass410R3 asteroid belts participate in live orbital/System Map authority",liveBelt);

    gen.gasGiantBeltProbability=1.0f;bool foundGiant=false,foundGiantBelt=false;
    for(int x=0;x<24&&!foundGiantBelt;++x){auto s=gen.GenerateSector(x,9,0);for(const auto&p:s.planets)if(p.type==PlanetType::GasGiant){foundGiant=true;for(const auto&b:s.asteroidBelts)if(b.beltClass==AsteroidBeltClass::PlanetaryDebris&&b.parentPlanetId==p.planetId)foundGiantBelt=true;}}
    TEST("Pass410R3 gas giants can own parent-relative mineable debris belts",foundGiant&&foundGiantBelt);
}

int main(){Pass401_Hud();Pass402_Context();Pass403_Strategic();Pass404_Stations();Pass405_Hangar();Pass406_SystemMap();Pass407_Galaxy();Pass408_Pi();Pass409_Fleet();Pass410_Acceptance();Pass410R3_RuntimeCorrections();std::cout<<"\n=== Pass401-410 Player-Facing Integration II Summary: "<<testsPassed<<" passed, "<<testsFailed<<" failed ===\n";return testsFailed?1:0;}
