#include "studio/StudioTransformStatusPolicy.h"
#include <cassert>
#include <iostream>
#include <string>
using namespace subspace;
int main(){
    using P=StudioTransformStatusPolicy;
    assert(P::SelectedLabel(ShipyardTransformSpace::Ship)=="SPACE PARENT");
    assert(P::EffectiveLabel(ShipyardTransformTool::Move,ShipyardTransformSpace::Ship)=="MOVE PARENT");
    assert(!P::HasEffectiveOverride(ShipyardTransformTool::Move,ShipyardTransformSpace::Ship));
    assert(P::EffectiveLabel(ShipyardTransformTool::Scale,ShipyardTransformSpace::Ship)=="SCALE OBJECT");
    assert(P::HasEffectiveOverride(ShipyardTransformTool::Scale,ShipyardTransformSpace::Ship));
    assert(P::EffectiveLabel(ShipyardTransformTool::Rotate,ShipyardTransformSpace::View)=="ROTATE EULER");
    assert(P::EffectiveLabel(ShipyardTransformTool::Rotate,ShipyardTransformSpace::Local)=="ROTATE EULER");
    assert(P::HasEffectiveOverride(ShipyardTransformTool::Rotate,ShipyardTransformSpace::Ship));
    assert(P::EffectiveLabel(ShipyardTransformTool::Move,ShipyardTransformSpace::Ship,true)=="MOVE OBJECT");
    std::cout<<"R53/R82R1 transform status policy: PASS\n";
}
