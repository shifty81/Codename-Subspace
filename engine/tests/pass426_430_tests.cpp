#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardPartTaxonomySystem.h"
#include "ship_editor/KitbashShipBuilderSystem.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace subspace;
static int passed=0,failed=0;
#define TEST(name,expr) do{if(expr){++passed;std::cout<<"[PASS] "<<name<<"\n";}else{++failed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static std::vector<VisualModuleSource> Fixture(){
    return {
        {"shipyard_a_hull_001_hullLong",2.6f,4.8f,1.2f},
        {"shipyard_a_hull_002_hullCompact",2.1f,3.0f,1.0f},
        {"shipyard_a_command_003_cockpit",1.4f,2.0f,.8f},
        {"shipyard_a_propulsion_004_engineBodyFish",1.1f,1.9f,.95f},
        {"shipyard_a_propulsion_005_engineStrutFoot",.5f,1.0f,.4f},
        {"shipyard_a_propulsion_006_engineFlap",.45f,.7f,.15f},
        {"shipyard_a_propulsion_007_engineTrumpet",.8f,1.4f,.7f},
        {"shipyard_a_hardpoint_008_hardpointBigGun",.6f,.8f,.35f},
        {"shipyard_a_component_009_cargoBox",.8f,1.0f,.7f},
        {"shipyard_a_component_010_tank",.7f,1.1f,.7f},
        {"shipyard_a_component_011_instrumentMast",.2f,.4f,.8f},
        {"shipyard_a_component_012_dualWindow",.6f,.3f,.15f},
        {"shipyard_a_component_013_angledBar",.25f,1.2f,.2f},
        {"shipyard_a_wing_014_wing",1.4f,1.2f,.2f},
        {"shipyard_a_detail_015_ventPanel",.4f,.5f,.12f},
        {"shipyard_a_adapter_016_hullJoined",.9f,1.0f,.55f}
    };
}

int main(){
    const auto catalog=ShipyardModuleSystem::BuildCatalog(Fixture());
    TEST("Pass426 every certified fixture part receives a non-empty builder taxonomy",catalog.size()==Fixture().size());
    std::set<ShipyardPartCategory> categories;for(const auto&r:catalog)categories.insert(r.builderCategory);
    TEST("Pass426 taxonomy exposes hull command structural adapter propulsion weapons utility sensors wings and surface shelves",
         categories.count(ShipyardPartCategory::Hull)&&categories.count(ShipyardPartCategory::Command)&&
         categories.count(ShipyardPartCategory::Structural)&&categories.count(ShipyardPartCategory::Adapter)&&
         categories.count(ShipyardPartCategory::Propulsion)&&categories.count(ShipyardPartCategory::Weapons)&&
         categories.count(ShipyardPartCategory::Utility)&&categories.count(ShipyardPartCategory::Sensors)&&
         categories.count(ShipyardPartCategory::Wings)&&categories.count(ShipyardPartCategory::Surface));

    auto roleOf=[&](const char* id){for(const auto&r:catalog)if(r.source.moduleId==id)return r.partRole;return ShipyardPartRole::Decoration;};
    TEST("Pass427 engine bodies are housings rather than terminal engines",roleOf("shipyard_a_propulsion_004_engineBodyFish")==ShipyardPartRole::EngineHousing);
    TEST("Pass427 engine struts are manual structural mounts",roleOf("shipyard_a_propulsion_005_engineStrutFoot")==ShipyardPartRole::EngineMount);
    TEST("Pass427 engine flaps are surface details",roleOf("shipyard_a_propulsion_006_engineFlap")==ShipyardPartRole::EngineDetail);
    TEST("Pass427 actual engine forms remain terminal main engines",roleOf("shipyard_a_propulsion_007_engineTrumpet")==ShipyardPartRole::MainEngine);

    const auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes(Fixture(),0x426430u);
    bool oneHull=!recipes.empty(),validGraphs=!recipes.empty();
    for(const auto&recipe:recipes){std::size_t hulls=0;for(const auto&m:recipe.modules){const auto*rec=KitbashShipBuilderSystem::FindRecord(catalog,m.moduleId);if(rec&&rec->primaryHull)++hulls;}oneHull&=hulls==1;std::string e;validGraphs&=ShipyardModuleSystem::ValidateAssemblyGraph(recipe,&e);}
    TEST("Pass428 procedural Shipyard generation uses exactly one primary authored hull",oneHull);
    TEST("Pass428 generated ships remain rooted certified graphs",validGraphs);

    auto builder=KitbashShipBuilderSystem::CreateStarter(catalog,"INDUSTRIAL");
    TEST("Pass429 in-game Shipyard palette exposes the full certified catalog",builder.palette.size()==catalog.size());
    std::string error;
    TEST("Pass429 manual builder rejects a second whole primary hull",
         !KitbashShipBuilderSystem::BeginDrag(builder,catalog,"shipyard_a_hull_001_hullLong",&error)&&!error.empty());
    error.clear();
    const bool drag=KitbashShipBuilderSystem::BeginDrag(builder,catalog,"shipyard_a_command_003_cockpit",&error);
    const auto preview=drag?KitbashShipBuilderSystem::PreviewDrop(builder,catalog,0,"forward"):KitbashShipBuilderPreview{};
    const bool commit=preview.valid&&KitbashShipBuilderSystem::CommitDrop(builder,catalog,preview,&error);
    TEST("Pass430 click-drag builder snaps a compatible authored part through certified sockets",drag&&preview.valid&&commit&&builder.blueprint.modules.size()==2);
    TEST("Pass430 committed manual blueprint validates with exactly one primary hull",KitbashShipBuilderSystem::ValidateBlueprint(builder,catalog,&error));

    std::cout<<"\n=== Pass426-430 Kitbash Shipyard Builder: "<<passed<<" passed, "<<failed<<" failed ===\n";
    return failed?1:0;
}
