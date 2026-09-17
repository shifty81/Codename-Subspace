#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardOverlayLayoutStore.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

using namespace subspace;
namespace {
int checks=0;
void Check(bool ok,const char* message){
    if(!ok){std::cerr<<"[FAIL] "<<message<<'\n';std::exit(EXIT_FAILURE);}
    ++checks;
}
bool Has(const std::vector<ShipyardBuilderControl>& controls,ShipyardBuilderCommand command){
    return std::any_of(controls.begin(),controls.end(),[&](const auto& c){return c.command==command;});
}
bool Materialized(const ShipyardBuilderRuntimeModel& model){
    const auto layers=SubspaceDockSystem::Materialize(model.dockWorkspace,1280,700,78.0f);
    return std::any_of(layers.begin(),layers.end(),[](const auto& d){
        return d.panelId=="asset_browser"&&d.visible&&d.rect.width>0&&d.rect.height>0;
    });
}
ShipyardModuleRecord MakeModule(){
    ShipyardModuleRecord record;
    record.source.moduleId="test_hull";
    record.moduleClass=ShipyardModuleClass::Hull;
    record.semantic=ShipyardModuleSemantic::HullMid;
    record.size=ShipyardModuleSize::M;
    record.generatorEligible=true;
    return record;
}
}
int main(){
    ProceduralShipVisualRecipe recipe;
    recipe.role="INDUSTRIAL";
    recipe.forwardAuthority="FORWARD_MARKER";
    VisualModulePlacement placement;
    placement.moduleId="test_hull";
    placement.scaleX=placement.scaleY=placement.scaleZ=1.0f;
    recipe.modules.push_back(placement);
    ShipyardBuilderSystem builder;
    builder.Initialize({MakeModule()},recipe);
    const auto normal=ShipyardBuilderSystem::BuildControls(builder.Model(),1280,768);
    Check(Has(normal,ShipyardBuilderCommand::DccRevealAssetBrowser),"global Assets recovery is in viewport chrome");
    Check(Has(normal,ShipyardBuilderCommand::DccResetLayout),"global Reset UI is in viewport chrome");
    Check(Materialized(builder.Model()),"default assets are materialized");

    Check(builder.Activate(ShipyardBuilderCommand::DccToggleAssetBrowser),"close Assets");
    Check(!Materialized(builder.Model()),"closed Assets are absent");
    const auto closed=ShipyardBuilderSystem::BuildControls(builder.Model(),1280,768);
    Check(Has(closed,ShipyardBuilderCommand::DccRevealAssetBrowser),"Assets recovery is outside the hidden panel");
    Check(!Has(closed,ShipyardBuilderCommand::DccAssetNext),"closed asset navigation cannot steal clicks");
    Check(builder.Activate(ShipyardBuilderCommand::DccRevealAssetBrowser),"recover closed Assets");
    Check(Materialized(builder.Model()),"recovered Assets render again");
    Check(Has(ShipyardBuilderSystem::BuildControls(builder.Model(),1280,768),
              ShipyardBuilderCommand::DccAssetNext),"recovered Assets regain navigation");

    auto* asset=SubspaceDockSystem::FindPanel(builder.MutableDockWorkspace(),"asset_browser");
    asset->collapsed=true;asset->autoHide=true;asset->hoverReveal=false;
    Check(builder.Activate(ShipyardBuilderCommand::DccRevealAssetBrowser),"recover collapsed/autohidden Assets");
    Check(!asset->collapsed&&!asset->autoHide&&asset->hoverReveal,"Assets body is expanded and revealed");
    Check(Materialized(builder.Model()),"expanded Assets render");

    auto* generator=SubspaceDockSystem::FindPanel(builder.MutableDockWorkspace(),"generator");
    auto* bottom=SubspaceDockSystem::FindNode(builder.MutableDockWorkspace(),"bottom");
    Check(generator&&bottom,"default bottom dock stack exists");
    generator->visible=true;bottom->activeTabId="generator";
    Check(!Materialized(builder.Model()),"another selected dock tab really hides Assets");
    Check(builder.Activate(ShipyardBuilderCommand::DccRevealAssetBrowser),"recover inactive Assets tab");
    Check(bottom->activeTabId=="asset_browser"&&Materialized(builder.Model()),"recovery selects the correct dock tab");

    Check(SubspaceDockSystem::FloatPanel(builder.MutableDockWorkspace(),"asset_browser",{150,180,600,280}),
          "asset shelf floats");
    Check(builder.Activate(ShipyardBuilderCommand::DccRevealAssetBrowser)&&Materialized(builder.Model()),
          "floating Assets can be raised without being destroyed");
    Check(builder.Activate(ShipyardBuilderCommand::DccToggleMaximizeViewport),"maximize viewport");
    Check(Has(ShipyardBuilderSystem::BuildControls(builder.Model(),1280,768),
              ShipyardBuilderCommand::DccRevealAssetBrowser),"recovery exists even in maximized viewport");
    Check(builder.Activate(ShipyardBuilderCommand::DccRevealAssetBrowser),"recover Assets from maximized viewport");
    Check(!builder.Model().dcc.maximizeViewport&&Materialized(builder.Model()),"recovery restores canvas overlays");

    Check(builder.Activate(ShipyardBuilderCommand::DccResetLayout),"reset layout works");
    Check(Materialized(builder.Model()),"reset restores the default Asset Browser");
    Check(SubspaceDockSystem::Validate(builder.Model().dockWorkspace),"recovery preserves valid dock ownership");
    std::cout<<"SHIPYARD ASSET VISIBILITY RECOVERY: "<<checks<<" assertions passed\n";
}
