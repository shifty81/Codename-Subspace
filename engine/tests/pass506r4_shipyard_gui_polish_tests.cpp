#include "ship_editor/ShipyardBuilderSystem.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;

namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}

bool Overlap(const ShipyardBuilderControl& a,const ShipyardBuilderControl& b){
    return a.x < b.x+b.width && a.x+a.width > b.x &&
           a.y < b.y+b.height && a.y+a.height > b.y;
}

ShipyardModuleRecord Make(ShipyardModuleClass cls,int n){
    ShipyardModuleRecord r;
    r.source.moduleId="shipyard_polish_"+std::to_string(static_cast<int>(cls))+"_"+std::to_string(n);
    r.moduleClass=cls;
    r.semantic=cls==ShipyardModuleClass::Wing?ShipyardModuleSemantic::Wing:ShipyardModuleSemantic::HullMid;
    r.size=ShipyardModuleSize::M;
    r.generatorEligible=true;
    return r;
}

ShipyardBuilderRuntimeModel Model(){
    ShipyardBuilderRuntimeModel m;
    m.initialized=true;
    for(int c=0;c<8;++c)for(int i=0;i<(c==static_cast<int>(ShipyardModuleClass::Wing)?10:4);++i)m.catalog.push_back(Make(static_cast<ShipyardModuleClass>(c),i));
    m.selectedClass=ShipyardModuleClass::Wing;
    for(int i=0;i<12;++i){VisualModulePlacement p;p.moduleId=m.catalog[static_cast<std::size_t>(i%m.catalog.size())].source.moduleId;m.recipe.modules.push_back(p);}
    m.validation.valid=true;
    return m;
}
}

int main(){
    auto m=Model();
    for(const auto dims:std::vector<std::pair<int,int>>{{1653,930},{1600,900},{1280,768}}){
        const auto layout=ShipyardBuilderSystem::Layout(dims.first,dims.second);
        Check(layout.valid,"layout authority is valid for supported Shipyard viewport");
        Check(layout.right>layout.left+layout.leftWidth+300.0f,"layout preserves a useful central 3D inspection viewport");
        Check(layout.validationY<layout.statusY,"validation card remains above status bar");
        auto moveModel=m;moveModel.transformTool=ShipyardTransformTool::Move;
        const auto controls=ShipyardBuilderSystem::BuildControls(moveModel,dims.first,dims.second);
        Check(!controls.empty(),"controls are produced by the shared layout authority");
        bool overlap=false;
        for(std::size_t i=0;i<controls.size();++i)for(std::size_t j=i+1;j<controls.size();++j)if(Overlap(controls[i],controls[j]))overlap=true;
        Check(!overlap,"responsive Shipyard control rectangles do not overlap");
        const auto has=[&](ShipyardBuilderCommand c){return std::any_of(controls.begin(),controls.end(),[&](const auto& x){return x.command==c;});};
        Check(has(ShipyardBuilderCommand::NudgePort)&&has(ShipyardBuilderCommand::NudgeStarboard)&&
              has(ShipyardBuilderCommand::NudgeForward)&&has(ShipyardBuilderCommand::NudgeAft)&&
              has(ShipyardBuilderCommand::NudgeDorsal)&&has(ShipyardBuilderCommand::NudgeVentral),
              "all six move/nudge directions are visible in Move mode");
        auto rotateModel=m;rotateModel.transformTool=ShipyardTransformTool::Rotate;
        const auto rotateControls=ShipyardBuilderSystem::BuildControls(rotateModel,dims.first,dims.second);
        const auto hasRotate=[&](ShipyardBuilderCommand c){return std::any_of(rotateControls.begin(),rotateControls.end(),[&](const auto& x){return x.command==c;});};
        Check(hasRotate(ShipyardBuilderCommand::RotatePitchNegative)&&hasRotate(ShipyardBuilderCommand::RotatePitchPositive)&&
              hasRotate(ShipyardBuilderCommand::RotateYawNegative)&&hasRotate(ShipyardBuilderCommand::RotateYawPositive)&&
              hasRotate(ShipyardBuilderCommand::RotateRollNegative)&&hasRotate(ShipyardBuilderCommand::RotateRollPositive),
              "full three-axis rotation palette remains visible in Rotate mode");
        float minEnabledWidth=10000.0f;
        for(const auto& c:controls)if(c.enabled)minEnabledWidth=std::min(minEnabledWidth,c.width);
        for(const auto& c:rotateControls)if(c.enabled)minEnabledWidth=std::min(minEnabledWidth,c.width);
        Check(minEnabledWidth>=50.0f,"enabled editor buttons retain a practical click target width");
    }

    const auto controls=ShipyardBuilderSystem::BuildControls(m,1653,930);
    const auto wingClass=std::find_if(controls.begin(),controls.end(),[](const auto& c){return c.command==ShipyardBuilderCommand::SelectClass&&c.value==static_cast<int>(ShipyardModuleClass::Wing);});
    Check(wingClass!=controls.end()&&wingClass->label.find("10")!=std::string::npos,"library class rows expose live module counts");
    const auto qSelect=std::find_if(controls.begin(),controls.end(),[](const auto& c){return c.command==ShipyardBuilderCommand::ToolSelect;});
    const auto frame=std::find_if(controls.begin(),controls.end(),[](const auto& c){return c.command==ShipyardBuilderCommand::FrameSelected;});
    Check(qSelect!=controls.end()&&qSelect->label.find("[Q]")!=std::string::npos,"selection tool advertises its keyboard shortcut");
    Check(frame!=controls.end()&&frame->label.find("[F]")!=std::string::npos,"frame-part action advertises its keyboard shortcut");

    const auto tooShort=ShipyardBuilderSystem::Layout(1653,700);
    Check(!tooShort.valid,"layout fails closed instead of overlapping controls on an undersized viewport");

    std::cout<<"Pass506R4 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
