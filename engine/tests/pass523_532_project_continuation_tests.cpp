#include "station/StationRuntimeQualitySystem.h"
#include "procedural/SolarSystemEcologySystem.h"
#include "procedural/SolarSystemPlacementSystem.h"
#include "procedural/GalaxyGenerator.h"
#include "rendering/ImportedPlanetVisualSystem.h"
#include "navigation/SystemNavigationSystem.h"
#include "interior/ShipInteriorLayoutSystem.h"
#include "interior/InteriorInteractionSystem.h"
#include "hangar/UniversalDockedStationSystem.h"
#include "station/StationServiceSystem.h"
#include "ships/ModuleCapabilityDamageSystem.h"
#include "station/StationActivityPresentationSystem.h"
#include "ui/RuntimeControlContextSystem.h"
#include "ui/ProductionInterfaceSystem.h"
#include "rendering/EnvironmentPresentationSystem.h"
#include "ships/Block.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
using namespace subspace;
namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}
bool Near(double a,double b,double e=.001){return std::fabs(a-b)<=e;}
ShipyardModuleRecord Module(const char* id,ShipyardPartRole role,ShipyardModuleSemantic semantic){ShipyardModuleRecord r;r.source.moduleId=id;r.source.halfWidth=r.source.halfLength=r.source.halfHeight=1.0f;r.partRole=role;r.semantic=semantic;r.generatorEligible=true;return r;}
VisualModulePlacement Placement(const char* id){VisualModulePlacement p;p.moduleId=id;return p;}
std::shared_ptr<Block> BlockOf(BlockType type,float hp=100,float max=100){auto b=std::make_shared<Block>();b->type=type;b->size={1,1,1};b->maxHP=max;b->currentHP=hp;return b;}
}
int main(){
    std::cout<<"[Pass523-532 Project Continuation]\n";
    std::vector<ShipyardModuleRecord> catalog{
        Module("structural_frame",ShipyardPartRole::StructuralFrame,ShipyardModuleSemantic::StructuralFrame),
        Module("structural_connector",ShipyardPartRole::StructuralAttachment,ShipyardModuleSemantic::StructuralFrame),
        Module("bridge",ShipyardPartRole::Bridge,ShipyardModuleSemantic::CommandBridge),
        Module("cargo",ShipyardPartRole::Cargo,ShipyardModuleSemantic::Component),
        Module("hangar",ShipyardPartRole::Hangar,ShipyardModuleSemantic::Component),
        Module("sensor",ShipyardPartRole::SensorDish,ShipyardModuleSemantic::Sensor),
        Module("hardpoint",ShipyardPartRole::HardpointBase,ShipyardModuleSemantic::TurretHardpoint),
        Module("engine",ShipyardPartRole::MainEngine,ShipyardModuleSemantic::MainEngine)};

    // Pass523 - station runtime kitbash QA.
    StationRuntimeQualitySystem stationQa;const auto military=stationQa.Audit(catalog,StationArchetype::Military,523,false);
    Check(military.certified,"Pass523 military station kitbash meets runtime quality gate");
    Check(!military.primitiveFallbackAllowed&&military.structuralModules>=2,"Pass523 certified structural content forbids primitive fallback");
    Check(military.commandModules>=1&&military.hardpointModules>=1,"Pass523 military silhouette includes command and hardpoint vocabulary");

    // Pass524 - solar ecology distribution after canonical normalization.
    GalaxyGenerator gen(524);gen.planetProbability=1.0f;gen.eventStageProbability=0.0f;auto sector=gen.GenerateSector(2,1,0);
    sector.hasStation=true;sector.station.stationId="qa_station";sector.station.name="QA Station";sector.station.stationType="Trade";
    if(sector.ships.empty()){ShipData ship;ship.shipId="qa_traffic";sector.ships.push_back(ship);}SolarSystemPlacementSystem placement;const auto normalized=placement.Normalize(sector);
    Check(normalized.certified,"Pass524 canonical placement normalizes generated system safely");
    const auto ecology=SolarSystemEcologySystem{}.Audit(sector);Check(ecology.unsafeContacts==0,"Pass524 ecology audit finds no celestial-overlap contacts");
    Check(ecology.occupiedBands>=2&&ecology.localTraffic>0&&ecology.distributionScore>=.70f,"Pass524 ecology preserves multiple activity bands and local traffic");

    // Pass525 - imported planet visual parity is per-world, not one global spin profile.
    const auto gas=ImportedPlanetVisualSystem::ProfileFor(PlanetType::GasGiant);const auto desert=ImportedPlanetVisualSystem::ProfileFor(PlanetType::Desert);const auto ocean=ImportedPlanetVisualSystem::ProfileFor(PlanetType::Oceanic);
    Check(!gas.cloudTextures.empty()&&gas.primaryCloudRotationRate>gas.surfaceRotationRate,"Pass525 gas giant clouds rotate independently of surface");
    Check(gas.atmosphereRadiusMultiplier>desert.atmosphereRadiusMultiplier,"Pass525 atmosphere shell depth varies by planet class");
    Check(!ocean.cloudTextures.empty()&&ocean.primaryCloudRotationRate!=gas.primaryCloudRotationRate,"Pass525 imported planet families keep distinct weather motion");

    // Pass526 - rolled-forward Pass515R2 Vector timing.
    SystemNavigationSystem nav;SystemDestination nearD;nearD.id=1;nearD.name="Near";nearD.position.localX=800000000.0;nav.RegisterDestination(nearD);SystemDestination farD=nearD;farD.id=2;farD.name="Far";farD.position.localX=2400000000.0;nav.RegisterDestination(farD);
    const auto nearSlow=nav.PlanWarp({},1,100000000.0,10000);const auto nearFast=nav.PlanWarp({},1,200000000.0,10000);const auto farSlow=nav.PlanWarp({},2,100000000.0,10000);
    Check(nearSlow.valid&&nearFast.valid&&farSlow.valid,"Pass526 Vector plans remain valid under distance/speed timing");
    Check(nearFast.cruiseSeconds<nearSlow.cruiseSeconds,"Pass526 faster ship completes identical route sooner");
    Check(farSlow.cruiseSeconds>nearSlow.cruiseSeconds,"Pass526 farther destination takes longer at identical speed");
    Check(Near(nearSlow.cruiseSeconds,std::max(1.5,nearSlow.distanceMeters/nearSlow.topSpeedMetersPerSecond),.01),"Pass526 Vector duration is route distance divided by ship top speed");

    // Pass527 - authored exterior recipe drives interior layout.
    ProceduralShipVisualRecipe recipe;recipe.modules={Placement("structural_frame"),Placement("structural_frame"),Placement("structural_frame"),Placement("structural_frame"),Placement("structural_frame"),Placement("structural_frame"),Placement("cargo"),Placement("cargo"),Placement("hangar"),Placement("engine"),Placement("engine"),Placement("engine"),Placement("bridge")};
    ShipInteriorSystem interiors;ShipInteriorLayoutSystem interiorLayout;const auto plan=interiorLayout.Materialize(527,catalog,recipe,interiors);const auto* layout=interiors.GetLayout(527);
    Check(layout&&static_cast<int>(layout->rooms.size())==plan.rooms&&plan.rooms>=10,"Pass527 authored recipe materializes multi-room ship interior");
    Check(plan.airlocks>=2&&plan.corridors>=1,"Pass527 hangar/exterior complexity creates usable airlock/corridor connectivity");
    Check(plan.decks>=1&&plan.decks<=3,"Pass527 interior deck count remains bounded by authored hull complexity");

    // Pass528 - doors/hatches/airlocks/consoles are stateful interactions.
    InteriorInteractionSystem interactions;InteriorFixtureState door;door.kind=InteriorFixtureKind::Door;auto open=interactions.Execute(door,"OPEN");Check(open.success&&door.open,"Pass528 door interaction changes persistent fixture state");
    interactions.Execute(door,"LOCK / UNLOCK");Check(door.locked&&!door.open,"Pass528 locking a door fails closed");
    InteriorFixtureState airlock;airlock.kind=InteriorFixtureKind::Airlock;auto cycle=interactions.Execute(airlock,"CYCLE AIRLOCK");Check(cycle.success&&!airlock.pressurized&&!airlock.open,"Pass528 airlock cycle seals before pressure transition");
    InteriorFixtureState console;console.kind=InteriorFixtureKind::Console;console.powered=false;Check(!interactions.Execute(console,"USE CONSOLE").success,"Pass528 unpowered console refuses interaction");

    // Pass529 - docked station services reflect actual station capabilities.
    GeneratedStationProfile shipyard;shipyard.name="QA Shipworks";shipyard.archetype=StationArchetype::Shipyard;shipyard.services={"Dock","Shipyard","Fitting","Repair","Fuel","Storage"};const auto dock=UniversalDockedStationSystem{}.Build(shipyard);
    Check(dock.serviceProfile.shipyard&&dock.serviceProfile.refit&&dock.serviceProfile.repairHull&&dock.serviceProfile.refuel,"Pass529 shipyard dock exposes real refit/repair/refuel capabilities");
    Check(!dock.serviceProfile.market,"Pass529 unavailable market is not fabricated by generic dock UI");
    StationServiceSystem service;Check(service.Quote(dock.serviceProfile,StationServiceType::RepairModules,10,0).available,"Pass529 executable module-repair service follows dock profile");

    // Pass530 - module damage degrades matching capabilities continuously.
    Ship damaged;damaged.blocks={BlockOf(BlockType::Engine,50),BlockOf(BlockType::Thruster,25),BlockOf(BlockType::Generator,100),BlockOf(BlockType::ShieldGenerator,0),BlockOf(BlockType::Computer,40),BlockOf(BlockType::WeaponMount,75),BlockOf(BlockType::Cargo,20)};const auto caps=ModuleCapabilityDamageSystem{}.Evaluate(damaged);
    Check(Near(caps.mainThrust,.5)&&Near(caps.maneuvering,.25),"Pass530 engine/RCS damage reduces thrust and maneuvering separately");
    Check(Near(caps.power,1.0)&&Near(caps.shields,0.0),"Pass530 power and shield-generator damage remain independent");
    Check(Near(caps.sensors,.4)&&Near(caps.weapons,.75)&&Near(caps.cargo,.2),"Pass530 sensor/weapon/cargo capability tracks module health");

    // Pass531 - stations produce lanes, traffic and security by archetype/population.
    const auto traffic=StationActivityPresentationSystem{}.Build(2000,StationArchetype::Military,DockingExperienceStage::Undocked);
    Check(traffic.approachLanes>=2&&traffic.inboundTraffic>0&&traffic.outboundTraffic>0,"Pass531 living station traffic owns approach/departure lanes");
    Check(traffic.securityCraft>=3,"Pass531 military station activity includes security traffic");
    Check(traffic.trafficDensity>0&&traffic.serviceDrones>0,"Pass531 station activity includes drones and density budget");

    // Pass532 - one mode context controls camera/input/HUD truth.
    RuntimeControlContextSystem contexts;const auto foot=contexts.Build(SandboxWorkspaceMode::Flight,ShipEmbodimentMode::InteriorOnFoot,DockingExperienceStage::Undocked,false,false);const auto docked=contexts.Build(SandboxWorkspaceMode::Flight,ShipEmbodimentMode::DockedHangar,DockingExperienceStage::Docked,false,false);const auto flight=contexts.Build(SandboxWorkspaceMode::Flight,ShipEmbodimentMode::CockpitControl,DockingExperienceStage::Undocked,false,false);
    Check(!foot.flightControls&&foot.interiorControls&&!foot.weapons&&foot.cameraMode==CameraMode::OnFoot,"Pass532 on-foot mode atomically switches controls/camera");
    Check(!docked.flightControls&&docked.dockingControls&&docked.cameraMode==CameraMode::DockedHangar,"Pass532 docked mode cannot leak flight controls");
    Check(flight.flightControls&&flight.weapons&&flight.scanner&&flight.cameraMode==CameraMode::ShipFlight,"Pass532 returning to cockpit restores flight context");
    const auto ui=ProductionInterfaceSystem{}.Build(ProductionContextKind::None,"",ShipEmbodimentMode::InteriorOnFoot,DockingExperienceStage::Undocked,false);bool hasI=false;for(const auto& h:ui.bottomHints)if(h.find("I COCKPIT / INTERIOR")!=std::string::npos)hasI=true;Check(hasI,"Pass532 HUD control legend matches actual I cockpit/interior binding");

    std::cout<<"Pass523-532 assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";return failures?1:0;
}
