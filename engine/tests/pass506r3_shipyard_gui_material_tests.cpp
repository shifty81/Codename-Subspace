#include "content/ShipyardAuthoredOrientation.generated.h"
#include "content/ShipyardModuleSystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardOrientationConstraintSystem.h"
#include "ship_editor/ShipyardPaintZoneSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;

namespace {
int failures=0,assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}

bool Overlap(const ShipyardBuilderControl& a,const ShipyardBuilderControl& b){
    return a.x < b.x+b.width && a.x+a.width > b.x &&
           a.y < b.y+b.height && a.y+a.height > b.y;
}

Vector3 RotateLocal(Vector3 v,const VisualModulePlacement& p){
    constexpr float d=3.14159265358979323846f/180.0f;
    const float yaw=p.yawDegrees*d,pitch=p.pitchDegrees*d,roll=p.rollDegrees*d;
    const float cr=std::cos(roll),sr=std::sin(roll);
    const float x1=v.x*cr+v.z*sr,y1=v.y,z1=-v.x*sr+v.z*cr;
    const float cp=std::cos(pitch),sp=std::sin(pitch);
    const float x2=x1,y2=y1*cp-z1*sp,z2=y1*sp+z1*cp;
    const float cy=std::cos(yaw),sy=std::sin(yaw);
    return {x2*cy-y2*sy,x2*sy+y2*cy,z2};
}

ShipyardModuleRecord Hull(){
    ShipyardModuleRecord r;
    r.source.moduleId="shipyard_a_hull_094_shipyard_hull_001_test";
    r.source.halfWidth=3.0f;r.source.halfLength=5.0f;r.source.halfHeight=1.8f;
    r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;
    r.size=ShipyardModuleSize::L;r.primaryHull=true;r.generatorEligible=true;
    r.sockets.push_back({"starboard_mid","lateral_surface",3.0f,0,0,1,0,0,0.10f});
    r.sockets.push_back({"port_mid","lateral_surface",-3.0f,0,0,-1,0,0,0.10f});
    return r;
}

ShipyardModuleRecord Wing147(){
    ShipyardModuleRecord r;
    r.source.moduleId="shipyard_a_wing_147_shipyard_wing_001_miscblockfinger";
    // Runtime-remapped extents measured from the certified OBJ.
    r.source.halfWidth=2.025722f*.5f;
    r.source.halfLength=3.296277f*.5f;
    r.source.halfHeight=2.103614f*.5f;
    r.moduleClass=ShipyardModuleClass::Wing;r.semantic=ShipyardModuleSemantic::Wing;
    r.size=ShipyardModuleSize::M;r.partRole=ShipyardPartRole::Wing;
    r.placementRole="LATERAL_STRUCTURE";r.generatorEligible=true;r.pairedPlacement=true;
    // Root-preserving lateral mount: pitch 180 changes forward/up but leaves X.
    r.sockets.push_back({"mount","lateral_mount",-r.source.halfWidth,0,0,-1,0,0,0.08f});
    return r;
}

VisualModulePlacement PlacementFor(const ShipyardModuleRecord& r){
    VisualModulePlacement p;p.moduleId=r.source.moduleId;
    p.scaleX=p.scaleY=p.scaleZ=1.0f;return p;
}
}

int main(){
    Check(ShipyardPaintZoneSystem::ForMaterial("Flat")==ShipyardPaintZone::Primary,
          "Greyoxide Flat resolves to PRIMARY instead of silent gray");
    Check(ShipyardPaintZoneSystem::ForMaterial("Flat.001")==ShipyardPaintZone::Secondary,
          "Greyoxide Flat.001 resolves to SECONDARY");
    Check(ShipyardPaintZoneSystem::ForMaterial("Glass")==ShipyardPaintZone::Glass,
          "Greyoxide Glass preserves GLASS semantics");
    Check(ShipyardPaintZoneSystem::ForMaterial("totally_unknown_region")==ShipyardPaintZone::Inherited,
          "unknown regions remain diagnosable rather than guessed");

    ShipyardBuilderRuntimeModel ui;
    ui.initialized=true;ui.catalog={Hull(),Wing147()};
    ui.selectedClass=ShipyardModuleClass::Wing;
    ui.transformTool=ShipyardTransformTool::Rotate; // R7 semantic successor: rotation palette is contextual.
    for(int i=0;i<12;++i){auto p=PlacementFor(i%2?ui.catalog[1]:ui.catalog[0]);p.x=float(i);ui.recipe.modules.push_back(p);}
    const auto controls=ShipyardBuilderSystem::BuildControls(ui,1653,930);
    Check(!controls.empty(),"Shipyard controls build at the certified 1653x930 viewport");
    bool overlap=false;
    for(std::size_t i=0;i<controls.size();++i)for(std::size_t j=i+1;j<controls.size();++j)
        if(Overlap(controls[i],controls[j])){overlap=true;std::cerr<<"[OVERLAP] "<<controls[i].label<<" vs "<<controls[j].label<<"\n";}
    Check(!overlap,"all visible Shipyard control hit rectangles are non-overlapping");
    const auto has=[&](ShipyardBuilderCommand c){return std::any_of(controls.begin(),controls.end(),[&](const auto& x){return x.command==c;});};
    Check(has(ShipyardBuilderCommand::RotatePitchPositive)&&has(ShipyardBuilderCommand::RotatePitchNegative),
          "pitch controls are visible");
    Check(has(ShipyardBuilderCommand::RotateYawPositive)&&has(ShipyardBuilderCommand::RotateYawNegative),
          "yaw controls are visible");
    Check(has(ShipyardBuilderCommand::RotateRollPositive)&&has(ShipyardBuilderCommand::RotateRollNegative),
          "roll controls are visible");
    Check(has(ShipyardBuilderCommand::FlipPitch)&&has(ShipyardBuilderCommand::FlipYaw)&&has(ShipyardBuilderCommand::FlipRoll),
          "180-degree flip controls are visible");
    Check(has(ShipyardBuilderCommand::FrameSelected)&&has(ShipyardBuilderCommand::FrameShip),
          "Frame Part and Frame Ship controls are visible");

    auto hull=Hull();auto wing=Wing147();
    const auto* authored=FindShipyardAuthoredOrientation("miscblockfinger");
    Check(authored!=nullptr,"Wing147 has user-certified source-basis metadata");
    if(authored){
        Check(authored->forwardY<-.9f && authored->upZ<-.9f,
              "Wing147 source basis records small end forward and broad face dorsal");
    }

    auto bad=PlacementFor(wing);
    Check(!ShipyardOrientationConstraintSystem::Validate(wing,bad).valid,
          "unflipped Wing147 fails source-basis validation");

    auto parent=PlacementFor(hull);
    const auto placed=ShipyardModuleSystem::BuildAttachmentPlacement(
        static_cast<const VisualModulePlacement&>(parent),
        static_cast<const ShipyardAssemblySocket&>(hull.sockets[0]),
        static_cast<const ShipyardModuleRecord&>(wing),
        static_cast<const ShipyardAssemblySocket&>(wing.sockets[0]),1.0f);
    const float mirror=placed.mirrorX?-1.0f:1.0f;
    Vector3 fwd=RotateLocal({0.0f*mirror,-1.0f,0.0f},placed).normalized();
    Vector3 up=RotateLocal({0.0f*mirror,0.0f,-1.0f},placed).normalized();
    Check(fwd.y>.90f,"Wing147 solver rotates the certified small/leading end toward ship forward");
    Check(up.z>.90f,"Wing147 solver flips the certified broad surface dorsal/up");
    Check(ShipyardOrientationConstraintSystem::Validate(wing,placed).valid,
          "corrected Wing147 attachment passes orientation validation");
    Check(std::fabs(std::fmod(std::fabs(placed.pitchDegrees),360.0f)-180.0f)<.1f ||
          std::fabs(std::fmod(std::fabs(placed.rollDegrees),360.0f)-180.0f)<.1f,
          "Wing147 correction uses a root-preserving 180-degree orientation family");

    auto manualBad=placed;
    manualBad.pitchDegrees=0.0f;manualBad.yawDegrees=0.0f;manualBad.rollDegrees=0.0f;
    ProceduralShipVisualRecipe recipe;recipe.role="INDUSTRIAL";recipe.seed=5063;recipe.modules={PlacementFor(hull),manualBad};
    recipe.attachments.push_back({0,1,"starboard_mid","mount",0.0f,true});
    ShipyardBuilderSystem builder;builder.Initialize({hull,wing},recipe);
    Check(builder.Activate(ShipyardBuilderCommand::SelectPlaced,1),"Wing147 selection succeeds");
    Check(builder.Activate(ShipyardBuilderCommand::ToolRotate),"rotate tool activates");
    Check(builder.RotateSelected({180.0f,0.0f,0.0f}),"manual 180-degree pitch rotation is accepted");
    const auto* working=builder.SelectedPlacedModule();
    Check(working&&std::fabs(std::fabs(working->pitchDegrees)-180.0f)<.1f,
          "manual rotation remains at 180 degrees instead of being silently normalized away");
    Check(builder.CommitTransform(),"manual 180-degree transform commits");
    const auto* committedWing=builder.SelectedPlacedModule();
    Check(committedWing&&ShipyardOrientationConstraintSystem::Validate(wing,*committedWing).valid,
          "committed corrected Wing147 orientation validates even in a deliberately minimal test hull");

    const float step0=builder.Model().rotationStepDegrees;
    Check(builder.Activate(ShipyardBuilderCommand::CycleRotationStep),"rotation step cycle activates");
    Check(step0==15.0f && builder.Model().rotationStepDegrees==5.0f,"rotation step cycles 15 to 5 degrees");
    builder.Activate(ShipyardBuilderCommand::CycleRotationStep);
    Check(builder.Model().rotationStepDegrees==1.0f,"rotation step cycles 5 to 1 degree");
    builder.Activate(ShipyardBuilderCommand::CycleRotationStep);
    Check(builder.Model().rotationStepDegrees==15.0f,"rotation step cycles 1 back to 15 degrees");

    std::cout<<"Pass506R3 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
