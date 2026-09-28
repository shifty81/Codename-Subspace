#include "ship_editor/ShipyardBuilderSystem.h"
#include <iostream>
using namespace subspace;
namespace {int failures=0,assertions=0;void Check(bool ok,const char* n){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<n<<"\n";}else std::cout<<"[PASS] "<<n<<"\n";} ShipyardModuleRecord Make(){ShipyardModuleRecord r;r.source.moduleId="context_part";r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;r.generatorEligible=true;ShipyardAssemblySocket a;a.name="a";a.type="hull_aft";ShipyardAssemblySocket b;b.name="b";b.type="hull_forward";r.sockets={a,b};return r;}}
int main(){
    auto rec=Make();ProceduralShipVisualRecipe recipe;VisualModulePlacement p;p.moduleId=rec.source.moduleId;recipe.modules={p};ShipyardBuilderSystem b;b.Initialize({rec},recipe);
    Check(b.Activate(ShipyardBuilderCommand::InspectorSockets),"R78 enters SOCKETS context");
    const auto modulesBefore=b.Recipe().modules.size(),socketsBefore=b.Model().catalog.front().sockets.size();
    Check(!b.Activate(ShipyardBuilderCommand::DuplicateSelection)&&!b.Model().dragPreview.active,"R78 duplicate cannot silently duplicate module from SOCKETS");
    Check(b.Activate(ShipyardBuilderCommand::DeleteSelectionSafe),"R78 Delete Selected succeeds in SOCKETS");
    Check(b.Recipe().modules.size()==modulesBefore,"R78 SOCKETS Delete does not delete module");
    Check(b.Model().catalog.front().sockets.size()+1==socketsBefore,"R78 SOCKETS Delete removes one socket");
    b.Activate(ShipyardBuilderCommand::WorkspaceAppearance);
    const auto modulesAppearance=b.Recipe().modules.size();
    Check(!b.Activate(ShipyardBuilderCommand::DeleteSelectionSafe)&&b.Recipe().modules.size()==modulesAppearance,"R78 non-structural workspace Delete has no module target");
    b.Activate(ShipyardBuilderCommand::WorkspaceBuild);b.Activate(ShipyardBuilderCommand::InspectorTransform);
    Check(b.Activate(ShipyardBuilderCommand::DuplicateSelection)&&b.Model().dragPreview.staged,"R78 Construct duplicate remains available after context changes");
    b.Activate(ShipyardBuilderCommand::CancelPlacement);
    std::cout<<"R78 context-selection assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";return failures?1:0;
}
