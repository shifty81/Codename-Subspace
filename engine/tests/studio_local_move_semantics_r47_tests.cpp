#include "ship_editor/ShipyardTransformSystem.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace subspace;
namespace { bool near(float a,float b){return std::fabs(a-b)<.001f;} }
int main(){
    VisualModulePlacement p{};p.scaleX=p.scaleY=p.scaleZ=1;p.yawDegrees=90.0f;
    ShipyardTransformTransaction parent{};
    assert(ShipyardTransformSystem::Begin(parent,0,p,ShipyardTransformTool::Move,ShipyardTransformSpace::Ship));
    parent.snap=false;ShipyardTransformSystem::Translate(parent,{2,0,0});
    assert(near(parent.working.x,2)&&near(parent.working.y,0));

    ShipyardTransformTransaction object{};
    assert(ShipyardTransformSystem::Begin(object,0,p,ShipyardTransformTool::Move,ShipyardTransformSpace::Local));
    object.snap=false;ShipyardTransformSystem::Translate(object,{2,0,0});
    // Deliberate OBJECT/local mode follows the rotated part. PARENT does not.
    assert(near(object.working.x,0)&&near(object.working.y,2));
    std::cout<<"R47 local move semantics: PASS\n";
}
