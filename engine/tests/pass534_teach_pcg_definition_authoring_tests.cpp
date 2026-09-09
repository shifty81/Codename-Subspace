#include "assets/CanonicalAsset.h"
#include "content/ShipyardCanonicalAssetBridge.h"
#include "ship_editor/ShipyardBuilderSystem.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace fs=std::filesystem;
static int passed=0,failed=0;
#define CHECK(name,expr) do{if(expr){++passed;std::cout<<"[PASS] "<<name<<"\n";}else{++failed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static std::vector<ShipyardModuleRecord> Catalog(){
    ShipyardModuleRecord r;
    r.source={"teach_test_part",1.0f,1.5f,.5f};
    r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;
    r.builderCategory=ShipyardPartCategory::Hull;r.partRole=ShipyardPartRole::PrimaryHull;r.primaryHull=true;r.generatorEligible=true;
    r.placementRole="HULL_CORE";r.preferredMountFace="";r.mountFaceConfidence=0.0f;
    r.sockets={{"manual_mount","detail_mount",.25f,0,.25f,1,0,0,.05f,0,0,1,true}};
    return {r};
}
static ProceduralShipVisualRecipe Recipe(){ProceduralShipVisualRecipe r;VisualModulePlacement p;p.moduleId="teach_test_part";p.scaleX=p.scaleY=p.scaleZ=1.0f;r.modules.push_back(p);r.role="INDUSTRIAL";r.seed=0x534u;return r;}

int main(){
    const auto baseline=Catalog();
    ShipyardBuilderSystem builder;builder.Initialize(baseline,Recipe());builder.Activate(ShipyardBuilderCommand::SelectPlaced,0);
    CHECK("Pass534 Teach PCG inspector activates",builder.Activate(ShipyardBuilderCommand::InspectorAuthoring)&&builder.Model().inspectorTab==ShipyardInspectorTab::Authoring);
    CHECK("Pass534 semantic edit applies to reusable definition",builder.Activate(ShipyardBuilderCommand::NextSemantic)&&builder.Model().catalog[0].semantic!=baseline[0].semantic);
    CHECK("Pass534 semantic edit marks definition sidecar dirty",builder.Model().definitionOverridesDirty);
    const bool eligible=builder.Model().catalog[0].generatorEligible;
    CHECK("Pass534 PCG eligibility is authorable",builder.Activate(ShipyardBuilderCommand::ToggleGeneratorEligible)&&builder.Model().catalog[0].generatorEligible!=eligible);
    CHECK("Pass534 paired-placement intent is authorable",builder.Activate(ShipyardBuilderCommand::TogglePairedPlacement)&&builder.Model().catalog[0].pairedPlacement);
    CHECK("Pass534 preferred mount face is authorable",builder.Activate(ShipyardBuilderCommand::CyclePreferredMountFace)&&!builder.Model().catalog[0].preferredMountFace.empty());

    const fs::path path=fs::temp_directory_path()/"subspace_pass534_definition_overrides.subspace_definition_overrides";std::error_code ec;fs::remove(path,ec);
    std::string error;std::size_t changed=0;
    CHECK("Pass534 definition sidecar saves non-destructively",builder.SaveDefinitionOverrides(path.string(),&error,&changed)&&changed==1&&fs::exists(path));
    ShipyardBuilderSystem reload;reload.Initialize(baseline,Recipe());std::size_t applied=0;
    CHECK("Pass534 definition sidecar reloads",reload.LoadDefinitionOverrides(path.string(),&error,&applied)&&applied==1);
    CHECK("Pass534 reloaded semantic persists",reload.Model().catalog[0].semantic==builder.Model().catalog[0].semantic);
    CHECK("Pass534 reloaded generator eligibility persists",reload.Model().catalog[0].generatorEligible==builder.Model().catalog[0].generatorEligible);
    CHECK("Pass534 reloaded pairing intent persists",reload.Model().catalog[0].pairedPlacement==builder.Model().catalog[0].pairedPlacement);
    CHECK("Pass534 reloaded mount face persists",reload.Model().catalog[0].preferredMountFace==builder.Model().catalog[0].preferredMountFace);

    auto asset=ShipyardCanonicalAssetBridge::BuildAsset(builder.Model().catalog[0]);
    CHECK("Pass534 canonical asset carries edited semantic",!asset.nodes.empty()&&asset.nodes[0].extras["subspace.semantic"]==ShipyardModuleSystem::SemanticName(builder.Model().catalog[0].semantic));
    CHECK("Pass534 canonical asset carries generator eligibility",!asset.nodes.empty()&&asset.nodes[0].extras["subspace.generatorEligible"]==(builder.Model().catalog[0].generatorEligible?"true":"false"));
    CHECK("Pass534 canonical socket preserves manual override authority",!asset.sockets.empty()&&asset.sockets[0].authority==assets::SocketAuthority::ManualOverride);

    const auto controls=ShipyardBuilderSystem::BuildControls(builder.Model(),1440,900);bool sem=false,elig=false,pair=false,mount=false,save=false;
    for(const auto& c:controls){sem|=c.command==ShipyardBuilderCommand::NextSemantic;elig|=c.command==ShipyardBuilderCommand::ToggleGeneratorEligible;pair|=c.command==ShipyardBuilderCommand::TogglePairedPlacement;mount|=c.command==ShipyardBuilderCommand::CyclePreferredMountFace;save|=c.command==ShipyardBuilderCommand::SaveDefinitionOverrides;}
    CHECK("Pass534 Teach PCG inspector exposes semantic eligibility pairing mount and save",sem&&elig&&pair&&mount&&save);
    CHECK("Pass534 baseline definition reset works",builder.Activate(ShipyardBuilderCommand::ResetDefinitionOverride)&&builder.Model().catalog[0].semantic==baseline[0].semantic);
    fs::remove(path,ec);
    std::cout<<"Pass534 assertions: "<<passed<<" passed / "<<failed<<" failed\n";return failed?1:0;
}
