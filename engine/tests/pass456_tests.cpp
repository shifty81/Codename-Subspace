#include "ships/ShipyardVisualAuthoritySystem.h"
#include "ships/ShipyardAuthoredShipSystem.h"
#include "ships/ShipyardDesignLanguageSystem.h"
#include "ship_editor/ShipyardRefitSystem.h"
#include "rendering/PlanetPresentationSystem.h"
#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardPartTaxonomySystem.h"
#include "ui/FrontendInteraction.h"
#include <cmath>
#include <iostream>
#include <vector>
using namespace subspace;
static int passed=0,failed=0;
#define TEST(n,e) do{if(e){++passed;std::cout<<"[PASS] "<<n<<"\n";}else{++failed;std::cout<<"[FAIL] "<<n<<"\n";}}while(0)
int main(){
    TEST("R5 normal runtime forbids pre-Shipyard synthetic ship geometry",!ShipyardVisualAuthoritySystem::LegacyShipGeometryAllowed());
    TEST("R5 old rectangle-thruster era engine/hull IDs are explicitly forbidden",ShipyardVisualAuthoritySystem::IsForbiddenLegacyVisualId("engine_main")&&ShipyardVisualAuthoritySystem::IsForbiddenLegacyVisualId("thruster_small")&&ShipyardVisualAuthoritySystem::IsForbiddenLegacyVisualId("hull_section"));
    const auto& defs=ShipyardAuthoredShipSystem::Definitions();
    TEST("R5 keeps exactly Scout/Battleship/Cruiser as authored universe ships",defs.size()==3&&defs[0].id=="SBS_SCOUT"&&defs[1].id=="SBS_BATTLESHIP"&&defs[2].id=="SBS_CRUISER");
    bool capturable=true,authoredOnly=true;for(const auto&d:defs){capturable&=d.capturable;authoredOnly&=ShipyardAuthoredShipSystem::BuildRecipe(d).sourceFamily=="SHIPYARD_STRIKES_BACK_AUTHORED";}
    TEST("R5 all three authored ships are capturable and remain outside procedural template family",capturable&&authoredOnly);
    TEST("R5 example ships are design-language ratios only",ShipyardDesignLanguageSystem::References().size()==3);
    const float horizontal=ShipyardDesignLanguageSystem::Score(12,32,10,true,true,true);const float vertical=ShipyardDesignLanguageSystem::Score(8,8,30,true,true,true);
    TEST("R5 design-language scoring strongly prefers horizontal ship silhouette",horizontal>=78.0f&&vertical<horizontal-20.0f);
    TEST("R5 atmosphere is a limb function rather than full-disc opacity",PlanetPresentationSystem::AtmosphereLimb(1.0f)<0.001f&&PlanetPresentationSystem::AtmosphereLimb(0.0f)>.95f);
    TEST("R5 continuous terminator ramps smoothly instead of cel bands",PlanetPresentationSystem::SmoothTerminator(-.2f)<PlanetPresentationSystem::SmoothTerminator(0.0f)&&PlanetPresentationSystem::SmoothTerminator(0.0f)<PlanetPresentationSystem::SmoothTerminator(.3f));

    ProceduralShipVisualRecipe original; original.modules.push_back({"hullA"}); original.modules.push_back({"engineA"});
    auto session=ShipyardRefitSystem::Begin(original,{{"engineB",1,1.0f,ShipyardRefitPartSource::Looted}});
    auto working=original;working.modules[1].moduleId="engineB";auto delta=ShipyardRefitSystem::Preview(session,working);
    TEST("R5 docked refit transaction consumes incoming looted/crafted module and returns removed module",delta.valid&&delta.incoming.size()==1&&delta.incoming[0]=="engineB"&&delta.outgoing.size()==1&&delta.outgoing[0]=="engineA");
    ShipyardRefitDelta committed;TEST("R5 valid docked refit commits transaction",ShipyardRefitSystem::Commit(session,working,&committed));
    auto unavailable=ShipyardRefitSystem::Begin(original,{});TEST("R5 refit fails closed when replacement part is not owned/stored/available",!ShipyardRefitSystem::Preview(unavailable,working).valid);

    std::vector<VisualModuleSource> src={{"shipyard_a_hull_001_shipyard_hull_001_hull_long",2,10,2},{"shipyard_a_command_001_shipyard_command_001_bridge_compact",1,2,1},{"shipyard_a_propulsion_001_shipyard_propulsion_001_engineBodyChin",1,2,1},{"shipyard_a_propulsion_002_shipyard_propulsion_002_engine5Engine",1,2,1},{"shipyard_a_propulsion_003_shipyard_propulsion_003_engineStrutLadder",1,2,1}};
    auto catalog=ShipyardModuleSystem::BuildCatalog(src);bool hull=false,strutNotDrive=false,housing=false;for(const auto&r:catalog){hull|=r.primaryHull;strutNotDrive|=r.semantic==ShipyardModuleSemantic::StructuralFrame&&!r.generatorEligible;housing|=r.semantic==ShipyardModuleSemantic::EngineHousing;}
    TEST("R5 canonical taxonomy populates primary-hull and reviewed engine-builder roles",hull&&strutNotDrive&&housing);

    const auto controls=BuildFrontendControls(FrontendScreen::MainMenu,1600,900);auto newSandbox=HitTestFrontendControls(controls,800,390);TEST("R5 main-menu rectangles remain mouse hit-testable",newSandbox!=FrontendCommand::None);
    std::cout<<"Pass456 assertions: "<<passed<<" passed / "<<failed<<" failed\n";return failed?1:0;
}
