#include "editor/ConstructionAxisContextSystem.h"
#include "editor/ConstructionTransformBasisSystem.h"
#include "ship_editor/ShipyardTransformSystem.h"
#include "studio/StudioGizmoDragPolicy.h"
#include "studio/StudioGizmoMath.h"
#include "studio/StudioTransformReadout.h"
#include <cmath>
#include <iostream>
#include <string>
using namespace subspace;
namespace {
int failures=0,assertions=0;
bool near(float a,float b){return std::fabs(a-b)<.001f;}
void Check(bool ok,const char* name){++assertions;if(ok)std::cout<<"[PASS] "<<name<<"\n";else{++failures;std::cerr<<"[FAIL] "<<name<<"\n";}}
}
int main(){
    const auto x=ConstructionAxisContextSystem::Describe(ConstructionAxis::X);
    const auto y=ConstructionAxisContextSystem::Describe(ConstructionAxis::Y);
    const auto z=ConstructionAxisContextSystem::Describe(ConstructionAxis::Z);
    Check(std::string(x.dimension)=="WIDTH"&&std::string(x.positive)=="+X"&&std::string(x.negative)=="-X","R34 X is WIDTH / +/-X");
    Check(std::string(y.dimension)=="LENGTH"&&std::string(y.positive)=="+Y"&&std::string(y.negative)=="-Y","R34 Y is LENGTH / +/-Y");
    Check(std::string(z.dimension)=="HEIGHT"&&std::string(z.positive)=="+Z"&&std::string(z.negative)=="-Z","R34 Z is HEIGHT / +/-Z");

    VisualModulePlacement basisPart{};basisPart.yawDegrees=90;
    const auto basis=ConstructionTransformBasisSystem::AssemblyWorld(basisPart,0,{1,1,1},true);
    Check(near(basis.x.x,0)&&near(basis.x.y,1)&&near(basis.x.z,0),"R34 90deg yaw maps local WIDTH/X to world +Y");
    Check(near(basis.y.x,-1)&&near(basis.y.y,0)&&near(basis.y.z,0),"R34 90deg yaw maps local LENGTH/Y to world -X");

    ProceduralShipVisualRecipe recipe{};recipe.widthScale=1.25f;recipe.lengthScale=.75f;
    VisualModulePlacement part{};part.moduleId="axis_contract";part.scaleX=part.scaleY=part.scaleZ=1;
    VisualModuleSource source{};source.moduleId=part.moduleId;source.halfWidth=2;source.halfLength=4;source.halfHeight=1;

    ShipyardTransformTransaction tx{};
    ShipyardTransformSystem::Begin(tx,0,part,ShipyardTransformTool::Scale);
    ShipyardTransformSystem::Scale(tx,{.5f,0,0});
    auto r=StudioTransformReadout::Build(tx.working,recipe,&source);
    Check(near(tx.working.scaleX,1.5f)&&near(tx.working.scaleY,1)&&near(tx.working.scaleZ,1),"R34 X scale edits only WIDTH/X");
    Check(near(r.nominalLocalMeters[0],7.5f)&&near(r.nominalLocalMeters[1],6.0f)&&near(r.nominalLocalMeters[2],2.0f),"R34 X scale readout preserves W/L/H mapping");

    ShipyardTransformSystem::Begin(tx,0,part,ShipyardTransformTool::Scale);
    ShipyardTransformSystem::Scale(tx,{0,.5f,0});
    r=StudioTransformReadout::Build(tx.working,recipe,&source);
    Check(near(tx.working.scaleX,1)&&near(tx.working.scaleY,1.5f)&&near(tx.working.scaleZ,1),"R34 Y scale edits only LENGTH/Y");
    Check(near(r.nominalLocalMeters[0],5.0f)&&near(r.nominalLocalMeters[1],9.0f)&&near(r.nominalLocalMeters[2],2.0f),"R34 Y scale readout preserves W/L/H mapping");

    ShipyardTransformSystem::Begin(tx,0,part,ShipyardTransformTool::Scale);
    ShipyardTransformSystem::Scale(tx,{0,0,.5f});
    r=StudioTransformReadout::Build(tx.working,recipe,&source);
    Check(near(tx.working.scaleX,1)&&near(tx.working.scaleY,1)&&near(tx.working.scaleZ,1.5f),"R34 Z scale edits only HEIGHT/Z");
    Check(near(r.nominalLocalMeters[0],5.0f)&&near(r.nominalLocalMeters[1],6.0f)&&near(r.nominalLocalMeters[2],3.0f),"R34 Z scale readout preserves W/L/H mapping");

    auto rotatePhysical=[&](StudioAxis physical,float amount){
        ShipyardTransformSystem::Begin(tx,0,part,ShipyardTransformTool::Rotate);
        // This block certifies axis/field mapping, not the separately tested
        // snapped editor gesture. Never ask an enabled 15-degree snap to yield
        // an unsnapped 20-degree value (historical R34 CRT abort root cause).
        tx.snap=false;
        const auto field=StudioGizmoMath::RotationFieldAxis(physical);
        const Vector3 packed{field==StudioAxis::X?amount:0,
                             field==StudioAxis::Y?amount:0,
                             field==StudioAxis::Z?amount:0};
        ShipyardTransformSystem::Rotate(tx,packed);
        return StudioTransformReadout::Build(tx.working,recipe,&source);
    };
    auto rx=rotatePhysical(StudioAxis::X,15);Check(near(rx.rotationDegrees[0],15)&&near(rx.rotationDegrees[1],0)&&near(rx.rotationDegrees[2],0),"R34 physical X rotation reads pitch/X");
    auto ry=rotatePhysical(StudioAxis::Y,20);Check(near(ry.rotationDegrees[0],0)&&near(ry.rotationDegrees[1],20)&&near(ry.rotationDegrees[2],0),"R34 physical Y rotation reads roll/Y");
    auto rz=rotatePhysical(StudioAxis::Z,25);Check(near(rz.rotationDegrees[0],0)&&near(rz.rotationDegrees[1],0)&&near(rz.rotationDegrees[2],25),"R34 physical Z rotation reads yaw/Z");
    ShipyardTransformSystem::Begin(tx,0,part,ShipyardTransformTool::Rotate);
    tx.snap=true;tx.rotationSnapDegrees=15.0f;
    ShipyardTransformSystem::Rotate(tx,{0,0,20});
    Check(near(tx.working.rollDegrees,15.0f),"R34 snapping 20-degree requested roll to 15 degrees is intentional");

    StudioAxisHandle hx{StudioAxis::X,{0,0},{68,0},true};hx.physicalPixelsPerUnit={32,0};hx.projectedAxisUsable=true;hx.fallbackPixelsPerUnit=32;
    StudioAxisHandle hy{StudioAxis::Y,{0,0},{0,-68},true};hy.physicalPixelsPerUnit={0,-24};hy.projectedAxisUsable=true;hy.fallbackPixelsPerUnit=24;
    Check(near(StudioGizmoDragPolicy::MoveUnits(hx,{64,17}),2),"R34 X drag projects along X handle");
    Check(near(StudioGizmoDragPolicy::MoveUnits(hx,{0,64}),0),"R34 X drag rejects perpendicular motion");
    Check(near(StudioGizmoDragPolicy::MoveUnits(hy,{11,-48}),2),"R34 Y drag projects along Y handle");
    Check(near(StudioGizmoDragPolicy::MoveUnits(hy,{48,0}),0),"R34 Y drag rejects perpendicular motion");
    std::cout<<"R34 axis-contract assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
