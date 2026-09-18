#include "ship_editor/ShipBlueprintLibrarySystem.h"
#include <filesystem>
#include <iostream>
using namespace subspace;
int main(){
    int checks=0,fail=0;
    auto check=[&](bool ok,const char* label){++checks;if(!ok){++fail;std::cerr<<"FAIL "<<label<<'\n';}};
    const auto test=std::filesystem::temp_directory_path()/"subspace_studio_roundtrip_test.subspace_ship";
    std::error_code ec;std::filesystem::remove(test,ec);
    ShipBlueprintDocument doc;
    doc.blueprintId="studio_test";doc.name="Studio test";doc.recipe.recipeId="test.recipe";
    doc.recipe.role="INDUSTRIAL";doc.recipe.seed=41;
    VisualModulePlacement module;module.moduleId="certified_test_hull";module.x=3;module.y=4;module.z=5;
    doc.recipe.modules.push_back(module);doc.tags={"DRAFT_REVIEWABLE"};
    std::string err;
    check(ShipBlueprintLibrarySystem::Save(doc,test.string(),&err),"canonical write");
    ShipBlueprintDocument loaded;
    check(ShipBlueprintLibrarySystem::Load(test.string(),loaded,&err),"canonical read");
    check(loaded.recipe.modules.size()==1,"module count persists");
    if(loaded.recipe.modules.size()==1){
        check(loaded.recipe.modules[0].moduleId==module.moduleId,"module ID persists");
        check(loaded.recipe.modules[0].x==3&&loaded.recipe.modules[0].z==5,"placement persists");
    }
    check(loaded.recipe.seed==41,"seed persists");
    check(!loaded.tags.empty()&&loaded.tags[0]=="DRAFT_REVIEWABLE","draft marker persists");
    std::filesystem::remove(test,ec);
    std::cout<<"Studio blueprint roundtrip: "<<checks-fail<<"/"<<checks<<" passed\n";
    return fail?1:0;
}
