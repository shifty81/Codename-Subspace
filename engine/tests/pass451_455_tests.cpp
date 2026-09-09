#include "content/ShipyardModuleSystem.h"
#include "ships/ThrusterLayoutSystem.h"

#include <cmath>
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
        {"shipyard_a_command_003_cockpit",1.4f,2.0f,.8f},
        {"shipyard_a_propulsion_004_engineBodyFish",1.1f,1.9f,.95f},
        {"shipyard_a_propulsion_007_engineTrumpet",.8f,1.4f,.7f},
        {"shipyard_a_propulsion_008_engineRect",.7f,1.2f,.65f},
        {"shipyard_a_propulsion_009_engineOnion",.9f,1.0f,.8f},
        {"shipyard_a_propulsion_010_engineMulti",.75f,1.25f,.72f},
        {"shipyard_a_hardpoint_011_hardpointBigGun",.6f,.8f,.35f}
    };
}

static const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>& c,const std::string& id){
    for(const auto&r:c)if(r.source.moduleId==id)return &r;return nullptr;
}

int main(){
    const auto sources=Fixture();
    const auto catalog=ShipyardModuleSystem::BuildCatalog(sources);
    const auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes(sources,0x451455u);
    TEST("Pass451 fixture produces at least one certified Shipyard recipe",!recipes.empty());
    if(recipes.empty()){std::cout<<"\n=== Pass451-455 Propulsion Binding: "<<passed<<" passed, "<<failed<<" failed ===\n";return 1;}

    const auto& recipe=recipes.front();
    const auto layout=ThrusterLayoutSystem::ForShipRecipe(recipe,catalog,"INDUSTRIAL");
    TEST("Pass451 recipe-bound propulsion retains complete six-axis planar authority",layout.IsCompleteSixAxisPlanarLayout());
    TEST("Pass451 recipe-bound propulsion retains valid plume directions",layout.HasClearPlumeDirections());

    bool integratedMain=false,externalKitbash=true,exhaustSeparated=true;
    std::set<std::string> externalMeshes;
    for(const auto&s:layout.sockets){
        if(s.role==ThrusterRole::Forward && !s.renderModule) integratedMain=true;
        if(s.externalMount){
            const auto* r=Find(catalog,s.moduleMesh);
            externalKitbash &= r && r->partRole==ShipyardPartRole::MainEngine;
            exhaustSeparated &= s.exhaustOffset.length()>0.01f;
            externalMeshes.insert(s.moduleMesh);
        }
    }
    TEST("Pass452 assembled main engines are reused instead of drawn twice as floating pods",integratedMain);
    TEST("Pass452 external attitude thrusters use certified kitbash propulsion modules",externalKitbash);
    TEST("Pass452 propulsion module origin and exhaust origin are distinct",exhaustSeparated);
    TEST("Pass453 one ship samples multiple authored propulsion shapes",externalMeshes.size()>=3);

    const auto* hull=Find(catalog,recipe.modules.front().moduleId);
    bool adaptiveEnvelope=hull!=nullptr;
    if(hull){
        const auto& p=recipe.modules.front();
        const float hw=hull->source.halfWidth*p.scaleX;
        const float hl=hull->source.halfLength*p.scaleY;
        for(const auto&s:layout.sockets)if(s.externalMount){
            adaptiveEnvelope &= std::fabs(s.localPosition.x-p.x)<=hw+1.15f;
            adaptiveEnvelope &= std::fabs(s.localPosition.y-p.y)<=hl+1.15f;
        }
    }
    TEST("Pass454 external pods are constrained to the selected primary-hull envelope",adaptiveEnvelope);

    const auto fallback=ThrusterLayoutSystem::StarterIndustrial();
    TEST("Pass455 legacy fallback remains available for non-Shipyard recipes",fallback.IsCompleteSixAxisPlanarLayout());

    std::cout<<"\n=== Pass451-455 Propulsion Binding: "<<passed<<" passed, "<<failed<<" failed ===\n";
    return failed?1:0;
}
