#include "ship_editor/ShipyardDocumentStartupSystem.h"
#include <cstdlib>
#include <iostream>
using namespace subspace;
static int checks=0;
void Check(bool condition,const char* label){++checks;if(!condition){std::cerr<<"FAIL: "<<label<<'\n';std::exit(1);}}
int main(){
    ProceduralShipVisualRecipe game{};game.recipeId="playable";game.role="EXPLORER";
    game.modules.emplace_back();game.modules.back().moduleId="starter_hull";
    game.attachments.emplace_back();game.details.emplace_back();game.anchors.emplace_back();
    game.hardpoints.emplace_back();game.articulations.emplace_back();
    const auto blank=ShipyardDocumentStartupSystem::SelectInitialDocument(true,game);
    Check(blank.modules.empty(),"studio does not preload ship modules");
    Check(blank.attachments.empty()&&blank.details.empty()&&blank.anchors.empty(),"studio carries no hidden kitbash contents");
    Check(blank.hardpoints.empty()&&blank.articulations.empty(),"studio carries no functional parts");
    Check(blank.recipeId=="shipyard.untitled","studio has an unsaved document ID");
    Check(!blank.runtimePcgCertified&&blank.runtimeCertificationMessage=="EMPTY_AUTHORING_DOCUMENT","blank draft is not presented as certified gameplay ship");
    Check(blank.cockpitModuleIndex==-1,"blank has no dangling cockpit index");
    const auto runtime=ShipyardDocumentStartupSystem::SelectInitialDocument(false,game);
    Check(runtime.recipeId==game.recipeId&&runtime.modules.size()==1,"gameplay preserves its loaded ship");
    Check(runtime.attachments.size()==1&&runtime.articulations.size()==1,"gameplay retains full assembly");
    std::cout<<"G2 blank startup: "<<checks<<" assertions passed\n";
}
