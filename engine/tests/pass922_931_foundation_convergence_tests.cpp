#include "debug_tools/PerformanceReferenceProfile.h"
#include "input/PlayerControlSystem.h"
#include "procedural/GalaxyGenerator.h"
#include "runtime/StableIdentitySystem.h"
#include "runtime/WorldPersistenceContract.h"
#include "world/SpatialFrameSystem.h"
#include <cmath>
#include <iostream>

using namespace subspace;
namespace {
int passed=0,failed=0;
void Test(const char* name,bool ok){if(ok){++passed;}else{++failed;std::cerr<<"FAIL: "<<name<<"\n";}}
bool Near(double a,double b,double e=1e-8){return std::fabs(a-b)<=e;}
DoubleQuat Z90(){const double s=std::sqrt(0.5);return {0,0,s,s};}
}

int main(){
    PlanetData defaultPlanet;
    Test("solid planet capability defaults landable", defaultPlanet.landable);

    EntityManager entities; InputState input;
    PlayerControlSystem controls(entities,input);
    Test("flight authority defaults to full 3D", controls.GetFlightAuthorityMode()==PlayerControlSystem::FlightAuthorityMode::Full3D);
    controls.SetFlightAuthorityMode(PlayerControlSystem::FlightAuthorityMode::TacticalPlanar);
    Test("tactical planar mode is explicit opt-in", controls.GetFlightAuthorityMode()==PlayerControlSystem::FlightAuthorityMode::TacticalPlanar);

    SpatialFrameSystem frames;
    SpatialFrame movingParent; movingParent.id=1; movingParent.localPosition={10,20,30}; movingParent.localRotation=Z90(); movingParent.linearVelocity={3,4,5}; movingParent.angularVelocity={0,0,0.25};
    Test("moving parent frame accepted",frames.Upsert(movingParent));
    SpatialKinematicState worldState; worldState.frameId=InvalidSpatialFrameId; worldState.localPosition={8,25,32}; worldState.localVelocity={1,2,3};
    auto local=frames.ReparentPreservingWorld(worldState,1);
    const auto pointRoundTrip=frames.ToWorldPoint(local.frameId,local.localPosition);
    const auto velocityRoundTrip=frames.ToWorldVelocity(local.frameId,local.localPosition,local.localVelocity);
    Test("rotated reparent preserves world point",Near(pointRoundTrip.x,8)&&Near(pointRoundTrip.y,25)&&Near(pointRoundTrip.z,32));
    Test("rotated moving reparent preserves world velocity",Near(velocityRoundTrip.x,1)&&Near(velocityRoundTrip.y,2)&&Near(velocityRoundTrip.z,3));

    const auto shipA=StableIdentitySystem::Deterministic(PersistentEntityKind::Ship,"reference-system","player-lineage",7);
    const auto shipB=StableIdentitySystem::Deterministic(PersistentEntityKind::Ship,"reference-system","player-lineage",7);
    const auto shipC=StableIdentitySystem::Deterministic(PersistentEntityKind::Ship,"reference-system","player-lineage",8);
    Test("stable identity deterministic",shipA==shipB&&shipA.IsValid());
    Test("stable identity generation differentiates lineage revision",shipA!=shipC);
    Test("stable identity has canonical 128-bit text",StableIdentitySystem::ToString(shipA).size()==32);
    Test("persistence schema v2 authority active",SubspacePersistenceSchemaVersion==2);

    const auto perf=PerformanceReferenceProfiles::P921Reference();
    Test("P921 performance baseline named",perf.baselineId=="SUBSPACE-P921-PERFORMANCE-REFERENCE-BASELINE");
    Test("performance baseline tied to certified gate",perf.certifiedGateId=="QG-20260912-193645-full-58693c78");
    Test("performance baseline requires core captures",perf.metrics.size()>=6);

    std::cout<<"Pass922-931 Foundation Convergence: "<<passed<<" passed / "<<failed<<" failed\n";
    return failed==0?0:1;
}
