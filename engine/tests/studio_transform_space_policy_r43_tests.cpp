#include "ship_editor/ShipyardTransformSpacePolicy.h"
#include <cassert>
#include <iostream>
#include <string>
using namespace subspace;
int main(){
    assert(ShipyardTransformSpacePolicy::Default()==ShipyardTransformSpace::Ship);
    assert(std::string(ShipyardTransformSpacePolicy::Name(ShipyardTransformSpace::Ship))=="PARENT");
    assert(std::string(ShipyardTransformSpacePolicy::Name(ShipyardTransformSpace::Local))=="OBJECT");
    assert(std::string(ShipyardTransformSpacePolicy::Name(ShipyardTransformSpace::View))=="VIEW");
    assert(ShipyardTransformSpacePolicy::Next(ShipyardTransformSpace::Ship)==ShipyardTransformSpace::Local);
    assert(ShipyardTransformSpacePolicy::Next(ShipyardTransformSpace::Local)==ShipyardTransformSpace::View);
    assert(ShipyardTransformSpacePolicy::Next(ShipyardTransformSpace::View)==ShipyardTransformSpace::Ship);
    assert(ShipyardTransformSpacePolicy::Effective(ShipyardTransformTool::Move,ShipyardTransformSpace::Local)==ShipyardTransformSpace::Local);
    assert(ShipyardTransformSpacePolicy::Effective(ShipyardTransformTool::Move,ShipyardTransformSpace::View)==ShipyardTransformSpace::View);
    assert(ShipyardTransformSpacePolicy::Effective(ShipyardTransformTool::Rotate,ShipyardTransformSpace::Local)==ShipyardTransformSpace::Ship);
    assert(ShipyardTransformSpacePolicy::Effective(ShipyardTransformTool::Rotate,ShipyardTransformSpace::View)==ShipyardTransformSpace::Ship);
    assert(ShipyardTransformSpacePolicy::Effective(ShipyardTransformTool::Scale,ShipyardTransformSpace::Ship)==ShipyardTransformSpace::Local);
    assert(!ShipyardTransformSpacePolicy::FollowsObject(ShipyardTransformTool::Rotate,ShipyardTransformSpace::Local));
    assert(ShipyardTransformSpacePolicy::FollowsObject(ShipyardTransformTool::Scale,ShipyardTransformSpace::Ship));
    assert(ShipyardTransformSpacePolicy::FollowsObject(ShipyardTransformTool::Move,ShipyardTransformSpace::Local));
    std::cout<<"R43/R82R1 transform-space policy: PASS\n";
}
