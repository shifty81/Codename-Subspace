#include "content/ShipyardModuleSystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "ships/ThrusterLayoutSystem.h"
#include "ships/ShipyardVisualAuthoritySystem.h"

#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace subspace;
static int testsPassed=0,testsFailed=0;
#define TEST(name,expr) do{if(expr){++testsPassed;std::cout<<"[PASS] "<<name<<"\n";}else{++testsFailed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static std::vector<VisualModuleSource> Fixture(){
    return {
        {"shipyard_a_hull_001_long_hull",2.6f,4.8f,1.2f},
        {"shipyard_a_hull_002_mid_hull",2.2f,3.4f,1.0f},
        {"shipyard_a_hull_003_aft_hull",2.5f,3.8f,1.1f},
        {"shipyard_a_command_001_cockpit",1.4f,2.0f,.8f},
        {"shipyard_a_command_002_bridge",1.7f,2.2f,1.1f},
        {"shipyard_a_propulsion_001_engine",.9f,1.7f,.8f},
        {"shipyard_a_propulsion_002_nozzle",.7f,1.2f,.7f},
        {"shipyard_a_propulsion_003_engine_housing",1.1f,1.9f,.95f},
        {"shipyard_a_propulsion_004_rcs_thruster",.32f,.55f,.30f},
        {"shipyard_a_hardpoint_001_turret_mount",.55f,.55f,.30f},
        {"shipyard_a_hardpoint_002_weapon_mount",.48f,.62f,.28f},
        {"shipyard_a_detail_001_greeble",.40f,.65f,.18f},
        {"shipyard_a_detail_002_vent_panel",.32f,.52f,.15f},
        {"shipyard_a_detail_003_sensor_array",.28f,.42f,.36f},
        {"shipyard_a_adapter_001_connector",1.0f,1.0f,.55f},
        {"shipyard_a_wing_001_fin",1.8f,1.3f,.22f}
    };
}

static void Pass428_ThrusterSupportIntegrity(){
    const auto layout=ThrusterLayoutSystem::StarterIndustrial();
    bool simulationLayoutStillPresent=!layout.sockets.empty();
    bool legacyVisualsForbidden=true;
    for(const auto& socket:layout.sockets){
        legacyVisualsForbidden&=ShipyardVisualAuthoritySystem::IsForbiddenLegacyVisualId(socket.moduleMesh);
    }
    TEST("Pass428 propulsion simulation sockets remain available without reviving legacy visual authority",simulationLayoutStillPresent);
    TEST("Pass428 legacy synthetic thruster meshes are forbidden from normal Shipyard rendering",legacyVisualsForbidden&&!ShipyardVisualAuthoritySystem::LegacyShipGeometryAllowed());
}

static void Pass428_NativeShipyardBuilder(){
    const auto sources=Fixture();
    const auto catalog=ShipyardModuleSystem::BuildCatalog(sources);
    const auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes(sources,0x428u);
    ShipyardBuilderSystem builder;builder.Initialize(catalog,recipes.front());
    TEST("Pass428 native Shipyard builder initializes from certified Greyoxide catalog",builder.IsInitialized()&&builder.Model().catalog.size()==catalog.size());
    TEST("Pass428 starter Shipyard recipe passes native structural socket validation",builder.Validate().valid);

    const auto transformControls=ShipyardBuilderSystem::BuildControls(builder.Model(),1280,900);
    bool hasAdd=false,hasReplace=false;
    for(const auto& c:transformControls){
        hasAdd|=c.command==ShipyardBuilderCommand::AddModule;
        hasReplace|=c.command==ShipyardBuilderCommand::ReplaceModule;
    }
    builder.Activate(ShipyardBuilderCommand::InspectorAssembly);
    const auto assemblyControls=ShipyardBuilderSystem::BuildControls(builder.Model(),1280,900);
    bool hasGenerate=false,hasApply=false;ShipyardBuilderControl target{};
    for(const auto& c:assemblyControls){
        hasGenerate|=c.command==ShipyardBuilderCommand::GenerateVariant;
        if(c.command==ShipyardBuilderCommand::Apply){hasApply=true;target=c;}
    }
    TEST("Pass428 builder exposes mouse controls for add replace generate and apply across workflow pages",hasAdd&&hasReplace&&hasGenerate&&hasApply);
    const auto hit=ShipyardBuilderSystem::HitTest(builder.Model(),1280,900,target.x+target.width*.5f,target.y+target.height*.5f);
    TEST("Pass428 drawing and mouse hit testing share the same Apply control rectangle on Assembly page",hit.command==ShipyardBuilderCommand::Apply);

    builder.Activate(ShipyardBuilderCommand::NextRole);
    const auto beforeSeed=builder.Model().seed;
    const bool generated=builder.Activate(ShipyardBuilderCommand::GenerateVariant);
    TEST("Pass428 in-game generator creates a new certified role variant",generated&&builder.Model().seed==beforeSeed+1&&!builder.Recipe().modules.empty());
    TEST("Pass428 generated variants remain structurally valid before apply",builder.Validate().valid);
    TEST("Pass428 Apply only raises a commit request for a valid design",builder.Activate(ShipyardBuilderCommand::Apply)&&builder.ConsumeApplyRequested());
}

int main(){
    Pass428_ThrusterSupportIntegrity();
    Pass428_NativeShipyardBuilder();
    std::cout<<"\nPass428 tests: "<<testsPassed<<" passed, "<<testsFailed<<" failed\n";
    return testsFailed?1:0;
}
