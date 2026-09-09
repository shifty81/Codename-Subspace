#include "content/ShipyardModuleSystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardDesignExchangeSystem.h"
#include "ship_editor/ShipBlueprintLibrarySystem.h"
#include "ui/FrontendInteraction.h"

#include <iostream>
#include <vector>

using namespace subspace;
static int passed=0,failed=0;
#define TEST(n,e) do{if(e){++passed;std::cout<<"[PASS] "<<n<<"\n";}else{++failed;std::cout<<"[FAIL] "<<n<<"\n";}}while(0)

static std::vector<VisualModuleSource> Fixture(){return {
    {"shipyard_a_hull_001_hull_long",2.4f,4.8f,1.2f},
    {"shipyard_a_command_001_bridge_compact",1.2f,1.8f,.8f},
    {"shipyard_a_propulsion_001_engine5engine",.8f,1.5f,.7f},
    {"shipyard_a_hardpoint_001_turret_mount",.5f,.5f,.25f}
};}

int main(){
    const auto frontend=BuildFrontendControls(FrontendScreen::MainMenu,1600,900);
    bool hasShipyard=false;FrontendControl shipyard{};
    for(const auto& c:frontend)if(c.command==FrontendCommand::MainShipyard){hasShipyard=true;shipyard=c;break;}
    TEST("Pass468 main menu exposes a real Shipyard blueprint-design command",hasShipyard);
    TEST("Pass468 Shipyard menu rectangle is mouse hit-testable",hasShipyard&&HitTestFrontendControls(frontend,shipyard.bounds.x+20,shipyard.bounds.y+10)==FrontendCommand::MainShipyard);

    const auto catalog=ShipyardModuleSystem::BuildCatalog(Fixture());
    const auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes(Fixture(),468u);
    ShipyardBuilderSystem builder;builder.Initialize(catalog,recipes.front());builder.SetLiveApplyEnabled(false,true);
    builder.Activate(ShipyardBuilderCommand::InspectorAssembly);
    const auto controls=ShipyardBuilderSystem::BuildControls(builder.Model(),1280,900);
    bool hasSave=false,applyDisabled=false;
    for(const auto& c:controls){if(c.command==ShipyardBuilderCommand::SaveBlueprint)hasSave=c.enabled;if(c.command==ShipyardBuilderCommand::Apply)applyDisabled=!c.enabled;}
    TEST("Pass468 standalone builder exposes enabled SAVE BLUEPRINT",hasSave);
    TEST("Pass468 standalone builder disables live APPLY until docked",applyDisabled&&!builder.Activate(ShipyardBuilderCommand::Apply));
    TEST("Pass468 standalone builder raises save request even for reviewable design",builder.Activate(ShipyardBuilderCommand::SaveBlueprint)&&builder.ConsumeSaveRequested());
    builder.SetLiveApplyEnabled(true,false);
    TEST("Pass468 docked builder restores transactional APPLY authority",builder.Model().liveApplyEnabled&&builder.Validate().valid&&builder.Activate(ShipyardBuilderCommand::Apply)&&builder.ConsumeApplyRequested());

    ShipBlueprintDocument bp;bp.name="Pass468 Interchange";bp.author="TEST";bp.recipe=recipes.front();bp.blueprintId=ShipBlueprintLibrarySystem::CanonicalId(bp);
    const auto text=ShipyardDesignExchangeSystem::Serialize(bp,catalog,"test");
    TEST("Pass468 native export uses Blender subspace.shipyard_design v1 schema",text.find("\"schema\": \"subspace.shipyard_design\"")!=std::string::npos&&text.find("\"version\": 1")!=std::string::npos);
    std::size_t moduleRows=0,pos=0;while((pos=text.find("\"instanceId\"",pos))!=std::string::npos){++moduleRows;pos+=12;}
    TEST("Pass468 interchange preserves every recipe module",moduleRows==bp.recipe.modules.size());
    TEST("Pass468 interchange emits editable per-module transforms",text.find("\"transform\"")!=std::string::npos&&text.find("\"position\"")!=std::string::npos&&text.find("\"rotationEulerDeg\"")!=std::string::npos);

    std::cout<<"Pass468 assertions: "<<passed<<" passed / "<<failed<<" failed\n";return failed?1:0;
}
