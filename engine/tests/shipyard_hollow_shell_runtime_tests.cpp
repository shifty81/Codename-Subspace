#include "interior/ShipInteriorDerivedShellSystem.h"
#include "interior/ShipInteriorShellTraversalSystem.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace subspace;
namespace {
int checks=0;
void Verify(bool b,const char* m){if(!b){std::cerr<<"FAIL: "<<m<<'\n';std::exit(1);}++checks;}
bool Near(float x,float y,float tolerance=.06f){return std::fabs(x-y)<tolerance;}
ShipInteriorCarvePlan Single(){
    ShipInteriorCarvePlan p;p.valid=true;
    InteriorCarvedVolume v;v.moduleIndex=0;v.moduleId="hull";
    v.center={0,0,0};v.halfExtents={2,2,2};v.walkable=true;
    p.volumes.push_back(v);return p;
}
WorldScaleProfile Scale(){
    auto s=WorldScaleProfile{};s.referenceDoorWidthMeters=1.2f;
    s.referenceDoorHeightMeters=2.1f;s.referencePlayerHeightMeters=1.8f;
    return s;
}
}
int main(){
    const float radius=.30f,height=1.75f;
    auto carve=Single();
    auto shell=ShipInteriorDerivedShellSystem::Build(carve,Scale());
    Verify(shell.ready&&shell.surfaces.size()==6,"six real wall quads");
    const Vector3 center{0,0,-2};
    Verify(ShipInteriorShellTraversalSystem::CanOccupy(carve,shell,center,radius,height),"safe spawn on derived floor");
    Verify(!ShipInteriorShellTraversalSystem::CanOccupy(carve,shell,{1.9f,0,-2},radius,height),"derived wall blocks capsule");
    Verify(!ShipInteriorShellTraversalSystem::CanOccupy(carve,shell,{0,0,-1.0f},radius,height),"no flying off derived floor");
    Verify(!ShipInteriorShellTraversalSystem::CanOccupy(carve,shell,{0,0,-2.5f},radius,height),"no phasing below floor");
    auto moved=ShipInteriorShellTraversalSystem::Move(carve,shell,center,{10,0,0},radius,height);
    Verify(moved.x<=1.71f&&moved.x>=1.60f,"large displacement cannot tunnel through outer wall");
    Verify(Near(ShipInteriorShellTraversalSystem::Move(carve,shell,center,{100,0,0},radius,height).x,0),
           "huge displacement cannot tunnel past bounded microstep policy");
    moved=ShipInteriorShellTraversalSystem::Move(carve,shell,{1.65f,1.65f,-2},{1,1,0},radius,height);
    Verify(moved.x<=1.72f&&moved.y<=1.72f,"capsule blocks diagonal corner penetration");
    Vector3 spawn{900,900,0};
    Verify(ShipInteriorShellTraversalSystem::Spawn(carve,shell,radius,height,spawn),"spawn finds real carve not starter-deck bounds");
    Verify(Near(spawn.x,0)&&Near(spawn.z,-2),"spawn is module floor in local metres");
    Verify(!ShipInteriorShellTraversalSystem::CanOccupy(carve,{},center,radius,height),"missing shell fails closed");
    Verify(Near(ShipInteriorShellTraversalSystem::Move(carve,{},center,{3,0,0},radius,height).x,0),"invalid shell disables walking");
    carve.volumes[0].yawDegrees=45;
    auto unsupported=ShipInteriorDerivedShellSystem::Build(carve,Scale());
    Verify(!unsupported.ready&&unsupported.surfaces.empty(),"unsupported rotation gets no false collision");
    carve=Single();
    InteriorCarvedVolume other=carve.volumes.front();other.moduleIndex=1;other.moduleId="hull2";
    other.center={4.12f,0,0};carve.volumes.push_back(other);
    carve.portals.push_back({0,1,{2.06f,0,0},InteriorPortalKind::OpenPassage,true});
    shell=ShipInteriorDerivedShellSystem::Build(carve,Scale());
    Verify(shell.ready&&shell.openings.size()==1,"certified portal has actual generated opening");
    Verify(shell.openings.front().gap>0,"between inset volumes there is a real bridging gap");
    Verify(Near(shell.openings.front().center.z,-.95f),"portal is floor aligned, never centered in mid-air");
    auto passed=ShipInteriorShellTraversalSystem::Move(carve,shell,center,{4.12f,0,0},radius,height);
    Verify(passed.x>3.90f&&Near(passed.z,-2),"capsule walks through same door deleted from visual wall");
    auto blocked=ShipInteriorShellTraversalSystem::Move(carve,shell,{0,1.1f,-2},{4.12f,0,0},radius,height);
    Verify(blocked.x<1.85f,"door frame blocks off-center passage, no invisible clearance");
    Vector3 reverse={4.12f,0,-2};
    auto reversePassed=ShipInteriorShellTraversalSystem::Move(carve,shell,reverse,{-4.12f,0,0},radius,height);
    Verify(reversePassed.x<.25f,"generated door permits two-way traversal");
    // Reverse spatial order but retain module identities. Portal clipping
    // and corridor membership cannot depend on which module is +X.
    carve.volumes[1].center.x=-4.12f;
    shell=ShipInteriorDerivedShellSystem::Build(carve,Scale());
    Verify(shell.ready&&shell.openings.size()==1,"reverse-facing portal produces geometry");
    auto left=ShipInteriorShellTraversalSystem::Move(carve,shell,center,{-4.12f,0,0},radius,height);
    Verify(left.x<-3.9f,"reverse-facing aperture traverses to second hull");
    carve.volumes[1].center.x=4.12f;
    carve.volumes[1].center.z=.25f;
    shell=ShipInteriorDerivedShellSystem::Build(carve,Scale());
    Verify(!shell.ready&&!shell.errors.empty(),"mismatched deck floors fail closed instead of floating portal");
    carve.volumes[1].center={0,0,4.12f};
    shell=ShipInteriorDerivedShellSystem::Build(carve,Scale());
    Verify(!shell.ready&&!shell.errors.empty(),"vertical portal requires authored stairs or elevator");
    carve.volumes[1].center={4.12f,0,0};
    carve.portals.clear();
    shell=ShipInteriorDerivedShellSystem::Build(carve,Scale());
    Verify(shell.ready,"independent cavities can have standalone surface plan");
    Verify(ShipInteriorShellTraversalSystem::Move(carve,shell,center,{4.12f,0,0},radius,height).x<1.85f,
           "uncut seam blocks walking without explicit portal");
    std::cout<<"PASS Hollow Shell Runtime: "<<checks<<" render/collision/portal assertions\n";
}
