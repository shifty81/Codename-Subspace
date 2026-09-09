#include "ship_editor/ShipyardBuilderSystem.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;

namespace {
int failures=0,assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}

bool Overlap(const ShipyardBuilderControl& a,const ShipyardBuilderControl& b){
    return a.x < b.x+b.width && a.x+a.width > b.x && a.y < b.y+b.height && a.y+a.height > b.y;
}

ShipyardModuleRecord Make(ShipyardModuleClass cls,int i){
    ShipyardModuleRecord r;r.source.moduleId="r5_part_"+std::to_string(static_cast<int>(cls))+"_"+std::to_string(i);
    r.moduleClass=cls;r.semantic=cls==ShipyardModuleClass::Wing?ShipyardModuleSemantic::Wing:ShipyardModuleSemantic::HullMid;
    r.size=ShipyardModuleSize::M;r.generatorEligible=true;return r;
}

ShipyardBuilderRuntimeModel Model(ShipyardInspectorTab tab){
    ShipyardBuilderRuntimeModel m;m.initialized=true;m.inspectorTab=tab;m.validation.valid=true;
    for(int c=0;c<8;++c)for(int i=0;i<10;++i)m.catalog.push_back(Make(static_cast<ShipyardModuleClass>(c),i));
    m.selectedClass=ShipyardModuleClass::Wing;
    for(int i=0;i<15;++i){VisualModulePlacement p;p.moduleId=m.catalog[static_cast<std::size_t>(i)].source.moduleId;m.recipe.modules.push_back(p);}
    return m;
}

bool Has(const std::vector<ShipyardBuilderControl>& controls,ShipyardBuilderCommand c){
    return std::any_of(controls.begin(),controls.end(),[&](const auto& x){return x.command==c;});
}
}

int main(){
    for(const auto dims:std::vector<std::pair<int,int>>{{1852,797},{1653,930},{1600,900},{1280,768}}){
        const auto layout=ShipyardBuilderSystem::Layout(dims.first,dims.second);
        Check(layout.valid,"R5 responsive layout is valid at supported viewport");
        Check(layout.tabHeight>=34.0f,"inspector tabs use a deliberate click target height");
        Check(layout.right>layout.left+layout.leftWidth+300.0f,"R5 preserves a useful central 3D viewport");
        Check(layout.statusY>layout.validationY,"validation remains clear of the status bar");

        for(const auto tab:{ShipyardInspectorTab::Transform,ShipyardInspectorTab::Assembly,ShipyardInspectorTab::Appearance}){
            const auto controls=ShipyardBuilderSystem::BuildControls(Model(tab),dims.first,dims.second);
            bool overlap=false;
            for(std::size_t i=0;i<controls.size();++i)for(std::size_t j=i+1;j<controls.size();++j)if(Overlap(controls[i],controls[j]))overlap=true;
            Check(!overlap,"active inspector page has no overlapping clickable rectangles");
            Check(Has(controls,ShipyardBuilderCommand::InspectorTransform)&&Has(controls,ShipyardBuilderCommand::InspectorAssembly)&&Has(controls,ShipyardBuilderCommand::InspectorAppearance),
                  "all three workflow tabs remain visible");
        }
    }

    auto transformModel=Model(ShipyardInspectorTab::Transform);transformModel.transformTool=ShipyardTransformTool::Rotate;
    auto transform=ShipyardBuilderSystem::BuildControls(transformModel,1852,797);
    Check(Has(transform,ShipyardBuilderCommand::RotatePitchPositive)&&Has(transform,ShipyardBuilderCommand::RotateYawPositive)&&Has(transform,ShipyardBuilderCommand::RotateRollPositive),
          "Transform page exposes full three-axis rotation");
    Check(Has(transform,ShipyardBuilderCommand::FlipPitch)&&Has(transform,ShipyardBuilderCommand::FlipYaw)&&Has(transform,ShipyardBuilderCommand::FlipRoll),
          "Transform page exposes 180-degree flips for every axis");
    Check(!Has(transform,ShipyardBuilderCommand::GenerateVariant)&&!Has(transform,ShipyardBuilderCommand::AddDecal),
          "Transform page is not polluted by assembly or appearance actions");

    auto assembly=ShipyardBuilderSystem::BuildControls(Model(ShipyardInspectorTab::Assembly),1852,797);
    Check(Has(assembly,ShipyardBuilderCommand::GenerateVariant)&&Has(assembly,ShipyardBuilderCommand::SaveBlueprint),
          "Assembly page owns blueprint generation and save actions");
    Check(!Has(assembly,ShipyardBuilderCommand::RotatePitchPositive)&&!Has(assembly,ShipyardBuilderCommand::AddDecal),
          "Assembly page avoids unrelated transform and appearance actions");

    auto appearance=ShipyardBuilderSystem::BuildControls(Model(ShipyardInspectorTab::Appearance),1852,797);
    Check(Has(appearance,ShipyardBuilderCommand::NextLiveryPreset)&&Has(appearance,ShipyardBuilderCommand::AddDecal),
          "Appearance page owns paint and decal actions");
    Check(Has(appearance,ShipyardBuilderCommand::NextPrimaryPaint)&&Has(appearance,ShipyardBuilderCommand::NextSecondaryPaint)&&Has(appearance,ShipyardBuilderCommand::NextTrimPaint),
          "Appearance page exposes independent primary secondary and accent paint controls");
    Check(!Has(appearance,ShipyardBuilderCommand::RotatePitchPositive)&&!Has(appearance,ShipyardBuilderCommand::GenerateVariant),
          "Appearance page avoids unrelated transform and generation actions");

    ShipyardBuilderSystem builder;
    auto seed=Model(ShipyardInspectorTab::Transform);
    ProceduralShipVisualRecipe recipe=seed.recipe;
    builder.Initialize(seed.catalog,recipe);
    Check(builder.Activate(ShipyardBuilderCommand::InspectorAssembly)&&builder.Model().inspectorTab==ShipyardInspectorTab::Assembly,
          "Assembly tab activation updates runtime page state");
    Check(builder.Activate(ShipyardBuilderCommand::InspectorAppearance)&&builder.Model().inspectorTab==ShipyardInspectorTab::Appearance,
          "Appearance tab activation updates runtime page state");
    Check(builder.Activate(ShipyardBuilderCommand::InspectorTransform)&&builder.Model().inspectorTab==ShipyardInspectorTab::Transform,
          "Transform tab activation updates runtime page state");

    const auto beforePreset=builder.Appearance().primary;
    Check(builder.Activate(ShipyardBuilderCommand::NextLiveryPreset),"paint preset activation succeeds");
    Check(builder.Appearance().primary.r!=beforePreset.r||builder.Appearance().primary.g!=beforePreset.g||builder.Appearance().primary.b!=beforePreset.b,
          "paint preset visibly changes primary livery data");
    const auto beforePrimary=builder.Appearance().primary;
    Check(builder.Activate(ShipyardBuilderCommand::NextPrimaryPaint),"independent primary paint activation succeeds");
    Check(builder.Appearance().primary.r!=beforePrimary.r||builder.Appearance().primary.g!=beforePrimary.g||builder.Appearance().primary.b!=beforePrimary.b,
          "independent primary paint changes live appearance data");
    const auto beforeSecondary=builder.Appearance().secondary;
    Check(builder.Activate(ShipyardBuilderCommand::NextSecondaryPaint),"independent secondary paint activation succeeds");
    Check(builder.Appearance().secondary.r!=beforeSecondary.r||builder.Appearance().secondary.g!=beforeSecondary.g||builder.Appearance().secondary.b!=beforeSecondary.b,
          "independent secondary paint changes live appearance data");
    const auto beforeTrim=builder.Appearance().trim;
    Check(builder.Activate(ShipyardBuilderCommand::NextTrimPaint),"independent accent paint activation succeeds");
    Check(builder.Appearance().trim.r!=beforeTrim.r||builder.Appearance().trim.g!=beforeTrim.g||builder.Appearance().trim.b!=beforeTrim.b,
          "independent accent paint changes live appearance data");
    Check(builder.Model().dirty,"paint changes mark the blueprint dirty");

    std::cout<<"Pass506R5 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
