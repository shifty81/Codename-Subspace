#include "ship_editor/ShipyardTransformSystem.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace subspace;
namespace { bool near(float a,float b){return std::fabs(a-b)<.001f;} }
int main(){
    VisualModulePlacement p{};p.scaleX=p.scaleY=p.scaleZ=1.0f;p.mirrorX=true;
    ShipyardTransformTransaction tx{};
    assert(ShipyardTransformSystem::Begin(tx,0,p,ShipyardTransformTool::Move,ShipyardTransformSpace::Local));
    tx.snap=false;ShipyardTransformSystem::Translate(tx,{2,0,0});
    assert(near(tx.working.x,-2.0f)&&near(tx.working.y,0.0f)&&near(tx.working.z,0.0f));
    std::cout<<"R82R1 mirrored OBJECT move semantics: PASS\n";
}
