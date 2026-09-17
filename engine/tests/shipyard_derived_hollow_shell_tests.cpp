#include "interior/ShipInteriorDerivedShellSystem.h"
#include "interior/ShipInteriorLayoutSystem.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
using namespace subspace;
namespace {
int checks=0;
void Check(bool ok,const char* msg){if(!ok){std::cerr<<"FAIL: "<<msg<<'\n';std::exit(1);}++checks;}
bool Near(float a,float b,float tol=.002f){return std::fabs(a-b)<tol;}
ShipyardModuleRecord Module(const char* id,ShipyardPartRole role){ShipyardModuleRecord r;r.source.moduleId=id;
    r.source.halfWidth=2;r.source.halfLength=2;r.source.halfHeight=2;r.partRole=role;
    r.semantic=role==ShipyardPartRole::Wing?ShipyardModuleSemantic::Wing:ShipyardModuleSemantic::HullMid;return r;}
VisualModulePlacement Place(const char* id,float x,float y=0){VisualModulePlacement v;v.moduleId=id;v.x=x;v.y=y;return v;}
ShipInteriorCarvePlan Carve(const std::vector<ShipyardModuleRecord>& c,const ProceduralShipVisualRecipe& r){return ShipInteriorCarvingSystem::Carve(c,r);}
float Axis(const Vector3& p,int axis){return axis==0?p.x:(axis==1?p.y:p.z);}
bool Covers(const InteriorShellSurface& s,float u,float v){
    int ua=s.axis==0?1:0,va=s.axis==2?1:2;float u0=1e9f,u1=-1e9f,v0=1e9f,v1=-1e9f;
    for(const auto& p:s.corners){u0=std::min(u0,Axis(p,ua));u1=std::max(u1,Axis(p,ua));
                                  v0=std::min(v0,Axis(p,va));v1=std::max(v1,Axis(p,va));}
    return u>u0+.01f&&u<u1-.01f&&v>v0+.01f&&v<v1-.01f;
}
}
int main(){
    std::vector<ShipyardModuleRecord> catalog{Module("hull",ShipyardPartRole::PrimaryHull),Module("wing",ShipyardPartRole::Wing)};
    ProceduralShipVisualRecipe ship;ship.modules={Place("hull",0)};
    auto initial=Carve(catalog,ship);Check(initial.valid,"single hull carves");
    auto shell=ShipInteriorDerivedShellSystem::Build(initial);Check(shell.ready,"single hull has ready derived shell");
    Check(shell.surfaces.size()==6,"single hull has exactly six sides");
    for(const auto& s:shell.surfaces){Check(s.blocksMovement&&s.pressureBoundary,"render/collision surface parity");
        Check(s.corners[0].x!=s.corners[2].x||s.corners[0].y!=s.corners[2].y||s.corners[0].z!=s.corners[2].z,
              "surface never degenerates");}
    ship.modules.push_back(Place("hull",0));
    auto identical=ShipInteriorDerivedShellSystem::Build(Carve(catalog,ship));
    Check(identical.ready&&identical.surfaces.size()==6,"coincident hulls dedupe all six exposed surfaces");
    ship.modules[1].x=3.90f;
    auto carve=Carve(catalog,ship);Check(carve.valid&&carve.portals.size()==1,"nearby assembled hulls connect");
    shell=ShipInteriorDerivedShellSystem::Build(carve);
    Check(shell.ready,"inset-separated modules generate a contiguous passage shell");
    Check(shell.openings.size()==1&&!shell.openings[0].exposedByUnion,"inset seam produces explicit opening");
    Check(shell.openings[0].gap>0&&shell.openings[0].gap<.5f,"opening bridges only the allowed hull inset gap");
    Check(shell.surfaces.size()>10,"cut door leaves real frame and passage liner geometry");
    const float planeA=carve.volumes[0].center.x+carve.volumes[0].halfExtents.x;
    const float planeB=carve.volumes[1].center.x-carve.volumes[1].halfExtents.x;
    int liner=0,faceA=0,faceB=0;
    for(const auto& s:shell.surfaces){
        Check(s.blocksMovement&&s.pressureBoundary,"derived render quad is the same collision quad");
        if(s.axis==0&&Near(s.corners[0].x,planeA)&&s.sourceModule==0){++faceA;
            Check(!Covers(s,0,0),"opening removes center from first hull and collision");}
        if(s.axis==0&&Near(s.corners[0].x,planeB)&&s.sourceModule==1){++faceB;
            Check(!Covers(s,0,0),"opening removes center from second hull and collision");}
        if(s.sourceModule==0&&s.axis!=0){
            bool spans=false;for(const auto& p:s.corners)if(Near(p.x,planeA))spans=true;
            bool spansB=false;for(const auto& p:s.corners)if(Near(p.x,planeB))spansB=true;
            if(spans&&spansB)++liner;
        }
    }
    // Slice 2 lowered the door opening to the actual floor. A floor-flush
    // opening has no bottom wall strip: only two jambs and a header remain
    // on each module face. Requiring a fourth strip would reintroduce the
    // knee-high collision barrier fixed by the runtime handoff.
    Check(faceA==3&&faceB==3,"floor-flush door has two jambs and header on both seam faces");
    const auto& doorway=shell.openings.front();
    const float floor=carve.volumes[0].center.z-carve.volumes[0].halfExtents.z;
    Check(Near(doorway.center.z-doorway.height*.5f,floor),
          "portal opening reaches the common deck floor without a blocking threshold");
    int floorLiners=0;
    for(const auto& s:shell.surfaces){
        if(s.sourceModule!=0||s.axis!=2)continue;
        bool spansA=false,spansB=false;
        for(const auto& corner:s.corners){
            if(Near(corner.x,planeA))spansA=true;
            if(Near(corner.x,planeB))spansB=true;
        }
        if(spansA&&spansB&&Near(s.corners[0].z,floor))++floorLiners;
    }
    Check(liner==4&&floorLiners==1,
          "passage has four liners including exactly one continuous floor collider");
    ship.modules[1].x=2.9f;
    shell=ShipInteriorDerivedShellSystem::Build(Carve(catalog,ship));
    Check(shell.ready&&shell.openings.size()==1&&shell.openings[0].exposedByUnion,
          "intersecting cavities open by union without redundant door cuts");
    Check(shell.surfaces.size()==10,"overlapping equal-section hulls suppress both internal full faces");
    ship.modules[1].y=1;
    carve=Carve(catalog,ship);
    shell=ShipInteriorDerivedShellSystem::Build(carve);
    Check(carve.valid&&shell.ready,"offset overlapping hulls preserve a valid union");
    const float partialPlane=carve.volumes[0].center.x+carve.volumes[0].halfExtents.x;
    bool exposedStrip=false;
    for(const auto& s:shell.surfaces)if(s.sourceModule==0&&s.axis==0&&s.direction==1&&Near(s.corners[0].x,partialPlane)){
        Check(!Covers(s,0,0),"partial overlap removes internal region from surface AND collider");
        if(Covers(s,-1.5f,0))exposedStrip=true;
    }
    Check(exposedStrip,"partial overlap keeps genuine exterior strip");
    ship.modules[1].y=0;
    ship.modules[1].x=4.2f;
    carve=Carve(catalog,ship);
    shell=ShipInteriorDerivedShellSystem::Build(carve);
    Check(carve.valid&&!shell.ready&&!shell.errors.empty()&&shell.surfaces.empty(),
          "logical portal with excessive physical gap fails closed, not an invisible bridge");
    ship.modules[1].x=8;
    shell=ShipInteriorDerivedShellSystem::Build(Carve(catalog,ship));
    Check(!shell.ready&&shell.surfaces.empty(),"disconnected carve cannot silently produce certified shell");
    ship.modules.resize(1);ship.modules.push_back(Place("wing",0));
    carve=Carve(catalog,ship);shell=ShipInteriorDerivedShellSystem::Build(carve);
    Check(shell.ready&&carve.exclusions.size()==1&&shell.surfaces.size()==6,"excluded wing never becomes walkable cavity");
    ship.modules.resize(1);ship.modules[0].yawDegrees=45;
    shell=ShipInteriorDerivedShellSystem::Build(Carve(catalog,ship));
    Check(!shell.ready&&shell.surfaces.empty()&&!shell.errors.empty(),
          "rotated hull fails closed rather than creating misaligned colliders");
    ship.modules[0].yawDegrees=0;
    ShipInteriorLayoutSystem layout;
    auto plan=layout.Plan(42,catalog,ship);
    Check(plan.carve.valid&&plan.shell.ready&&plan.shell.surfaces.size()==6,
          "real interior layout exposes derived shell to downstream consumers");
    std::cout<<"PASS derived hollow shell: "<<checks<<" checks\n";
}
