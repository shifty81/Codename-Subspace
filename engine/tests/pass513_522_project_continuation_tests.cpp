#include "combat/CombatSystem.h"
#include "ui/ShipCommandHudSystem.h"
#include "procedural/GalaxyGenerator.h"
#include "procedural/SolarSystemPlacementSystem.h"
#include "navigation/SystemNavigationSystem.h"
#include "navigation/VectorTravelSystem.h"
#include "navigation/TravelArrivalSystem.h"
#include "rendering/ForwardSpacePresentationSystem.h"
#include "rendering/EnvironmentPresentationSystem.h"
#include "station/StationKitbashVisualSystem.h"
#include "hangar/DockingExperienceSystem.h"
#include "interior/ShipEmbodimentSystem.h"

#include <cmath>
#include <iostream>
#include <vector>

using namespace subspace;
namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}
bool Near(float a,float b,float e=.001f){return std::fabs(a-b)<=e;}
ShipyardModuleRecord Module(const char* id, ShipyardPartRole role, ShipyardModuleSemantic semantic){
    ShipyardModuleRecord r;r.source.moduleId=id;r.source.halfWidth=r.source.halfLength=r.source.halfHeight=1.0f;
    r.partRole=role;r.semantic=semantic;r.generatorEligible=true;return r;
}
}

int main(){
    std::cout<<"[Pass513-522 Project Continuation]\n";

    // Pass513: combat durability -> HUD truth.
    CombatComponent combat;combat.armorRating=0.0f;combat.ConfigureDurability(10,15,20,40,false);
    CombatSystem combatSystem;DamageInfo hit;hit.damage=40;hit.damageType=DamageType::Energy;
    combatSystem.ApplyDamageToTarget(combat,hit);
    Check(Near(combat.shields.currentShieldHP,0)&&Near(combat.currentArmorHP,0)&&Near(combat.currentHullHP,5),"damage resolves shield -> armor -> hull using stored integrity");
    combat.ConfigureDurability(20,30,40,80,false);combat.shields.currentShieldHP=10;combat.currentArmorHP=15;combat.currentHullHP=10;combat.currentEnergy=20;
    const auto hud=ShipCommandHudSystem::Build(12,true,false,{"PRIMARY","SCAN"},&combat);
    Check(Near(hud.shield,.5f)&&Near(hud.armor,.5f)&&Near(hud.hull,.25f)&&Near(hud.power,.25f),"command HUD consumes authoritative combat fractions");
    combat.currentHullHP=5;const auto criticalHud=ShipCommandHudSystem::Build(0,true,false,{},&combat);
    Check(criticalHud.critical&&criticalHud.integrityStatus=="HULL CRITICAL","command HUD exposes critical integrity state");

    // Pass514: final materialization repair remains outside all celestial envelopes.
    GalaxyGenerator generator(514522);generator.planetProbability=1.0f;generator.eventStageProbability=0.0f;
    auto sector=generator.GenerateSector(1,2,0);SolarSystemPlacementSystem placement;
    const SectorPosition starCandidate=sector.star.position;
    const auto repairedStar=placement.RepairSpawnPosition(sector,starCandidate,12000,8000);
    Check(placement.IsSpawnPositionSafe(sector,repairedStar,12000,8000),"spawn repair evacuates stellar exclusion zone");
    if(!sector.planets.empty()){
        const auto repairedPlanet=placement.RepairSpawnPosition(sector,sector.planets.front().position,12000,8000);
        Check(placement.IsSpawnPositionSafe(sector,repairedPlanet,12000,8000),"spawn repair evacuates planet envelope");
    }

    // Pass515R2: warp charge is compact; cruise time is distance / ship Vector top speed and exposes ETA.
    SystemNavigationSystem nav;SystemDestination dest;dest.id=44;dest.name="Outer Marker";dest.type=SystemDestinationType::DeepSpace;dest.position.localX=900000000.0;dest.position.localY=100000000.0;
    Check(nav.RegisterDestination(dest),"warp destination registers");
    const WarpPlan plan=nav.PlanWarp({},44,100000000.0,5000.0);
    Check(plan.valid&&plan.chargeSeconds>=.85&&plan.chargeSeconds<=2.4&&plan.cruiseSeconds>1.5&&Near(static_cast<float>(plan.cruiseSeconds),static_cast<float>(plan.distanceMeters/plan.topSpeedMetersPerSecond),.01f),"warp plan uses short charge and distance/top-speed cruise time");
    const auto fasterPlan=nav.PlanWarp({},44,200000000.0,5000.0);Check(fasterPlan.valid&&fasterPlan.cruiseSeconds<plan.cruiseSeconds,"higher ship Vector top speed lowers ETA for the same destination");
    VectorTravelSession travel;VectorTravelSystem vector;Check(vector.Begin(travel,plan)&&travel.plannedSeconds>0&&travel.remainingSeconds>0,"vector travel publishes planned and remaining time");
    const double before=travel.remainingSeconds;vector.Update(travel,.5);Check(travel.remainingSeconds<before,"vector ETA counts down during travel");

    if(!sector.planets.empty()){
        const auto& p=sector.planets.front();SystemDestination pd;pd.id=77;pd.name=p.name;pd.type=SystemDestinationType::Planet;pd.position.localX=(p.position.x/.0120)*1000000.0;pd.position.localY=(p.position.y/.0120)*1000000.0;
        const auto arrival=TravelArrivalSystem{}.Resolve(pd,sector,18.0f);
        Check(arrival.valid&&arrival.environment==ArrivalEnvironment::PlanetOrbit&&arrival.localSceneReadyAtExit,"planet warp resolves to ready orbital scene");
        Check(arrival.safeFromPlanetSurface&&arrival.arrivalWorld.x!=arrival.focalWorld.x,"planet warp exits at safe off-axis orbital standoff");
    }

    // Pass516: deterministic, system-specific deep-space presentation.
    const auto skyA=ForwardSpacePresentationSystem{}.ForSector(sector);const auto skyB=ForwardSpacePresentationSystem{}.ForSector(sector);
    Check(Near(skyA.nebulaR,skyB.nebulaR)&&Near(skyA.galacticBandTilt,skyB.galacticBandTilt),"space backdrop is deterministic per sector");
    Check(skyA.starTwinkle>=.08f&&skyA.dustLaneStrength>=.06f&&skyA.nebulaB>0,"space backdrop carries twinkle, dust lane and nebula profile");

    // Pass517: stations reuse certified Shipyard vocabulary.
    std::vector<ShipyardModuleRecord> catalog{
        Module("frame_connector",ShipyardPartRole::StructuralAttachment,ShipyardModuleSemantic::StructuralFrame),
        Module("bridge",ShipyardPartRole::Bridge,ShipyardModuleSemantic::CommandBridge),
        Module("cargo",ShipyardPartRole::Cargo,ShipyardModuleSemantic::Component),
        Module("sensor",ShipyardPartRole::SensorDish,ShipyardModuleSemantic::Sensor),
        Module("hardpoint",ShipyardPartRole::HardpointBase,ShipyardModuleSemantic::TurretHardpoint)};
    const auto station=StationKitbashVisualSystem::Build(catalog,StationArchetype::Military,517,true);
    Check(station.resolved&&station.modules.size()>=8&&station.usedStructuralConnectors,"station recipe resolves shared structural Shipyard kitbash");
    Check(station.usedCommandModule&&station.usedSensorModule&&station.usedHardpointModule,"military station composes command/sensor/hardpoint semantic families");

    // Pass518-519: explicit cockpit transition; zoom can never expose interior.
    ShipEmbodimentSystem body;Check(!body.ZoomMayRevealInterior(100.0f),"camera zoom cannot reveal ship interior");
    Check(body.ExitCockpit(9)&&body.IsOnFoot(),"explicit cockpit exit enters on-foot interior");
    const auto start=body.Avatar().localPosition;body.Move(1,1,.10);Check((body.Avatar().localPosition-start).length()>0.01f,"on-foot interior avatar responds to movement");
    Check(!body.TakeControls(),"pilot controls require returning to cockpit interaction point");

    // Pass520: docking publishes active guidance, range and speed limits.
    DockingExperienceState dock;DockingExperienceSystem docking;Check(docking.Request(dock,55,{100,100,0},{100,30,0},true),"docking request acquires physical guidance corridor");
    docking.Update(dock,{100,30,0},.1);Check(!dock.guidanceCue.empty()&&dock.distanceToGuidance>=0&&dock.assignedSpeedLimit>0,"docking state publishes live guidance cue/range/speed limit");

    // Pass521: shield-online state follows actual shield activation/reserve.
    combat.currentHullHP=combat.maxHullHP;combat.currentArmorHP=combat.maxArmorHP;combat.shields.currentShieldHP=0;combat.shields.isShieldActive=true;const auto shieldDown=ShipCommandHudSystem::Build(0,true,false,{},&combat);
    Check(!shieldDown.shieldOnline&&shieldDown.integrityStatus=="SHIELDS DOWN","shield presentation gates off when reserve is depleted");

    // Pass522: camera response is mode/speed aware and restores flight profile.
    EnvironmentPresentationSystem env;const auto foot=env.MotionFor(CameraMode::OnFoot,0);const auto flightSlow=env.MotionFor(CameraMode::ShipFlight,0);const auto flightFast=env.MotionFor(CameraMode::ShipFlight,100);
    Check(foot.velocityLookAhead==0&&foot.followSmoothness>flightSlow.followSmoothness,"on-foot camera is tight and has no velocity lookahead");
    Check(flightFast.velocityLookAhead>flightSlow.velocityLookAhead&&flightFast.followSmoothness<flightSlow.followSmoothness,"flight camera increases lookahead and loosens response at speed");

    std::cout<<"Pass513-522 assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
