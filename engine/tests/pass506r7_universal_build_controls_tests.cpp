#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardTransformSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using namespace subspace;

namespace {
int failures=0,assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}
bool Near(float a,float b,float e=.0006f){return std::fabs(a-b)<=e;}
ShipyardModuleRecord Make(const std::string& id){ShipyardModuleRecord r;r.source.moduleId=id;r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;r.generatorEligible=true;return r;}
bool Has(const std::vector<ShipyardBuilderControl>& c,ShipyardBuilderCommand cmd){return std::any_of(c.begin(),c.end(),[&](const auto& v){return v.command==cmd;});}
bool NoOverlap(const std::vector<ShipyardBuilderControl>& c){
    for(std::size_t i=0;i<c.size();++i)for(std::size_t j=i+1;j<c.size();++j){
        const auto&a=c[i],&b=c[j];
        const bool overlap=a.x<b.x+b.width&&a.x+a.width>b.x&&a.y<b.y+b.height&&a.y+a.height>b.y;
        if(overlap)return false;
    }
    return true;
}
}

int main(){
    VisualModulePlacement p;p.moduleId="hull_a";p.scaleX=p.scaleY=p.scaleZ=1.0f;
    ShipyardTransformTransaction tx;
    Check(ShipyardTransformSystem::Begin(tx,0,p,ShipyardTransformTool::Move,ShipyardTransformSpace::View),"transform transaction supports camera/view space");
    tx.snap=true;tx.translationSnap=.10f;ShipyardTransformSystem::Translate(tx,{.10f,0,0},true);
    Check(Near(tx.working.x,.01f),"Shift/fine movement is one tenth the normal increment");
    ShipyardTransformSystem::Cancel(tx);

    ShipyardTransformSystem::Begin(tx,0,p,ShipyardTransformTool::Rotate,ShipyardTransformSpace::View);tx.snap=true;tx.rotationSnapDegrees=15.0f;
    ShipyardTransformSystem::Rotate(tx,{10.0f,0,0},true);
    Check(Near(tx.working.pitchDegrees,1.0f),"Shift/fine rotation is one tenth the normal delta");
    ShipyardTransformSystem::Cancel(tx);

    ShipyardTransformSystem::Begin(tx,0,p,ShipyardTransformTool::Scale,ShipyardTransformSpace::View);tx.snap=true;tx.scaleSnap=.05f;
    ShipyardTransformSystem::Scale(tx,{.05f,.05f,.05f},false);
    Check(Near(tx.working.scaleX,1.05f)&&Near(tx.working.scaleY,1.05f)&&Near(tx.working.scaleZ,1.05f),"scale tool supports snapped uniform module scaling");
    ShipyardTransformSystem::Scale(tx,{100,100,100},false);
    Check(Near(tx.working.scaleX,4.0f),"module scale clamps at safe authoring maximum");

    std::vector<ShipyardModuleRecord> catalog{Make("hull_a"),Make("hull_b")};
    ProceduralShipVisualRecipe recipe;recipe.role="INDUSTRIAL";recipe.seed=7;recipe.forwardAuthority="FORWARD_MARKER";
    VisualModulePlacement a;a.moduleId="hull_a";a.x=-1.0f;a.scaleX=a.scaleY=a.scaleZ=1.0f;
    VisualModulePlacement b;b.moduleId="hull_b";b.x=1.0f;b.scaleX=b.scaleY=b.scaleZ=1.0f;
    recipe.modules={a,b};
    ShipyardBuilderSystem builder;builder.Initialize(catalog,recipe);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::View,"player-friendly camera transform space is the default");
    builder.Activate(ShipyardBuilderCommand::ToggleTransformSpace);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::Ship,"transform space cycles camera to ship");
    builder.Activate(ShipyardBuilderCommand::ToggleTransformSpace);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::Local,"transform space cycles ship to local");
    builder.Activate(ShipyardBuilderCommand::ToggleTransformSpace);
    Check(builder.Model().transformSpace==ShipyardTransformSpace::View,"transform space cycles local back to camera");

    Check(builder.Activate(ShipyardBuilderCommand::ToolScale)&&builder.Model().transformTool==ShipyardTransformTool::Scale,"R-scale tool is a first-class builder mode");
    Check(builder.Activate(ShipyardBuilderCommand::ScaleUniformPositive),"selected part scale command applies");
    Check(Near(builder.Recipe().modules[0].scaleX,1.05f),"selected part scale persists on the module placement");
    Check(builder.Activate(ShipyardBuilderCommand::ResetScale)&&Near(builder.Recipe().modules[0].scaleX,1.0f),"selected part scale can reset independently");

    Check(builder.Activate(ShipyardBuilderCommand::ScaleAssemblyUp),"whole assembly scale command applies");
    Check(Near(builder.Recipe().modules[0].x,-1.05f)&&Near(builder.Recipe().modules[1].x,1.05f),"assembly scale expands child positions around assembly center");
    Check(Near(builder.Recipe().modules[0].scaleX,1.05f)&&Near(builder.Recipe().modules[1].scaleX,1.05f),"assembly scale expands every module uniformly");

    auto model=builder.Model();model.inspectorTab=ShipyardInspectorTab::Transform;model.transformTool=ShipyardTransformTool::Move;
    auto moveControls=ShipyardBuilderSystem::BuildControls(model,1852,797);
    Check(Has(moveControls,ShipyardBuilderCommand::ToolScale),"sleek transform toolbar exposes scale next to select/move/rotate");
    Check(Has(moveControls,ShipyardBuilderCommand::NudgePort)&&!Has(moveControls,ShipyardBuilderCommand::RotatePitchPositive)&&!Has(moveControls,ShipyardBuilderCommand::ScaleUniformPositive),"move tool shows only movement-specific controls");
    Check(NoOverlap(moveControls),"1852x797 move-mode control rectangles do not overlap");

    model.transformTool=ShipyardTransformTool::Scale;
    auto scaleControls=ShipyardBuilderSystem::BuildControls(model,1280,768);
    Check(Has(scaleControls,ShipyardBuilderCommand::ScaleUniformPositive)&&Has(scaleControls,ShipyardBuilderCommand::ScaleAssemblyUp),"scale mode exposes part and whole-assembly scaling");
    Check(!Has(scaleControls,ShipyardBuilderCommand::NudgePort)&&!Has(scaleControls,ShipyardBuilderCommand::RotatePitchPositive),"scale mode removes unrelated move/rotate button matrices");
    Check(NoOverlap(scaleControls),"1280x768 scale-mode control rectangles do not overlap");

    std::cout<<"Pass506R7 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
