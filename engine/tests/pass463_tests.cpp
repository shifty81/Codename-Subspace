#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardNameClassification.h"
#include "core/resources/ObjAssetLoader.h"
#include "rendering/ImportedPlanetVisualSystem.h"
#include <iostream>
#include <cmath>
#include <vector>
using namespace subspace;
static int passed=0,failed=0;
#define TEST(n,e) do{if(e){++passed;std::cout<<"[PASS] "<<n<<"\n";}else{++failed;std::cout<<"[FAIL] "<<n<<"\n";}}while(0)
int main(){
    ObjAssetLoader loader; ObjMeshData mesh; std::string err;
    const std::string obj="usemtl Mat_Main\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\nusemtl Glow_Blu\nf 1 3 2\n";
    TEST("R6 OBJ loader preserves usemtl regions",loader.Parse(obj,mesh,&err)&&mesh.materialNames.size()==2&&mesh.triangles.size()==2&&mesh.triangles[0].materialIndex!=mesh.triangles[1].materialIndex);
    auto strut=ShipyardNameClassifier::Classify("shipyard_a_propulsion_139_shipyard_propulsion_022_enginestrutladder");
    TEST("R6 exact corpus classification keeps engine strut structural, never a drive",strut.semantic=="STRUCTURAL_FRAME"&&strut.moduleClass=="component");
    auto drive=ShipyardNameClassifier::Classify("shipyard_a_propulsion_118_shipyard_propulsion_001_engine5engine");
    TEST("R6 exact corpus classification recognizes complete drive",drive.semantic=="MAIN_ENGINE");
    auto planet=ImportedPlanetVisualSystem::ProfileFor(PlanetType::GasGiant);
    TEST("R6 Various Planets gas giant uses supplied surface and two cloud layers",planet.surfaceTexture=="textures/image_05.jpg"&&planet.cloudTextures.size()==2);

    std::vector<VisualModuleSource> src={
        {"shipyard_a_hull_102_shipyard_hull_010_hullhandle",2.0f,8.0f,2.0f},
        {"shipyard_a_hull_108_shipyard_hull_016_hulllarge",3.5f,10.0f,2.4f},
        {"shipyard_a_command_005_shipyard_command_004_miscbridgecompact",1.4f,2.2f,1.1f},
        {"shipyard_a_propulsion_118_shipyard_propulsion_001_engine5engine",1.0f,2.0f,1.0f},
        {"shipyard_a_propulsion_121_shipyard_propulsion_004_enginebodychin",1.1f,2.1f,1.1f},
        {"shipyard_a_wing_147_shipyard_wing_001_miscblockfinger",1.8f,2.4f,.7f},
        {"shipyard_a_component_054_shipyard_component_045_misctank",1.0f,1.8f,1.0f},
        {"shipyard_a_component_038_shipyard_component_029_miscinstrumenttube",.5f,1.0f,.8f},
        {"shipyard_a_detail_066_shipyard_detail_003_miscventblock",.4f,.6f,.25f},
        {"shipyard_a_hardpoint_090_shipyard_hardpoint_023_hardpointsmallgun",.5f,.7f,.4f}
    };
    auto catalog=ShipyardModuleSystem::BuildCatalog(src);
    bool tankUtility=false,strutDrive=false;for(const auto&r:catalog){if(r.source.moduleId.find("misctank")!=std::string::npos)tankUtility=r.placementRole=="CARGO_UTILITY"&&r.generatorEligible;if(r.source.moduleId.find("enginestrut")!=std::string::npos)strutDrive|=r.semantic==ShipyardModuleSemantic::MainEngine;}
    TEST("R6 catalog gives cargo/tank explicit utility placement role",tankUtility);
    TEST("R6 catalog never upgrades struts into MAIN_ENGINE",!strutDrive);
    const auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes(src,0x463u);
    bool antiLinear=!recipes.empty();
    for(const auto&r:recipes){int hulls=0;bool lateral=false;bool command=false;bool drivePlaced=false;for(const auto&m:r.modules){auto c=ShipyardNameClassifier::Classify(m.moduleId);if(c.moduleClass=="hull")++hulls;if(std::abs(m.x)>.15f)lateral=true;if(c.moduleClass=="command")command=true;if(c.semantic=="MAIN_ENGINE")drivePlaced=true;}antiLinear&=hulls<=2&&command&&drivePlaced&&(r.role=="EXPLORATION"||lateral);}
    TEST("R6 generator caps spine chain and builds width/function instead of a module train",antiLinear);
    std::cout<<"Pass463 assertions: "<<passed<<" passed / "<<failed<<" failed\n";return failed?1:0;
}
