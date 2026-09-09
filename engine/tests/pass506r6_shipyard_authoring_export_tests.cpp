#include "ship_editor/ShipyardAuthoringSampleSystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;

namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}

ShipyardModuleRecord Make(const std::string& id,ShipyardModuleClass cls,ShipyardModuleSemantic semantic){
    ShipyardModuleRecord r;r.source.moduleId=id;r.moduleClass=cls;r.semantic=semantic;r.size=ShipyardModuleSize::M;r.generatorEligible=true;return r;
}

const ShipyardBuilderControl* Find(const std::vector<ShipyardBuilderControl>& controls,ShipyardBuilderCommand command){
    const auto it=std::find_if(controls.begin(),controls.end(),[&](const auto& c){return c.command==command;});
    return it==controls.end()?nullptr:&*it;
}
}

int main(){
    std::vector<ShipyardModuleRecord> catalog{
        Make("shipyard_hull_custom",ShipyardModuleClass::Hull,ShipyardModuleSemantic::HullMid),
        Make("shipyard_engine_custom",ShipyardModuleClass::Propulsion,ShipyardModuleSemantic::MainEngine)
    };

    ProceduralShipVisualRecipe baseline;baseline.role="INDUSTRIAL";baseline.seed=77;baseline.forwardAuthority="COCKPIT";
    VisualModulePlacement hull;hull.moduleId="shipyard_hull_custom";baseline.modules.push_back(hull);
    VisualModulePlacement engine;engine.moduleId="shipyard_engine_custom";engine.x=1.0f;engine.y=-2.0f;engine.material=SpaceMaterialKind::EngineHousing;baseline.modules.push_back(engine);

    auto edited=baseline;edited.modules[1].x=2.5f;edited.modules[1].yawDegrees=35.0f;
    VisualModulePlacement extra;extra.moduleId="shipyard_hull_custom";extra.x=-1.7f;edited.modules.push_back(extra);

    ShipAppearanceState appearance;appearance.primary.r=.2f;appearance.primary.g=.4f;appearance.primary.b=.6f;
    const std::vector<std::string> errors{"Detached Shipyard attachment gap=1.5 module=shipyard_engine_custom"};
    const std::vector<std::string> warnings{"Custom silhouette requires review"};
    const auto json=ShipyardAuthoringSampleSystem::Serialize(baseline,edited,appearance,catalog,errors,warnings);
    Check(json.find("subspace.shipyard_authoring_sample")!=std::string::npos,"authoring sample uses dedicated reviewable schema");
    Check(json.find("PLAYER_GENERATOR_REFINEMENT_EXAMPLE")!=std::string::npos,"sample explicitly records generator-refinement intent");
    Check(json.find("\"generatorCertification\": \"DRAFT\"")!=std::string::npos,"invalid custom placement exports as draft instead of being rejected");
    Check(json.find("\"op\":\"TRANSFORM\"")!=std::string::npos,"sample records transform edits from generated baseline");
    Check(json.find("\"op\":\"ADD\"")!=std::string::npos,"sample records custom added modules");
    Check(json.find("Detached Shipyard attachment")!=std::string::npos,"sample preserves validation evidence for review");

    ShipyardBuilderSystem builder;builder.Initialize(catalog,edited);builder.SetLiveApplyEnabled(false,true);
    // The synthetic design is intentionally not generator-valid: saving must
    // still be possible as an authoring draft while runtime apply remains gated.
    const auto validation=builder.Validate();
    Check(!validation.valid,"custom synthetic design is not strict-generator certified");
    Check(builder.Activate(ShipyardBuilderCommand::SaveBlueprint),"custom invalid design may still request a draft save");
    Check(builder.ConsumeSaveRequested(),"draft save request reaches persistence layer");
    Check(builder.Model().status.find("draft")!=std::string::npos||builder.Model().status.find("Draft")!=std::string::npos,"draft save status clearly distinguishes review state");

    auto model=builder.Model();model.inspectorTab=ShipyardInspectorTab::Assembly;model.validation.valid=false;
    const auto controls=ShipyardBuilderSystem::BuildControls(model,1852,797);
    const auto* save=Find(controls,ShipyardBuilderCommand::SaveBlueprint);
    const auto* apply=Find(controls,ShipyardBuilderCommand::Apply);
    Check(save&&save->enabled&&save->label=="SAVE DRAFT","Assembly UI explicitly exposes Save Draft for non-certified custom ships");
    Check(apply&&!apply->enabled,"strict runtime/docked apply remains blocked for a non-certified draft");

    const auto certifiedJson=ShipyardAuthoringSampleSystem::Serialize(baseline,baseline,appearance,catalog,{},{});
    Check(certifiedJson.find("\"generatorCertification\": \"PASS\"")!=std::string::npos,"clean examples can be exported as generator-certified candidates");

    std::cout<<"Pass506R6 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
