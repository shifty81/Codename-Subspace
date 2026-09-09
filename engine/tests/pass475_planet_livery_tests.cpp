#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipBlueprintLibrarySystem.h"
#include "ship_editor/ShipyardDesignExchangeSystem.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

using namespace subspace;
static int failures=0;
static void Check(bool ok,const char* name){std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}

int main(){
    std::cout<<"[Pass475PlanetLivery]\n";

    ShipyardModuleRecord record;
    record.source.moduleId="shipyard_a_hull_test";
    record.moduleClass=ShipyardModuleClass::Hull;
    record.partRole=ShipyardPartRole::PrimaryHull;
    record.generatorEligible=true;
    VisualModulePlacement placement;placement.moduleId=record.source.moduleId;
    ProceduralShipVisualRecipe recipe;recipe.recipeId="pass475_fixture";recipe.role="INDUSTRIAL";recipe.modules.push_back(placement);

    ShipyardBuilderSystem builder;builder.Initialize({record},recipe);
    const auto before=builder.Appearance().primary;
    Check(builder.Activate(ShipyardBuilderCommand::NextLiveryPreset),"builder cycles a real paint/livery preset");
    const auto after=builder.Appearance().primary;
    Check(std::fabs(before.r-after.r)>.01f||std::fabs(before.g-after.g)>.01f||std::fabs(before.b-after.b)>.01f,"paint preset changes authoritative primary color");
    Check(builder.Activate(ShipyardBuilderCommand::AddDecal),"builder adds decal to selected assembled module");
    Check(builder.Appearance().decals.size()==1&&builder.Appearance().decals.front().moduleIndex==0,"decal stores selected module identity");
    Check(!builder.Appearance().decals.front().decalAsset.empty(),"decal stores an original Subspace decal preset id");

    ShipBlueprintDocument doc;doc.name="Pass475 Test";doc.recipe=builder.Recipe();doc.appearance=builder.Appearance();
    const auto tmp=std::filesystem::temp_directory_path()/"subspace_pass475_livery.subspace_ship";
    std::string error;Check(ShipBlueprintLibrarySystem::Save(doc,tmp.string(),&error),"native blueprint saves paint/decal state");
    ShipBlueprintDocument loaded;Check(ShipBlueprintLibrarySystem::Load(tmp.string(),loaded,&error),"native blueprint reloads paint/decal state");
    Check(std::fabs(loaded.appearance.primary.r-doc.appearance.primary.r)<.0001f,"primary paint round-trips");
    Check(loaded.appearance.decals.size()==1&&loaded.appearance.decals.front().moduleIndex==0,"decal layer round-trips");
    std::error_code ec;std::filesystem::remove(tmp,ec);

    const auto json=ShipyardDesignExchangeSystem::Serialize(doc,{record},"Pass475");
    Check(json.find("\"decals\"")!=std::string::npos,"Blender/design exchange exposes decal array");
    Check(json.find(doc.appearance.decals.front().decalAsset)!=std::string::npos,"design exchange preserves decal asset id");
    Check(ShipBlueprintLibrarySystem::CanonicalId(doc)!=ShipBlueprintLibrarySystem::CanonicalId(ShipBlueprintDocument{}),"blueprint identity incorporates authored appearance/design state");

    Check(builder.Activate(ShipyardBuilderCommand::RemoveDecal),"builder removes selected-module decal");
    Check(builder.Appearance().decals.empty(),"decal removal updates authoritative appearance state");

    std::cout<<"Pass475 assertions: "<<(failures?"FAIL":"PASS")<<"\n";
    return failures?1:0;
}
