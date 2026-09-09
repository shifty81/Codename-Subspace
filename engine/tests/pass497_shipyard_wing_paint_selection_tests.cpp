#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardPartTaxonomySystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;

namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}

const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>& c,const std::string& needle){
    for(const auto& r:c)if(r.source.moduleId.find(needle)!=std::string::npos)return &r;
    return nullptr;
}
const ShipyardAssemblySocket* Socket(const ShipyardModuleRecord& r,const char* name){
    for(const auto& s:r.sockets)if(s.name==name)return &s;return nullptr;
}

VisualModuleSource Source(std::string id,float w=2,float l=4,float h=1){
    VisualModuleSource s;s.moduleId=std::move(id);s.halfWidth=w;s.halfLength=l;s.halfHeight=h;return s;
}
}

int main(){
    std::vector<VisualModuleSource> sources={
        Source("shipyard_a_hull_100_shipyard_hull_001_hullcompact",3.0f,5.5f,1.5f),
        Source("shipyard_a_command_005_shipyard_command_004_miscbridgecompact",1.4f,1.8f,1.2f),
        Source("shipyard_a_propulsion_120_shipyard_propulsion_001_enginecubeengine",1.2f,2.0f,1.0f),
        Source("shipyard_a_wing_149_shipyard_wing_003_miscfinhanger",1.25f,3.55f,3.12f),
        Source("shipyard_a_wing_150_shipyard_wing_004_miscfinrunner",0.4f,3.6f,0.8f),
        Source("shipyard_a_wing_156_shipyard_wing_010_miscwing2",2.0f,3.0f,0.55f)
    };
    auto catalog=ShipyardModuleSystem::BuildCatalog(sources);
    const auto* hull=Find(catalog,"hullcompact");
    const auto* command=Find(catalog,"miscbridgecompact");
    const auto* engine=Find(catalog,"enginecubeengine");
    const auto* badNamedWing=Find(catalog,"miscfinhanger");
    const auto* fin=Find(catalog,"miscfinrunner");
    const auto* wing=Find(catalog,"miscwing2");
    Check(hull&&command&&engine&&badNamedWing&&fin&&wing,"fixture catalog resolves all required module records");
    if(!hull||!command||!engine||!badNamedWing||!fin||!wing)return 1;

    Check(badNamedWing->partRole==ShipyardPartRole::Wing,"user-certified miscFinHanger resolves as Wing rather than Fin");
    Check(!badNamedWing->generatorEligible,"uncertified miscFinHanger root is quarantined from procedural generation");
    Check(badNamedWing->preferredMountFace=="USER_ROOT_REVIEW","miscFinHanger records explicit root-review authority");
    Check(fin->partRole==ShipyardPartRole::Fin,"other fin-named parts remain fins until individually certified");

    VisualModulePlacement root;root.moduleId=hull->source.moduleId;root.scaleX=root.scaleY=root.scaleZ=1.0f;
    ProceduralShipVisualRecipe recipe;recipe.recipeId="pass497_fixture";recipe.role="INDUSTRIAL";recipe.modules.push_back(root);
    auto add=[&](const ShipyardModuleRecord& child,const char* parentSocketName){
        const auto* ps=Socket(*hull,parentSocketName);const auto* cs=Socket(child,"mount");
        if(!ps||!cs)return std::size_t(-1);
        using AttachFn=VisualModulePlacement (*)(const VisualModulePlacement&,const ShipyardAssemblySocket&,const ShipyardModuleRecord&,const ShipyardAssemblySocket&,float);
        AttachFn attach=&ShipyardModuleSystem::BuildAttachmentPlacement;
        auto placed=attach(root,*ps,child,*cs,1.0f);
        const auto index=recipe.modules.size();recipe.modules.push_back(placed);recipe.attachments.push_back({0,index,parentSocketName,"mount",0,true});return index;
    };
    const auto commandIndex=add(*command,"dorsal_forward");
    const auto engineIndex=add(*engine,"engine_port");
    Check(commandIndex!=std::size_t(-1)&&engineIndex!=std::size_t(-1),"command and drive attach through certified sockets");
    auto baseline=ShipyardModuleSystem::ValidateAssemblyGraph(catalog,recipe);
    Check(baseline.valid,"minimal hull-command-drive recipe remains valid");

    const auto wingIndex=add(*wing,"port_forward");
    Check(wingIndex!=std::size_t(-1),"ordinary wing attaches to lateral hull socket");
    if(wingIndex!=std::size_t(-1)){
        // Deliberately stand the wing on edge while preserving its attachment
        // record. Pass497 must reject the old false-positive PASS state.
        recipe.modules[wingIndex].pitchDegrees=90.0f;
        const auto invalid=ShipyardModuleSystem::ValidateAssemblyGraph(catalog,recipe);
        bool sawVertical=false;for(const auto&e:invalid.errors)if(e.find("Wing mounted vertically")!=std::string::npos)sawVertical=true;
        Check(!invalid.valid&&sawVertical,"vertical ordinary wing fails assembly certification");
    }

    // Selection authority: choosing a placed wing must synchronize the left
    // browser/class information to that same instance instead of displaying a
    // stale HULL_BOW while the viewport/right row highlight a wing.
    ShipyardBuilderSystem builder;builder.Initialize(catalog,recipe);
    Check(builder.Activate(ShipyardBuilderCommand::SelectPlaced,static_cast<int>(wingIndex)),"placed wing can be selected");
    Check(builder.Model().selectedClass==ShipyardModuleClass::Wing,"placed selection synchronizes browser class to Wing");
    const auto* selectedCatalog=builder.SelectedCatalogModule();
    Check(selectedCatalog&&selectedCatalog->source.moduleId==recipe.modules[wingIndex].moduleId,"placed selection synchronizes browser module identity");

    std::cout<<"Pass497 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
