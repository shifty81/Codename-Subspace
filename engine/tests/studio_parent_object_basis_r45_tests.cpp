#include "editor/ConstructionTransformBasisSystem.h"
#include "ship_editor/ShipyardTransformSpacePolicy.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace subspace;
namespace { bool near(float a,float b){return std::fabs(a-b)<.001f;} }
int main(){
    VisualModulePlacement rotated{};rotated.yawDegrees=63.0f;rotated.pitchDegrees=21.0f;rotated.rollDegrees=-17.0f;
    const auto parent=ConstructionTransformBasisSystem::ShipWorld(0.0f,{1,1,1});
    const auto object=ConstructionTransformBasisSystem::AssemblyWorld(rotated,0.0f,{1,1,1},true);
    // Rotating the object cannot rotate the PARENT frame.
    assert(near(parent.x.x,1)&&near(parent.x.y,0)&&near(parent.y.x,0)&&near(parent.y.y,1)&&near(parent.z.z,1));
    assert(!near(object.x.x,parent.x.x)||!near(object.x.y,parent.x.y)||!near(object.x.z,parent.x.z));
    assert(ShipyardTransformSpacePolicy::Effective(ShipyardTransformTool::Move,ShipyardTransformSpace::Ship)==ShipyardTransformSpace::Ship);
    assert(ShipyardTransformSpacePolicy::Effective(ShipyardTransformTool::Rotate,ShipyardTransformSpace::Ship)==ShipyardTransformSpace::Ship);
    assert(ShipyardTransformSpacePolicy::Effective(ShipyardTransformTool::Scale,ShipyardTransformSpace::Ship)==ShipyardTransformSpace::Local);
    std::cout<<"R45 parent/object basis separation: PASS\n";
}
