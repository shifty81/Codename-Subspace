#include "ships/ShipAttachmentIntegritySystem.h"
#include "rendering/ShipInspectionReviewSystem.h"
#include "celestial/OrbitalDynamicsSystem.h"
#include "station/StationEcologySystem.h"
#include "navigation/UniverseSystemMapSystem.h"
#include "ships/ShipProfileDirector.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "input/InputState.h"
#include "input/InputBindingProfile.h"
#include "ui/GameUiFramework.h"
#include "ui/ContextActionSystem.h"
#include "flight/StrategicFlightSystem.h"
#include "ui/ShipActionHotbarSystem.h"
#include "ui/ChatCommunicationSystem.h"
#include "hangar/UniversalDockedStationSystem.h"
#include "station/ModularStationAssemblySystem.h"
#include "economy/InfrastructureModuleSystem.h"
#include "navigation/GalaxyCatalogSystem.h"
#include "navigation/GalaxyMapSystem.h"
#include "navigation/GalaxyRoutePlannerSystem.h"
#include "economy/PlanetaryIndustrySystem.h"
#include "fleet/FleetIntentSystem.h"
#include "runtime/ShipOnlyVerticalAcceptanceSystem.h"
#include <cmath>
#include <iostream>
#include <set>

using namespace subspace;
static int testsPassed=0,testsFailed=0;
#define TEST(name,expr) do{if(expr){++testsPassed;std::cout<<"[PASS] "<<name<<"\n";}else{++testsFailed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static void Pass361_362_Attachments(){
    ShipAttachmentIntegritySystem sys;
    std::vector<ShipAttachmentProbe> good={{"engine",ShipMountKind::Pylon,{8,0,0},{4,0,0},2,4,true},{"retro",ShipMountKind::DirectHull,{0,4,0},{0,4,0},1,.05f,true}};
    auto a=sys.Audit(good); TEST("Pass361 universal attachment audit accepts physically mounted modules",a.valid&&a.bridgeMounts==1&&a.directMounts==1);
    good.push_back({"floating",ShipMountKind::Invalid,{20,0,0},{0,0,0},1,20,true});a=sys.Audit(good);TEST("Pass361 detached module is a hard rejection",!a.valid&&a.rejected==1);
    TEST("Pass362 pylon/fairing bridge length is derived from real anchor separation",ShipAttachmentIntegritySystem::SuggestedBridgeLength(good.front())>2.0f);
}
static void Pass363_364_Inspection(){
    ShipInspectionReviewSystem s;ShipInspectionCameraState c;s.Orbit(c,400,-100);TEST("Pass363 inspection camera supports complete yaw and underbelly pitch",c.yawDegrees>=0&&c.yawDegrees<360&&c.pitchDegrees<0);
    s.Pan(c,8,-5);s.Zoom(c,20);TEST("Pass363 MMB-style inspection pan and close zoom authority",c.pan.x>0&&c.pan.y<0&&c.distance<=3.0f);
    s.Snap(c,ShipInspectionSnap::Bottom);TEST("Pass363 bottom inspection snap looks underneath ship",c.pitchDegrees<-80);
    ShipInspectionOverlay o;o.sockets=o.attachmentLines=o.invalidModules=true;TEST("Pass364 visual QA overlay exposes sockets attachment paths and invalid modules",s.OverlayLegend(o).size()==3);
}
static void Pass365_368_Orbits(){
    OrbitalDynamicsSystem d;StarData sun;sun.radius=1200;auto safety=d.StarSafety(sun);TEST("Pass365 stellar exclusion radius exceeds visual/corona radius",safety.spawnRadius>safety.hazardRadius&&safety.hazardRadius>safety.visualRadius);
    TEST("Pass365 positions inside enlarged star spawn envelope are rejected",!d.IsOutsideStarSafety({100,0,0},{0,0,0},safety));
    OrbitalBodyRecord p;p.id=2;p.parentId=1;p.name="Planet";p.orbit.semiMajorAxis=10000;p.orbit.eccentricity=.1;p.orbit.orbitalPeriodSeconds=1000;p.orbit.inclinationDegrees=6;
    auto p0=d.Evaluate(p,0),p1=d.Evaluate(p,250);TEST("Pass366 Keplerian body position evolves continuously with simulation time",(p1-p0).length()>1000);
    OrbitalBodyRecord station;station.id=3;station.parentId=2;station.kind=OrbitalBodyKind::Station;station.orbit.semiMajorAxis=500;station.orbit.orbitalPeriodSeconds=80;auto sp=d.Evaluate(station,20,p1);TEST("Pass367 station orbit is evaluated in moving planet reference frame",(sp-p1).length()>400&&(sp-p1).length()<600);
    auto intercept=d.PredictIntercept(p,{},100,75);TEST("Pass368 Vector navigation predicts moving target at arrival time",intercept.valid&&intercept.arrivalTimeSeconds==175&&((intercept.predictedTarget-d.Evaluate(p,100)).length()>100));
    GalaxySector sector;sector.hasStar=true;sector.star.name="Refine Star";sector.star.radius=1200;PlanetData gp;gp.name="Refine Giant";gp.type=PlanetType::GasGiant;gp.radius=700;gp.surfaceSeed=42;gp.position={9000,0,0};sector.planets.push_back(gp);sector.hasStation=true;sector.station.name="Refine Dock";
    auto hierarchy=d.DeriveSystemOrbits(sector);bool moon=false,planetParent=false,stationParent=false;for(const auto& body:hierarchy){moon|=body.kind==OrbitalBodyKind::Moon&&body.parentId==100;planetParent|=body.kind==OrbitalBodyKind::Planet&&body.parentId==1;stationParent|=body.kind==OrbitalBodyKind::Station&&body.parentId==100;}
    TEST("Pass366 refinement derives planet-relative moons and station hierarchy",moon&&planetParent&&stationParent);
}
static void Pass369_371_StationsAndMap(){
    StationEcologySystem e;auto s=e.BuildStartingSystem(44,4,true);TEST("Pass369 populated starting system guarantees multiple dockable station experiences",s.size()>=3&&s[0].dockable&&s[1].dockable);
    bool tether=false;for(auto&x:s)tether|=x.archetype==StationArchetype::TetherTerminal;TEST("Pass369 starting system includes planetary tether infrastructure",tether);
    auto ast=e.BuildAsteroidStation(99,4);TEST("Pass370 asteroid station is explicitly embedded and dockable",ast.asteroidEmbedded&&ast.orbitClass==StationOrbitClass::AsteroidEmbedded&&ast.dockable);
    OrbitalBodyRecord star;star.id=1;star.kind=OrbitalBodyKind::Star;star.name="Sun";OrbitalBodyRecord p;p.id=2;p.parentId=1;p.name="World";p.orbit.semiMajorAxis=10000;p.orbit.orbitalPeriodSeconds=1000;OrbitalBodyRecord st;st.id=3;st.parentId=2;st.name="Dock";st.kind=OrbitalBodyKind::Station;st.dockable=true;st.orbit.semiMajorAxis=500;st.orbit.orbitalPeriodSeconds=80;UniverseSystemMapSystem map;auto m=map.Build({star,p,st},123,32);TEST("Pass371 Universe-style system map exposes current positions and orbit tracks",m.nodes.size()==3&&m.nodes[1].orbitTrack.size()==32&&m.nodes[2].dockable);
}
static void Pass372_378_ShipProfiles(){
    ShipProfileDirector d;auto combat=d.Build(ShipRoleProfile::Combat,1);auto hauler=d.Build(ShipRoleProfile::Hauler,2);auto mining=d.Build(ShipRoleProfile::Mining,3);
    TEST("Pass372 ship profile grammar closes clean nose-to-tail proportions",std::fabs(combat.noseRatio+combat.hullRatio+combat.utilityRatio+combat.propulsionRatio-1.0f)<.02f);
    TEST("Pass373 role-driven hull profiles produce materially different massing",hauler.utilityRatio>combat.utilityRatio*2.0f&&mining.widthToLength>combat.widthToLength);
    std::set<int> cockpits;for(unsigned i=1;i<40;++i)cockpits.insert(int(d.Build(ShipRoleProfile::Exploration,i).cockpit));TEST("Pass374 generator exposes multiple cockpit/command families",cockpits.size()>=6);
    TEST("Pass375 command and propulsion sections retain meaningful profile share",combat.noseRatio>=.15f&&combat.propulsionRatio>=.2f);
    TEST("Pass376 controlled negative space is first-class profile data",mining.negativeSpace>=.18f&&hauler.negativeSpace>=.10f);
    std::set<std::string> makers;for(unsigned i=1;i<30;++i)makers.insert(d.Build(ShipRoleProfile::Patrol,i).manufacturer);TEST("Pass377 manufacturer visual DNA varies across procedural catalog",makers.size()>=5);
    std::vector<std::string> errors;TEST("Pass378 production profile director validates representative generated ships",d.Validate(combat,&errors)&&d.Validate(hauler)&&d.Validate(mining)&&combat.profileScore>=88);

    // Pass361-400 Refinement R1: cockpit/profile authority now reaches the
    // actual procedural render recipes instead of living only in profile data.
    const std::vector<std::string> modules={"cargo_bay","cockpit_basic","cockpit_small","engine_main","engine_small","hull_section","hull_section_enhanced","hull_section_small","power_core","sensor_array","thruster","thruster_small","weapon_mount","wing_left","wing_right","wing_small_left","wing_small_right"};
    auto visual=ProceduralVisualVariantSystem::Build(modules,0x51A9u,12);
    std::set<std::string> visualCockpits;std::size_t accepted=0,withBridges=0;
    for(const auto& recipe:visual.shipRecipes){
        visualCockpits.insert(recipe.cockpitFamily);accepted+=recipe.acceptedByArtDirector?1u:0u;
        bool bridge=false;for(const auto& detail:recipe.details)bridge|=detail.kind==VisualDetailKind::MountBridge;withBridges+=bridge?1u:0u;
    }
    TEST("Pass374 refinement drives at least six cockpit families into rendered procedural recipes",visualCockpits.size()>=6);
    TEST("Pass378 refinement keeps generated render catalog above production art-director floor",accepted==visual.shipRecipes.size());
    TEST("Pass361 refinement gives procedural recipes real mount-bridge geometry",withBridges>visual.shipRecipes.size()/2);
}
static void Pass379_385_CommandUi(){
    GameUiFramework ui;auto workspace=ui.DefaultFlightWorkspace();TEST("Pass379 one normalized Subspace GUI theme/component authority",ui.DefaultTheme().compact&&ui.DefaultTheme().panelOpacity>.8f);
    TEST("Pass380 modular HUD reserves chat bottom-left and hotbar bottom-center",ui.Validate(workspace)&&workspace[3].anchor==GameUiAnchor::BottomLeft&&workspace[4].anchor==GameUiAnchor::BottomCenter);
    ContextActionSystem ctx;InteractionContext h;h.kind=ContextObjectKind::HostileShip;h.fleetAvailable=true;auto ha=ctx.Resolve(h);TEST("Pass381 universal RMB context knows hostile ship and fleet actions",ha.size()>=6);
    InteractionContext st;st.kind=ContextObjectKind::Station;st.dockable=true;auto sa=ctx.Resolve(st);bool dock=false;for(auto&a:sa)dock|=a.id=="dock"&&a.enabled;TEST("Pass381 station RMB context exposes docking",dock);
    StrategicFlightSystem flight;flight.SetMode(FlightControlMode::Strategic);StrategicFlightOrder o;o.kind=StrategicOrderKind::Engage;o.valid=true;flight.Issue(o);auto fi=flight.Evaluate({0,0,0},{100,0,0},100);TEST("Pass382 strategic command layer translates Engage into physical flight/fire intent",fi.fire&&fi.desiredThrottle>0);
    flight.ToggleMode();TEST("Pass383 manual-strategic mode toggle is explicit and reversible",flight.Mode()==FlightControlMode::Manual);
    TEST("Pass383 refinement gives flight-mode toggle a distinct input action from boost",InputAction::ToggleFlightMode!=InputAction::Boost);
    auto bindings=InputBindingProfile::Defaults();
    TEST("Pass383 refinement binds TAB to flight mode and SHIFT to boost",bindings.KeyFor(InputAction::ToggleFlightMode)=="TAB"&&bindings.KeyFor(InputAction::Boost)=="SHIFT");
    TEST("Pass400 refinement removes player-facing interior/cutaway bindings",bindings.KeyFor(InputAction::ToggleInterior).empty()&&bindings.KeyFor(InputAction::ToggleCutaway).empty());
    ShipActionHotbarSystem hot;auto bar=hot.BuildDefault({"weapon","mining","scanner","drone"});TEST("Pass384 fitted ship capabilities drive numbered action hotbar",bar.size()==4&&bar[0].slot==1&&bar.back().slot==4);
    TEST("Pass384 target/power requirements prevent invalid module activation",!hot.CanActivate(bar[0],false,100)&&hot.CanActivate(bar[0],true,100));
    ChatCommunicationSystem chat;chat.Post(ChatChannel::Fleet,"Wing","Aligning",1);chat.Post(ChatChannel::Local,"Dock","Clear",2);TEST("Pass385 bottom-left chat authority retains channel-specific unread state",chat.Unread(ChatChannel::Fleet)==1&&chat.Messages(ChatChannel::Fleet).size()==1);chat.MarkRead(ChatChannel::Fleet);TEST("Pass385 channel read state clears without affecting other channels",chat.Unread(ChatChannel::Fleet)==0&&chat.Unread(ChatChannel::Local)==1);
}
static void Pass386_391_DocksInfrastructure(){
    StationEcologySystem ecology;UniversalDockedStationSystem dock;auto tether=ecology.BuildStartingSystem(5,3,false)[1];auto dp=dock.Build(tether);TEST("Pass386 every station dock uses orbitable actual-fitted-ship presentation",dp.orbitCamera&&dp.showActualFittedShip&&!dp.onFoot);
    TEST("Pass387 station type defines player-facing hangar and service profile",dp.hangarProfile=="PLANETARY_TETHER_ORBIT"&&dp.services.size()>=4);
    ModularStationPlan p;p.modules={{1,StationModuleFunction::Core,"Core",10,20,0,{{"a","standard",false}}},{2,StationModuleFunction::Dock,"Dock",5,0,10,{{"a","standard",false}}},{3,StationModuleFunction::Storage,"Storage",2,0,100,{{"a","standard",false},{"b","standard",false}}}};ModularStationAssemblySystem asmbl;TEST("Pass388 X4-style prebuilt station modules connect through validated sockets",asmbl.Connect(p,1,"a",3,"a")&&asmbl.Connect(p,3,"b",2,"a")&&asmbl.Validate(p).valid);
    InfrastructureModuleSystem infra;InfrastructureModuleSpec mod;mod.powerUse=8;mod.minimumTech=PowerTechnology::Burner;TEST("Pass389 shared infrastructure module contract gates operation by power and tech",infra.CanOperate(mod,PowerTechnology::Burner,8)&&!infra.CanOperate(mod,PowerTechnology::Burner,2));
    auto fuels=infra.FuelProgression();TEST("Pass390 burner/refined-carbon fuel chain exists at progression start",fuels.size()>=8&&fuels[0].tech==PowerTechnology::Burner&&fuels[1].tech==PowerTechnology::RefinedCarbon);
    TEST("Pass390 fuel burn-time upgrades materially extend early logistics uptime",infra.EffectiveBurnTime(fuels[0],2.0)>fuels[0].burnSecondsPerUnit*2.5);
    TEST("Pass391 advanced progression reaches fusion proton experimental and alien power",fuels.back().tech==PowerTechnology::Alien&&fuels[5].tech==PowerTechnology::ProtonPlasma&&fuels[6].tech==PowerTechnology::Experimental);
}
static void Pass392_396_Galaxy(){
    GalaxyCatalogSystem cat;auto full=cat.Generate(123456,10000);TEST("Pass392 maximum seed catalog supports 10,000 lightweight solar systems",full.size()==10000&&full.front().discovered);
    GalaxyMapSystem map;GalaxyMapCamera cam;map.Orbit(cam,50,10);map.Zoom(cam,10);auto sel=map.Select(full,42);TEST("Pass393 3D galaxy map supports free orbit zoom and system inspection",sel.valid&&cam.yaw>25&&cam.distance<5400);
    auto yards=map.Filter(full,GalaxyOverlay::Shipyards);auto anomalies=map.Filter(full,GalaxyOverlay::Anomalies);TEST("Pass394 strategic overlays expose shipyards and anomalies from one catalog",!yards.empty()&&!anomalies.empty());
    auto small=cat.Generate(33,180);GalaxyRoutePlannerSystem routes;GalaxyRouteRequest r;r.start=1;r.end=80;r.jumpRange=1500;r.mode=GalaxyRouteMode::Safest;auto route=routes.Plan(small,r);TEST("Pass395 galaxy route planner resolves constrained multi-system routes",route.valid&&route.systems.front()==1&&route.systems.back()==80&&route.distance>0);
    OrbitalBodyRecord star;star.id=1;star.kind=OrbitalBodyKind::Star;OrbitalBodyRecord planet;planet.id=2;planet.parentId=1;planet.orbit.semiMajorAxis=8000;planet.orbit.orbitalPeriodSeconds=500;UniverseSystemMapSystem sys;auto a=sys.Build({star,planet},0,24),b=sys.Build({star,planet},100,24);TEST("Pass396 living System Map presents planets at evolving orbital positions",(a.nodes[1].currentPosition-b.nodes[1].currentPosition).length()>1000&&a.nodes[1].orbitTrack.size()==24);
}
static void Pass397_400_PiFleetAcceptance(){
    PlanetData p;p.type=PlanetType::Rocky;p.resourceRichness=.8f;p.hazardLevel=.2f;p.industryRepresentation=PlanetIndustryRepresentation::SurfaceHexGrid;PlanetaryIndustrySystem pi;auto s=pi.Generate(p,3,77);HexCoord origin{0,0};TEST("Pass397 surveyed planetary industry uses deterministic hex-grid authority",s.hexes.size()==37&&s.hexes[origin].surveyed);
    s.hexes[{1,0}].surveyed=true;s.hexes[{0,1}].surveyed=true;TEST("Pass397 hex accepts one functional installation when surveyed",pi.Place(s,{1,{0,0},PiInstallationKind::Storage,PowerTechnology::Burner,0,0,100,5,true})&&pi.Place(s,{2,{1,0},PiInstallationKind::Logistics,PowerTechnology::Burner,0,0,0,5,true})&&pi.Place(s,{3,{0,1},PiInstallationKind::Power,PowerTechnology::Burner,0,0,0,15,true}));
    PlanetData gas=p;gas.type=PlanetType::GasGiant;gas.industryRepresentation=PlanetIndustryRepresentation::AtmosphericCollectorRing;auto g=pi.Generate(gas,2,12);g.hexes[{0,0}].surveyed=true;TEST("Pass398 gas giants use atmospheric collector-ring installations rather than fake ground factory",pi.Place(g,{1,{0,0},PiInstallationKind::AtmosphericCollector,PowerTechnology::Solar,0,20,50,2,true}));
    auto g2=g;g2.hexes[{1,0}].surveyed=true;TEST("Pass398 gas giant rejects incompatible terrestrial extractor",!pi.Place(g2,{2,{1,0},PiInstallationKind::Extractor,PowerTechnology::Burner,0,10,0,1,true}));
    FleetIntentSystem fleet;std::vector<FleetWingShip> wing={{1,FleetShipRole::Leader,true},{2,FleetShipRole::Mining,true},{3,FleetShipRole::Combat,true},{4,FleetShipRole::Support,true},{5,FleetShipRole::Salvage,true},{6,FleetShipRole::Combat,true}};auto mining=fleet.Mirror(wing,StrategicOrderKind::Mine);TEST("Pass399 immediate flight is capped at four wing ships and role-aware intent mirroring",mining.size()==4&&mining[0].behavior.find("Mine")!=std::string::npos);
    FleetFlightConfig cfg;TEST("Pass399 fleet automatically changes formation for combat/mining/docking",fleet.FormationFor(cfg,StrategicOrderKind::Engage)==FormationType::Wedge&&fleet.FormationFor(cfg,StrategicOrderKind::Mine)==FormationType::Line&&fleet.FormationFor(cfg,StrategicOrderKind::Dock)==FormationType::Column);
    ShipOnlyVerticalAcceptanceSystem accept;ShipOnlyAcceptanceState state;state.stationDocked=state.fittedShipVisible=state.strategicFlight=state.contextOrders=state.hotbarReady=state.galaxyMapReady=state.orbitalSystemReady=state.planetaryIndustryReady=state.fleetWingReady=state.persistenceReady=true;auto report=accept.Evaluate(state);TEST("Pass400 unified ship-only vertical acceptance closes at 100 with no on-foot dependency",report.pass&&report.score==100&&report.blockers.empty());state.onFootPathExposed=true;TEST("Pass400 on-foot path is explicitly rejected by current production direction",!accept.Evaluate(state).pass);
}

int main(){
    Pass361_362_Attachments();Pass363_364_Inspection();Pass365_368_Orbits();Pass369_371_StationsAndMap();Pass372_378_ShipProfiles();Pass379_385_CommandUi();Pass386_391_DocksInfrastructure();Pass392_396_Galaxy();Pass397_400_PiFleetAcceptance();
    std::cout<<"\n=== Pass361-400 Command Galaxy Summary: "<<testsPassed<<" passed, "<<testsFailed<<" failed ===\n";
    return testsFailed?1:0;
}
