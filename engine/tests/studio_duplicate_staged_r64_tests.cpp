#include "ship_editor/ShipyardBuilderSystem.h"
#include <cmath>
#include <iostream>
using namespace subspace;
namespace {int failures=0, assertions=0; void Check(bool ok,const char* n){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<n<<"\n";}else std::cout<<"[PASS] "<<n<<"\n";} ShipyardModuleRecord Make(){ShipyardModuleRecord r;r.source.moduleId="dup_part";r.source.halfWidth=.5f;r.source.halfLength=1.0f;r.source.halfHeight=.25f;r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;r.generatorEligible=true;return r;}}
int main(){
    auto rec=Make(); ProceduralShipVisualRecipe recipe; VisualModulePlacement p;p.moduleId=rec.source.moduleId;p.x=2;p.y=3;p.z=4;p.yawDegrees=37;p.scaleX=1.2f;p.scaleY=.8f;p.scaleZ=1.1f;p.mirrorX=true;recipe.modules.push_back(p);
    ShipyardBuilderSystem b;b.Initialize({rec},recipe);const auto before=b.Recipe();
    Check(b.Activate(ShipyardBuilderCommand::DuplicateSelection),"Construct duplicate command starts");
    Check(b.Model().dragPreview.active&&b.Model().dragPreview.staged,"Construct duplicate is staged, not immediately committed");
    Check(b.Recipe().modules.size()==before.modules.size(),"staged duplicate does not mutate authored recipe");
    const auto& g=b.Model().dragPreview.ghost;
    Check(g.moduleId==p.moduleId&&std::fabs(g.yawDegrees-p.yawDegrees)<.001f,"staged duplicate preserves identity and rotation");
    Check(std::fabs(g.scaleX-p.scaleX)<.001f&&std::fabs(g.scaleY-p.scaleY)<.001f&&std::fabs(g.scaleZ-p.scaleZ)<.001f,"staged duplicate preserves non-uniform scale");
    Check(g.mirrorX==p.mirrorX,"staged duplicate preserves mirror state");
    Check(b.Activate(ShipyardBuilderCommand::ConfirmPlacement),"explicit confirm commits duplicate");
    Check(b.Recipe().modules.size()==2,"confirmed duplicate creates exactly one module");
    Check(b.AuthoringUndoCount()==1,"duplicate confirmation creates exactly one undo entry");
    Check(b.UndoAuthoring()&&b.Recipe().modules.size()==1,"one Undo removes the confirmed duplicate");
    Check(b.RedoAuthoring()&&b.Recipe().modules.size()==2,"Redo restores the duplicate");
    std::cout<<"R64 staged duplicate assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";return failures?1:0;
}
