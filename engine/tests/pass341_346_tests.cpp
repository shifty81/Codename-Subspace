// Pass341-346 modular thrusters + animated Vector travel acceptance suite.
#include <cmath>
#include <iostream>
#include <set>
#include <string>

#include "navigation/VectorTravelSystem.h"
#include "rendering/ForwardSpacePresentationSystem.h"
#include "ships/ThrusterLayoutSystem.h"

using namespace subspace;
static int testsPassed=0; static int testsFailed=0;
#define TEST(name,expr) do { if(expr){++testsPassed;std::cout<<"  PASS: "<<name<<"\n";}else{++testsFailed;std::cout<<"  FAIL: "<<name<<" ("<<__FILE__<<":"<<__LINE__<<")\n";} } while(0)

static void TestPass341ThrusterModuleAuthority(){
    std::cout<<"[Pass341ThrusterModuleAuthority]\n";
    const auto industrial=ThrusterLayoutSystem::StarterIndustrial();
    TEST("Pass341 industrial layout remains complete six-axis planar control",industrial.IsCompleteSixAxisPlanarLayout());
    TEST("Pass341 forward drive is externally mounted",industrial.HasExternallyMountedRole(ThrusterRole::Forward));
    TEST("Pass341 retro drive is externally mounted",industrial.HasExternallyMountedRole(ThrusterRole::Reverse));
    TEST("Pass341 lateral drive is externally mounted",industrial.HasExternallyMountedRole(ThrusterRole::StrafeLeft)&&industrial.HasExternallyMountedRole(ThrusterRole::StrafeRight));
    TEST("Pass341 yaw drive is externally mounted",industrial.HasExternallyMountedRole(ThrusterRole::YawLeft)&&industrial.HasExternallyMountedRole(ThrusterRole::YawRight));
    TEST("Pass341 industrial layout uses multiple visible module shapes",industrial.CountDistinctModuleShapes()>=4);
    TEST("Pass341 every thruster has clear nozzle/plume authority",industrial.HasClearPlumeDirections());
    bool currentMeshes=true;
    const std::set<std::string> authored={"engine_main","engine_small","thruster","thruster_small"};
    for(const auto&s:industrial.sockets) if(!authored.count(s.moduleMesh)) currentMeshes=false;
    TEST("Pass341 visible thrusters reuse current authored ship models",currentMeshes);
}

static void TestPass342RoleAwareLayouts(){
    std::cout<<"[Pass342RoleAwareLayouts]\n";
    const auto combat=ThrusterLayoutSystem::ForShipRole("Combat Frigate");
    const auto hauler=ThrusterLayoutSystem::ForShipRole("Heavy Hauler");
    const auto scout=ThrusterLayoutSystem::ForShipRole("Exploration Scout");
    const auto miner=ThrusterLayoutSystem::ForShipRole("Industrial Mining Ship");
    TEST("Pass342 combat ships receive combat layout",combat.layoutId.find("combat")!=std::string::npos);
    TEST("Pass342 haulers receive heavy drive layout",hauler.layoutId.find("hauler")!=std::string::npos);
    TEST("Pass342 exploration ships receive canted/vector layout",scout.layoutId.find("exploration")!=std::string::npos);
    TEST("Pass342 miners receive industrial external layout",miner.layoutId.find("industrial")!=std::string::npos);
    TEST("Pass342 combat main drive uses paired exposed engines",combat.CountRole(ThrusterRole::Forward)>=2);
    TEST("Pass342 heavy hauler uses a different module treatment from combat",hauler.layoutId!=combat.layoutId&&hauler.CountDistinctModuleShapes()!=0);
}

static WarpPlan MakePlan(){
    WarpPlan p;p.valid=true;p.chargeSeconds=1.5;p.cruiseSeconds=2.4;p.fuelCost=12.0;return p;
}

static void TestPass343VectorStateProgress(){
    std::cout<<"[Pass343VectorStateProgress]\n";
    VectorTravelSystem system;VectorTravelSession s;
    TEST("Pass343 valid plan begins Vector alignment",system.Begin(s,MakePlan())&&s.stage==VectorTravelStage::Aligning);
    system.Update(s,0.60);
    TEST("Pass343 alignment exposes normalized stage progress",s.stageProgress>0.45&&s.stageProgress<0.60);
    const double phaseBefore=s.visualPhase;system.Update(s,0.10);
    TEST("Pass343 Vector maintains continuous animation phase",s.visualPhase>phaseBefore);
    while(s.stage==VectorTravelStage::Aligning)system.Update(s,0.10);
    TEST("Pass343 alignment transitions into charging",s.stage==VectorTravelStage::Charging&&s.stageProgress==0.0);
    while(s.stage==VectorTravelStage::Charging)system.Update(s,0.10);
    TEST("Pass343 charging transitions into slipstream cruise",s.stage==VectorTravelStage::Cruise&&s.status=="VECTOR SLIPSTREAM");
    system.Update(s,0.80);
    TEST("Pass343 cruise exposes independent normalized stage progress",s.stageProgress>0.25&&s.stageProgress<0.45);
}

static void TestPass344AnimatedVectorPresentation(){
    std::cout<<"[Pass344AnimatedVectorPresentation]\n";
    ForwardSpacePresentationSystem visuals;
    const auto charge=visuals.VectorVisual(VectorTravelStage::Charging,0.75);
    const auto cruise=visuals.VectorVisual(VectorTravelStage::Cruise,0.50);
    const auto exit=visuals.VectorVisual(VectorTravelStage::Decelerating,0.80);
    TEST("Pass344 charge builds a visible tunnel rather than static splash",charge.tunnelOpacity>0.20f&&charge.tunnelFlow>0.40f);
    TEST("Pass344 charge transitions camera toward chase view",charge.chaseBias>0.70f);
    TEST("Pass344 cruise drives continuous tunnel motion",cruise.tunnelFlow>=0.99f&&cruise.tunnelTwist>0.60f);
    TEST("Pass344 cruise keeps player ship field visible",cruise.shipEnvelope>0.85f);
    TEST("Pass344 cruise overdrives real main thruster presentation",cruise.engineOverdrive>=0.99f);
    TEST("Pass344 foreground streak layer remains active during cruise",cruise.foregroundStreaks>0.80f);
    TEST("Pass344 exit reveals destination while tunnel collapses",exit.destinationReveal>0.75f&&exit.tunnelOpacity<0.25f);
}

static void TestPass345TravelCompletion(){
    std::cout<<"[Pass345TravelCompletion]\n";
    VectorTravelSystem system;VectorTravelSession s;system.Begin(s,MakePlan());
    for(int i=0;i<120&&system.InTransit(s);++i)system.Update(s,0.10);
    TEST("Pass345 animated travel completes deterministically",s.stage==VectorTravelStage::Complete);
    TEST("Pass345 completed travel reaches full global progress",std::fabs(s.progress-1.0)<1e-6);
    TEST("Pass345 completed travel retains terminal stage progress",std::fabs(s.stageProgress-1.0)<1e-6);
    TEST("Pass345 visual phase advanced through the complete trip",s.visualPhase>4.0);
}

int main(){
    TestPass341ThrusterModuleAuthority();
    TestPass342RoleAwareLayouts();
    TestPass343VectorStateProgress();
    TestPass344AnimatedVectorPresentation();
    TestPass345TravelCompletion();
    std::cout<<"\n=== Pass341-346 Flight Visual Summary: "<<testsPassed<<" passed, "<<testsFailed<<" failed ===\n";
    return testsFailed?1:0;
}
