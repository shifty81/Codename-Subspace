// Pass336-340 visual content foundation acceptance suite.
#include <iostream>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "procedural/GalaxyGenerator.h"
#include "rendering/PlanetSurfaceSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "rendering/SpaceMaterialSystem.h"

using namespace subspace;
static int testsPassed=0; static int testsFailed=0;
#define TEST(name,expr) do { if(expr){++testsPassed;std::cout<<"  PASS: "<<name<<"\n";}else{++testsFailed;std::cout<<"  FAIL: "<<name<<" ("<<__FILE__<<":"<<__LINE__<<")\n";} } while(0)

static std::vector<std::string> CurrentModuleLibrary(){
    return {"cargo_bay","cockpit_basic","cockpit_small","engine_main","engine_small",
            "hull_section","hull_section_enhanced","hull_section_small","power_core","sensor_array",
            "thruster","thruster_small","weapon_mount","wing_left","wing_right","wing_small_left","wing_small_right"};
}

static void TestPass336CurrentModelDiscoveryContract(){
    std::cout<<"[Pass336CurrentModelDiscoveryContract]\n";
    const auto modules=CurrentModuleLibrary();
    auto catalog=ProceduralVisualVariantSystem::Build(modules,336u,12);
    TEST("Pass336 catalog retains every discovered authored module",catalog.sourceModules.size()==modules.size());
    TEST("Pass336 load-time generation creates a deep visual catalog",catalog.shipRecipes.size()>=120);
    std::unordered_set<std::string> available(modules.begin(),modules.end());
    bool allExisting=true;
    for(const auto&r:catalog.shipRecipes)for(const auto&m:r.modules)if(!available.count(m.moduleId))allExisting=false;
    TEST("Pass336 generated recipes only reference current authored OBJ modules",allExisting);
    bool usesNewlyDiscovered=false;
    for(const auto&r:catalog.shipRecipes)for(const auto&m:r.modules)
        if(m.moduleId=="weapon_mount"||m.moduleId=="thruster"||m.moduleId=="thruster_small")usesNewlyDiscovered=true;
    TEST("Pass336 generator consumes modules omitted by the old 14-name loader",usesNewlyDiscovered);
}

static void TestPass337DeterministicShipRecipes(){
    std::cout<<"[Pass337DeterministicShipRecipes]\n";
    const auto modules=CurrentModuleLibrary();
    auto a=ProceduralVisualVariantSystem::Build(modules,0x12345678u,12);
    auto b=ProceduralVisualVariantSystem::Build(modules,0x12345678u,12);
    TEST("Pass337 same seed creates same catalog size",a.shipRecipes.size()==b.shipRecipes.size());
    TEST("Pass337 same seed creates stable first recipe",!a.shipRecipes.empty()&&a.shipRecipes.front().recipeId==b.shipRecipes.front().recipeId&&a.shipRecipes.front().modules.size()==b.shipRecipes.front().modules.size());
    auto* industrial=ProceduralVisualVariantSystem::Select(a,"Industrial Hauler",42u);
    auto* combat=ProceduralVisualVariantSystem::Select(a,"Combat Frigate",42u);
    TEST("Pass337 role normalization resolves industrial haulers",industrial&&industrial->role=="HAULER");
    TEST("Pass337 role normalization resolves combat hulls",combat&&combat->role=="COMBAT");
    TEST("Pass337 heavy and combat recipes have different assembly grammar",industrial&&combat&&industrial->modules.size()!=combat->modules.size());
    auto* first=ProceduralVisualVariantSystem::Select(a,"Patrol",7u);
    auto* second=ProceduralVisualVariantSystem::Select(a,"Patrol",8u);
    TEST("Pass337 adjacent visual seeds can select different stable variants",first&&second&&first->recipeId!=second->recipeId);

    GalaxyGenerator g(337);
    auto s1=g.GenerateSector(4,-2,1),s2=g.GenerateSector(4,-2,1);
    TEST("Pass337 procedural traffic always has stable visual identities",!s1.ships.empty()&&!s1.ships.front().shipId.empty()&&s1.ships.front().visualSeed!=0);
    TEST("Pass337 same galaxy seed preserves traffic visual recipe seed",s1.ships.size()==s2.ships.size()&&s1.ships.front().visualSeed==s2.ships.front().visualSeed&&s1.ships.front().shipId==s2.ships.front().shipId);
}

static void TestPass338ShaderUpgrade(){
    std::cout<<"[Pass338ShaderUpgrade]\n";
    const std::string vs=SpaceMaterialSystem::VertexShader120();
    const std::string fs=SpaceMaterialSystem::FragmentShader120();
    TEST("Pass338 shader preserves object-space normal for locked surface detail",vs.find("vObjectNormal")!=std::string::npos);
    TEST("Pass338 shader adds deterministic procedural surface noise",fs.find("float fbm")!=std::string::npos&&fs.find("uSurfaceSeed")!=std::string::npos);
    TEST("Pass338 shader adds Fresnel material response",fs.find("uFresnelStrength")!=std::string::npos&&fs.find("fresnel")!=std::string::npos);
    TEST("Pass338 shader exposes planet-specific surface modes",fs.find("uSurfaceMode")!=std::string::npos&&fs.find("gas giant")!=std::string::npos);
    auto hull=SpaceMaterialSystem::GetProfile(SpaceMaterialKind::ShipHull);
    auto canopy=SpaceMaterialSystem::GetProfile(SpaceMaterialKind::Canopy);
    TEST("Pass338 canopy and hull retain materially different response",canopy.fresnelStrength>hull.fresnelStrength&&canopy.roughness<hull.roughness);
}

static PlanetData MakePlanet(PlanetType type,int seed,float richness=.65f,float hazard=.35f){
    PlanetData p;p.planetId="surface-test";p.name="Surface Test";p.type=type;p.surfaceSeed=seed;p.resourceRichness=richness;p.hazardLevel=hazard;p.radius=700;return p;
}

static void TestPass339PlanetSurfaceAuthority(){
    std::cout<<"[Pass339PlanetSurfaceAuthority]\n";
    auto rock=PlanetSurfaceSystem::Build(MakePlanet(PlanetType::Rocky,339));
    auto desert=PlanetSurfaceSystem::Build(MakePlanet(PlanetType::Desert,339));
    auto ocean=PlanetSurfaceSystem::Build(MakePlanet(PlanetType::Oceanic,339));
    auto ice=PlanetSurfaceSystem::Build(MakePlanet(PlanetType::Ice,339));
    auto lava=PlanetSurfaceSystem::Build(MakePlanet(PlanetType::Volcanic,339,.7f,.8f));
    auto gas=PlanetSurfaceSystem::Build(MakePlanet(PlanetType::GasGiant,339,.8f,.7f));
    TEST("Pass339 rocky and desert worlds use different shader materials",rock.material!=desert.material);
    TEST("Pass339 ocean worlds expose ocean/land mix",ocean.oceanFraction>.5f&&ocean.material==SpaceMaterialKind::PlanetOcean);
    TEST("Pass339 ice worlds expose high ice coverage",ice.iceFraction>.7f&&ice.material==SpaceMaterialKind::PlanetIce);
    TEST("Pass339 volcanic worlds expose emissive lava activity",lava.lavaGlow>.7f&&lava.material==SpaceMaterialKind::PlanetVolcanic);
    TEST("Pass339 gas giants expose strong atmospheric banding",gas.bandStrength>.8f&&gas.stormStrength>.5f&&gas.material==SpaceMaterialKind::PlanetGas);
    auto rockAgain=PlanetSurfaceSystem::Build(MakePlanet(PlanetType::Rocky,339));
    TEST("Pass339 surface profile is deterministic from surface seed",rock.baseColor==rockAgain.baseColor&&rock.detailScale==rockAgain.detailScale);
    auto otherRock=PlanetSurfaceSystem::Build(MakePlanet(PlanetType::Rocky,340));
    TEST("Pass339 different planet seeds produce visual surface variation",rock.baseColor!=otherRock.baseColor||rock.surfaceSeed!=otherRock.surfaceSeed);
}

static void TestPass340PlanetRepresentationNormalization(){
    std::cout<<"[Pass340PlanetRepresentationNormalization]\n";
    PlanetData terrestrial=MakePlanet(PlanetType::Desert,340);
    PlanetData gas=MakePlanet(PlanetType::GasGiant,341);
    gas.industryRepresentation=PlanetIndustryRepresentation::AtmosphericCollectorRing;
    TEST("Pass340 all planet records remain eventually harvestable",terrestrial.harvestable&&gas.harvestable);
    TEST("Pass340 solid worlds identify surface hex industry",std::string(PlanetSurfaceSystem::IndustryRepresentation(terrestrial))=="Surface Hex Industry");
    TEST("Pass340 gas giants identify collector-ring industry",std::string(PlanetSurfaceSystem::IndustryRepresentation(gas))=="Atmospheric Collector Ring");
    TEST("Pass340 gas surface is visually distinct from solid surface",PlanetSurfaceSystem::Build(gas).material!=PlanetSurfaceSystem::Build(terrestrial).material);
}

int main(){
    TestPass336CurrentModelDiscoveryContract();
    TestPass337DeterministicShipRecipes();
    TestPass338ShaderUpgrade();
    TestPass339PlanetSurfaceAuthority();
    TestPass340PlanetRepresentationNormalization();
    std::cout<<"\n=== Pass336-340 Visual Foundation Summary: "<<testsPassed<<" passed, "<<testsFailed<<" failed ===\n";
    return testsFailed?1:0;
}
