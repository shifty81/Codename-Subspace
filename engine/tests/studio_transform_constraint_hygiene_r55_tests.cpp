#include "ship_editor/ShipyardBuilderSystem.h"
#include <cassert>
#include <iostream>
using namespace subspace;
namespace {
ShipyardModuleRecord Make(){
    ShipyardModuleRecord r;r.source.moduleId="hull_a";r.moduleClass=ShipyardModuleClass::Hull;
    r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;r.generatorEligible=true;return r;
}
}
int main(){
    VisualModulePlacement p;p.moduleId="hull_a";p.scaleX=p.scaleY=p.scaleZ=1.0f;
    ProceduralShipVisualRecipe recipe;recipe.role="INDUSTRIAL";recipe.modules={p};
    ShipyardBuilderSystem b;b.Initialize({Make()},recipe);
    assert(b.Model().transformSpace==ShipyardTransformSpace::Ship);

    b.Activate(ShipyardBuilderCommand::ToolMove);
    assert(b.SetTransformConstraint(ShipyardTransformConstraint::X));
    assert(b.SetTransformConstraint(ShipyardTransformConstraint::X));
    assert(b.Model().transformConstraintLocal);
    const bool dirtyBefore=b.Model().dirty;const auto undoBefore=b.AuthoringUndoCount();
    assert(b.Activate(ShipyardBuilderCommand::ToggleTransformSpace));
    assert(b.Model().transformSpace==ShipyardTransformSpace::Local);
    assert(b.Model().transformConstraint==ShipyardTransformConstraint::Free);
    assert(!b.Model().transformConstraintLocal);
    assert(b.Model().dirty==dirtyBefore&&b.AuthoringUndoCount()==undoBefore);

    b.SetTransformConstraint(ShipyardTransformConstraint::Y);
    b.SetTransformConstraint(ShipyardTransformConstraint::Y);
    assert(b.Model().transformConstraintLocal);
    assert(b.Activate(ShipyardBuilderCommand::ToolRotate));
    assert(b.Model().transformSpace==ShipyardTransformSpace::Local); // orientation choice persists
    assert(b.Model().transformConstraint==ShipyardTransformConstraint::Free);
    assert(!b.Model().transformConstraintLocal); // gesture/tool lock does not leak

    b.SetTransformConstraint(ShipyardTransformConstraint::Z);
    assert(b.Activate(ShipyardBuilderCommand::ToolScale));
    assert(b.Model().transformConstraint==ShipyardTransformConstraint::Free);
    assert(b.Model().transformSpace==ShipyardTransformSpace::Local);
    std::cout<<"R55 transform constraint hygiene: PASS\n";
}
