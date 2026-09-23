#include "editor/ConstructionAxisContextSystem.h"
#include "editor/ConstructionScalePivotSystem.h"
#include "editor/ConstructionSymmetrySystem.h"
#include "editor/ConstructionTransformBasisSystem.h"

#include <cmath>
#include <iostream>
#include <string>

using namespace subspace;
namespace {
int failures=0,checks=0;
void Check(bool ok,const char* msg){++checks;if(!ok){++failures;std::cerr<<"FAIL "<<msg<<"\n";}}
bool Near(float a,float b,float e=.0005f){return std::fabs(a-b)<=e;}
void Vec(const Vector3& v,float x,float y,float z,const char* msg){Check(Near(v.x,x)&&Near(v.y,y)&&Near(v.z,z),msg);}
}
int main(){
    const auto x=ConstructionAxisContextSystem::Describe(ConstructionAxis::X);
    const auto y=ConstructionAxisContextSystem::Describe(ConstructionAxis::Y);
    const auto z=ConstructionAxisContextSystem::Describe(ConstructionAxis::Z);
    Check(std::string(x.token)=="X"&&std::string(x.dimension)=="WIDTH","X is generic width axis");
    Check(std::string(y.token)=="Y"&&std::string(y.dimension)=="LENGTH","Y is generic length axis");
    Check(std::string(z.token)=="Z"&&std::string(z.dimension)=="HEIGHT","Z is generic height axis");
    Check(std::string(ConstructionSymmetrySystem::AxisName(ConstructionSymmetryAxis::PortStarboard))=="MIRROR X","legacy symmetry enum presents generic X");
    Check(std::string(ConstructionSymmetrySystem::LegacyAxisName(ConstructionSymmetryAxis::PortStarboard))=="PORT <-> STARBOARD","legacy ship wording remains available for old content");

    auto mb=ConstructionTransformBasisSystem::ModelLocal({0,0,90});
    Vec(mb.x,0,1,0,"model local X follows RzRyRx rotation");
    Vec(mb.y,-1,0,0,"model local Y follows RzRyRx rotation");
    Vec(mb.z,0,0,1,"model local Z remains up under Z rotation");

    VisualModulePlacement p{};p.yawDegrees=90;
    auto ab=ConstructionTransformBasisSystem::AssemblyLocal(p,false);
    Vec(ab.x,0,1,0,"assembly local X follows renderer yaw");
    Vec(ab.y,-1,0,0,"assembly local Y follows renderer yaw");
    p={};p.rollDegrees=90;ab=ConstructionTransformBasisSystem::AssemblyLocal(p,false);
    Vec(ab.x,0,0,-1,"assembly roll is renderer Y rotation");
    Vec(ab.z,1,0,0,"assembly rolled Z becomes +X");

    p={};p.yawDegrees=90;
    auto world=ConstructionTransformBasisSystem::AssemblyWorld(p,0,{2,3,4},false);
    Vec(world.x,0,3,0,"rotated local X receives root Y scale, not root X scale");
    Vec(world.y,-2,0,0,"rotated local Y receives root X scale");
    p={};p.mirrorX=true;ab=ConstructionTransformBasisSystem::AssemblyLocal(p,true);
    Vec(ab.x,-1,0,0,"mirrored part exposes mirrored visual X basis");

    const ConstructionTransformBasis identity{};
    Vec(ConstructionScalePivotSystem::OppositeFaceShift({2,4,6},{4,4,6},identity),1,0,0,
        "opposite-face scale keeps negative X face fixed");
    VisualModuleSource src{};src.halfWidth=2;src.halfLength=3;src.halfHeight=1;
    VisualModulePlacement before{};before.scaleX=before.scaleY=before.scaleZ=1;
    auto after=before;after.scaleX=2;
    Vec(ConstructionScalePivotSystem::AnchoredModulePosition(src,before,after),2,0,0,
        "module scale from 4m to 8m shifts center +2m");
    before.mirrorX=true;after=before;after.scaleX=2;
    Vec(ConstructionScalePivotSystem::AnchoredModulePosition(src,before,after),-2,0,0,
        "mirrored module anchors its visually opposite X face");

    std::cout<<"R22 contract assertions "<<(checks-failures)<<"/"<<checks<<" PASS\n";
    return failures?1:0;
}
