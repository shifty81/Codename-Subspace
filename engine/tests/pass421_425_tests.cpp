#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardGeometryAnalysisSystem.h"
#include "core/resources/ObjAssetLoader.h"
#include "content/ShipyardCertificationSystem.h"
#include "content/ThirdPartyAssetIntakeSystem.h"
#include "developer/provenance/AssetProvenanceManifest.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace subspace;
static int testsPassed=0,testsFailed=0;
#define TEST(name,expr) do{if(expr){++testsPassed;std::cout<<"[PASS] "<<name<<"\n";}else{++testsFailed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static std::vector<VisualModuleSource> Fixture(){
    return {
        {"shipyard_a_hull_001_long_hull",2.6f,4.8f,1.2f},
        {"shipyard_a_hull_002_mid_hull",2.2f,3.4f,1.0f},
        {"shipyard_a_hull_003_aft_hull",2.5f,3.8f,1.1f},
        {"shipyard_a_command_001_cockpit",1.4f,2.0f,.8f},
        {"shipyard_a_command_002_bridge",1.7f,2.2f,1.1f},
        {"shipyard_a_propulsion_001_engine",.9f,1.7f,.8f},
        {"shipyard_a_propulsion_002_nozzle",.7f,1.2f,.7f},
        {"shipyard_a_propulsion_003_engine_housing",1.1f,1.9f,.95f},
        {"shipyard_a_propulsion_004_rcs_thruster",.32f,.55f,.30f},
        {"shipyard_a_hardpoint_001_turret_mount",.55f,.55f,.30f},
        {"shipyard_a_hardpoint_002_weapon_mount",.48f,.62f,.28f},
        {"shipyard_a_detail_001_greeble",.40f,.65f,.18f},
        {"shipyard_a_detail_002_vent_panel",.32f,.52f,.15f},
        {"shipyard_a_detail_003_sensor_array",.28f,.42f,.36f},
        {"shipyard_a_adapter_001_connector",1.0f,1.0f,.55f},
        {"shipyard_a_wing_001_fin",1.8f,1.3f,.22f}
    };
}

static void Pass421_GovernedIntake(){
    ThirdPartyAssetDescriptor d;
    d.assetId="Greyoxide Shipyard v0.7";d.title="Shipyard v0.7";d.author="Greyoxide";
    d.sourceUrl="https://opengameart.org/content/shipyard-v07-extracted";d.licenseId="CC0-1.0";
    d.licenseUrl="https://creativecommons.org/publicdomain/zero/1.0/";
    d.attributionText="Greyoxide - Shipyard v0.7 - CC0";d.checksumSha256=std::string(64,'a');
    d.upstreamVersion="0.7";d.sourcePath="content/third_party/greyoxide_shipyard_v07/source";
    AssetProvenanceManifest manifest;ThirdPartyAssetIntakeSystem intake;const auto result=intake.ValidateAndRegister(d,manifest);
    TEST("Pass421 Shipyard CC0 source satisfies governed third-party intake",result.accepted&&result.errors.empty());
    TEST("Pass421 upstream source and project-owned derivatives remain separated",result.canonicalSourcePath.find("content/third_party/")==0&&result.derivedPath.find("content/derived/")==0);
    const auto* record=manifest.Find("greyoxide_shipyard_v07");
    TEST("Pass421 provenance retains author license and source URL",record&&record->author=="Greyoxide"&&record->license=="CC0-1.0"&&!record->sourceUrl.empty());
}

static void Pass422_ModuleCatalog(){
    auto fixture=Fixture();fixture.push_back({"native_hull",1,1,1});fixture.push_back({"shipyard_hull_uncertified",1,1,1});
    const auto catalog=ShipyardModuleSystem::BuildCatalog(fixture);
    TEST("Pass422 runtime catalog contains only Grade-A certified Shipyard authored objects",catalog.size()==Fixture().size());
    std::set<ShipyardModuleClass> kinds;std::set<ShipyardModuleSemantic> semantics;bool aspect=true,sockets=true;
    for(const auto&r:catalog){kinds.insert(r.moduleClass);semantics.insert(r.semantic);aspect&=r.preserveAspectRatio;sockets&=!r.sockets.empty();}
    TEST("Pass422 classifier recognizes hull command propulsion hardpoint detail adapter and wing",kinds.count(ShipyardModuleClass::Hull)&&kinds.count(ShipyardModuleClass::Command)&&kinds.count(ShipyardModuleClass::Propulsion)&&kinds.count(ShipyardModuleClass::Hardpoint)&&kinds.count(ShipyardModuleClass::Detail)&&kinds.count(ShipyardModuleClass::Adapter)&&kinds.count(ShipyardModuleClass::Wing));
    TEST("Pass422 semantic classifier distinguishes cockpit bridge aft hull engine housing engine nozzle RCS and sensor",semantics.count(ShipyardModuleSemantic::CommandCockpit)&&semantics.count(ShipyardModuleSemantic::CommandBridge)&&semantics.count(ShipyardModuleSemantic::HullAft)&&semantics.count(ShipyardModuleSemantic::EngineHousing)&&semantics.count(ShipyardModuleSemantic::EngineNozzle)&&semantics.count(ShipyardModuleSemantic::RcsThruster)&&semantics.count(ShipyardModuleSemantic::Sensor));
    TEST("Pass422 certified Shipyard records preserve authored aspect ratio and expose typed sockets",aspect&&sockets);
    TEST("Pass422 raw Shipyard IDs remain identifiable but are not runtime-certified",!ShipyardModuleSystem::IsShipyardModule("hull_section")&&ShipyardModuleSystem::IsShipyardModule("shipyard_hull_001")&&!ShipyardModuleSystem::IsCertifiedShipyardModule("shipyard_hull_001")&&ShipyardModuleSystem::IsCertifiedShipyardModule("shipyard_a_hull_001"));
}

static void Pass423_AssemblyRules(){
    const VisualModuleSource hull{"shipyard_a_hull_999_aft_hull",2.5f,4.0f,1.2f};
    const VisualModuleSource engine{"shipyard_a_propulsion_999_engine",.9f,1.5f,.8f};
    const auto hullSockets=ShipyardModuleSystem::BuildSockets(hull,ShipyardModuleSemantic::HullAft);
    const auto engineSockets=ShipyardModuleSystem::BuildSockets(engine,ShipyardModuleSemantic::MainEngine);
    bool hasPort=false,hasStarboard=false,hasMount=false;
    for(const auto&s:hullSockets){hasPort|=s.name=="engine_port"&&s.type=="engine_cavity";hasStarboard|=s.name=="engine_starboard"&&s.type=="engine_cavity";}
    for(const auto&s:engineSockets)hasMount|=s.name=="mount"&&s.type=="engine_mount";
    TEST("Pass423 aft hull rules expose paired inset engine-cavity sockets",hasPort&&hasStarboard);
    TEST("Pass423 engine rules expose an engine-mount socket that mates to hull cavities",hasMount&&ShipyardModuleSystem::CanMate("engine_cavity","engine_mount"));
    TEST("Pass423 hull and command socket grammar permits forward/aft structural mating",ShipyardModuleSystem::CanMate("hull_forward","hull_aft")&&ShipyardModuleSystem::CanMate("hull_aft","hull_forward"));
    TEST("Pass423 role grammar keeps cockpit/sensor specialization without forbidding shared structural hulls",ShipyardModuleSystem::RoleSuitable(ShipyardModuleSemantic::HullMid,"HAULER")&&ShipyardModuleSystem::RoleSuitable(ShipyardModuleSemantic::CommandCockpit,"COMBAT")&&ShipyardModuleSystem::RoleSuitable(ShipyardModuleSemantic::Sensor,"EXPLORATION")&&!ShipyardModuleSystem::RoleSuitable(ShipyardModuleSemantic::CommandCockpit,"HAULER"));
}

static void Pass424_ShowcaseAssembly(){
    const auto fixture=Fixture();
    const auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes(fixture,0x424u);
    TEST("Pass424 Shipyard assembler produces ten deterministic showcase ships",recipes.size()==10);
    std::set<std::string> roles;bool family=true,uniform=true,hasDrive=true,hasHardpoint=true,accepted=true;
    std::map<std::string,VisualModuleSource> metric;for(const auto&s:fixture)metric[s.moduleId]=s;
    for(const auto&r:recipes){
        roles.insert(r.role);family&=r.sourceFamily=="SHIPYARD_V07_CC0"&&r.manufacturerFamily=="GREYOXIDE_SHIPYARD";
        hasHardpoint&=!r.hardpoints.empty();accepted&=r.acceptedByArtDirector&&r.qualityScore>=78.0f;
        bool drive=false;
        float hullRear=1e9f;
        for(const auto&m:r.modules){
            family&=ShipyardModuleSystem::IsCertifiedShipyardModule(m.moduleId);
            if(std::fabs(m.scaleX-m.scaleY)>0.0001f||std::fabs(m.scaleY-m.scaleZ)>0.0001f)uniform=false;
            auto it=metric.find(m.moduleId);if(it==metric.end())continue;
            if(ShipyardModuleSystem::Classify(it->second)==ShipyardModuleClass::Hull)hullRear=std::min(hullRear,m.y-it->second.halfLength*m.scaleY);
        }
        for(const auto&a:r.anchors){
            if(a.id.find("DRIVE_")==0) drive=true;
        }
        // R4 can seat a main engine through an authored engine housing that
        // legitimately extends beyond the primary hull's raw AABB. Certified
        // graph contact, not engine-center containment, is the authority.
        hasDrive&=drive&&ShipyardModuleSystem::ValidateAssemblyGraph(r,nullptr)&&r.attachments.size()+1==r.modules.size();
    }
    TEST("Pass424 showcase covers industrial combat mining hauler and exploration roles",roles.size()==5&&roles.count("INDUSTRIAL")&&roles.count("COMBAT")&&roles.count("MINING")&&roles.count("HAULER")&&roles.count("EXPLORATION"));
    TEST("Pass424 showcase recipes use only certified preserved Shipyard modules",family);
    TEST("Pass424 structural assembly uses uniform scaling rather than distorting authored meshes",uniform);
    TEST("Pass424 drive modules are seated through certified rooted attachments rather than floating",hasDrive);
    TEST("Pass424 Shipyard hulls expose gameplay turret hardpoints and clear visual acceptance",hasHardpoint&&accepted);
}

static void Pass425_SourceFamilySelection(){
    auto native=ProceduralVisualVariantSystem::Build(std::vector<std::string>{"cargo_bay","cockpit_basic","cockpit_small","engine_main","engine_small","hull_section","hull_section_small","thruster","weapon_mount"},0x425u,2);
    auto external=ShipyardModuleSystem::BuildShowcaseRecipes(Fixture(),0x425u);
    native.shipRecipes.insert(native.shipRecipes.end(),external.begin(),external.end());
    const auto* sy=ProceduralVisualVariantSystem::SelectSourceFamily(native,"INDUSTRIAL","SHIPYARD_V07_CC0",1u);
    const auto* missing=ProceduralVisualVariantSystem::SelectSourceFamily(native,"INDUSTRIAL","DOES_NOT_EXIST",1u);
    TEST("Pass425 renderer can explicitly select Shipyard family for player/fitting preview",sy&&sy->sourceFamily=="SHIPYARD_V07_CC0"&&sy->role=="INDUSTRIAL");
    TEST("Pass425 source-family selection fails closed when optional corpus is unavailable",missing==nullptr);
    const auto* ordinary=ProceduralVisualVariantSystem::Select(native,"COMBAT",9u);
    TEST("Pass425 ordinary visual selection remains compatible with mixed native/external catalog",ordinary&&ordinary->role=="COMBAT");
}

static void Pass425R2_PreserveAuthoredMeshDiagnostics(){
    const std::string twoBlocks =
        "v -2 -1 -1\n" "v 0 -1 -1\n" "v 0 1 -1\n" "v -2 1 -1\n"
        "v -2 -1 1\n" "v 0 -1 1\n" "v 0 1 1\n" "v -2 1 1\n"
        "v 2 -1 -1\n" "v 4 -1 -1\n" "v 4 1 -1\n" "v 2 1 -1\n"
        "v 2 -1 1\n" "v 4 -1 1\n" "v 4 1 1\n" "v 2 1 1\n"
        "f 1 2 3 4\n" "f 5 8 7 6\n" "f 1 5 6 2\n" "f 2 6 7 3\n" "f 3 7 8 4\n" "f 5 1 4 8\n"
        "f 9 10 11 12\n" "f 13 16 15 14\n" "f 9 13 14 10\n" "f 10 14 15 11\n" "f 11 15 16 12\n" "f 13 9 12 16\n";
    const auto cert=ShipyardCertificationSystem::AnalyzeObj(twoBlocks,"twin_hull","hull");
    TEST("Pass425R2 connectivity analysis still detects authored loose geometry for diagnostics",cert.valid&&cert.islands.size()==2);
    TEST("Pass425R2 multi-island authored Shipyard object remains one Grade-A module instead of being split",cert.preserveAuthoredObject&&cert.moduleGrade==ShipyardCertificationGrade::A);
    TEST("Pass425R2 whole authored module receives assembly sockets independent of island count",!cert.sockets.empty());

    const std::string hullWithTinyFloat =
        "v -2 -2 -1\n" "v 2 -2 -1\n" "v 2 2 -1\n" "v -2 2 -1\n" "v -2 -2 1\n" "v 2 -2 1\n" "v 2 2 1\n" "v -2 2 1\n"
        "v 20 20 20\n" "v 20.02 20 20\n" "v 20 20.02 20\n"
        "f 1 2 3 4\n" "f 5 8 7 6\n" "f 1 5 6 2\n" "f 2 6 7 3\n" "f 3 7 8 4\n" "f 5 1 4 8\n" "f 9 10 11\n";
    const auto tiny=ShipyardCertificationSystem::AnalyzeObj(hullWithTinyFloat,"hull_with_authored_detail","hull");
    TEST("Pass425R2 tiny loose detail is reported but does not trigger destructive source surgery",tiny.valid&&tiny.islands.size()==2&&tiny.preserveAuthoredObject&&tiny.moduleGrade==ShipyardCertificationGrade::A);

    const std::string duplicatedSeam =
        "v 0 0 0\n" "v 1 0 0\n" "v 0 1 0\n"
        "v 1 0 0\n" "v 1 1 0\n" "v 0 1 0\n"
        "f 1 2 3\n" "f 4 5 6\n";
    const auto seam=ShipyardCertificationSystem::AnalyzeObj(duplicatedSeam,"seam","hull");
    TEST("Pass425R2 position-identical seam vertices remain one connectivity component",seam.valid&&seam.islands.size()==1);
}

static void Pass425R2_Determinism(){
    const auto first=ShipyardModuleSystem::BuildShowcaseRecipes(Fixture(),0x51A7D007u);
    const auto second=ShipyardModuleSystem::BuildShowcaseRecipes(Fixture(),0x51A7D007u);
    bool deterministic=first.size()==second.size();
    for(std::size_t i=0;i<first.size()&&i<second.size();++i){
        deterministic&=first[i].recipeId==second[i].recipeId&&first[i].modules.size()==second[i].modules.size();
        for(std::size_t m=0;m<first[i].modules.size()&&m<second[i].modules.size();++m){
            deterministic&=first[i].modules[m].moduleId==second[i].modules[m].moduleId;
            deterministic&=std::fabs(first[i].modules[m].x-second[i].modules[m].x)<0.0001f;
            deterministic&=std::fabs(first[i].modules[m].y-second[i].modules[m].y)<0.0001f;
        }
    }
    TEST("Pass425R2 socketed authored-mesh assembly remains deterministic for save/replay stability",deterministic);
}


static void Pass425R3V1_MaterialBoundaryPreservation(){
    const std::string obj=
        "mtllib fixture.mtl\n"
        "v 0 0 0\n" "v 1 0 0\n" "v 0 1 0\n"
        "vt 0 0\n" "vt 1 0\n" "vt 0 1\n"
        "vn 0 0 1\n"
        "usemtl Glass\n"
        "f 1/1/1 2/2/1 3/3/1\n";
    ObjMeshData mesh;std::string error;ObjAssetLoader loader;
    const bool ok=loader.Parse(obj,mesh,&error);
    TEST("Pass425R3V1 OBJ bridge preserves usemtl material boundaries",ok&&mesh.materialNames.size()==1&&mesh.materialNames[0]=="Glass"&&mesh.triangles.size()==1&&mesh.triangles[0].materialIndex==0);
    TEST("Pass425R3V1 OBJ bridge accepts authored UV/normal records alongside position geometry",ok&&mesh.texcoords.size()==3&&mesh.normals.size()==1&&mesh.positions.size()==3);
}

static void Pass425R3V1_RobustSurfaceContacts(){
    // Main box ends at source +Z=2. A tiny disconnected triangle at +Z=20
    // must not steal the forward assembly surface merely because it defines
    // the raw bounding-box extreme.
    const std::string obj=
        "v -2 -1 -2\n" "v 2 -1 -2\n" "v 2 1 -2\n" "v -2 1 -2\n"
        "v -2 -1 2\n" "v 2 -1 2\n" "v 2 1 2\n" "v -2 1 2\n"
        "v 0 0 20\n" "v .02 0 20\n" "v 0 .02 20\n"
        "f 1 2 3 4\n" "f 5 8 7 6\n" "f 1 5 6 2\n" "f 2 6 7 3\n" "f 3 7 8 4\n" "f 5 1 4 8\n"
        "f 9 10 11\n";
    ObjMeshData mesh;std::string error;ObjAssetLoader loader;const bool ok=loader.Parse(obj,mesh,&error);
    const auto contacts=ShipyardGeometryAnalysisSystem::Analyze(mesh);
    TEST("Pass425R3V1 area-weighted terminal analysis rejects tiny protrusion as socket authority",ok&&contacts.forward.valid&&contacts.forward.point.y<5.0f&&contacts.forward.point.y>1.0f);
    TEST("Pass425R3V1 six-axis authored surface analysis produces usable hull contacts",contacts.aft.valid&&contacts.port.valid&&contacts.starboard.valid&&contacts.dorsal.valid&&contacts.ventral.valid);
}


static void Pass425R3V1_CertificationArtifacts(){
    namespace fs=std::filesystem;
    const fs::path root=fs::temp_directory_path()/"subspace_r3v1_shipyard_cert_fixture";
    const fs::path input=root/"input",output=root/"output";
    std::error_code ec;fs::remove_all(root,ec);fs::create_directories(input,ec);
    const std::string obj=
        "mtllib fixture.mtl\n"
        "v 0 0 0\n" "v 1 0 0\n" "v 0 1 0\n"
        "vt 0 0\n" "vt 1 0\n" "vt 0 1\n"
        "vn 0 0 1\n" "usemtl Flat\n" "f 1/1/1 2/2/1 3/3/1\n";
    const std::string bare=
        "v 0 0 0\n" "v 1 0 0\n" "v 0 1 0\n" "f 1 2 3\n";
    {std::ofstream f(input/"b_hull.obj");f<<bare;}
    {std::ofstream f(input/"a_hull.obj");f<<obj;}
    {std::ofstream f(input/"fixture.mtl");f<<"newmtl Flat\nKd .4 .5 .6\nmap_Kd paint.png\n";}
    {std::ofstream f(input/"paint.png",std::ios::binary);f<<"fixture-texture";}
    ShipyardCorpusCertificationSummary summary;std::string error;
    const bool ok=ShipyardCertificationSystem::CertifyCorpus(input.string(),output.string(),&summary,&error);
    std::ifstream cat(output/"certified_module_catalog.csv");std::string header,first;std::getline(cat,header);std::getline(cat,first);
    TEST("Pass425R3V1 corpus certification is deterministic instead of filesystem-order dependent",
         ok&&first.find("shipyard_a_hull_001_a_hull")==0&&summary.sourceObjects==2);
    TEST("Pass425R3V1 certification preserves MTL and texture sidecars when source authority exists",
         ok&&fs::exists(output/"modules"/"fixture.mtl")&&fs::exists(output/"modules"/"paint.png")&&summary.resolvedMaterials==1);
    TEST("Pass425R3V1 certification emits authoritative marker and material diagnostics",
         ok&&fs::exists(output/"SHIPYARD_CERTIFIED_R3V1.txt")&&fs::exists(output/"material_diagnostics.csv"));
    fs::remove_all(root,ec);
}

static void Pass425R3V1_AttachmentGraph(){
    auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes(Fixture(),0x51A7D007u);
    bool graphs=!recipes.empty();bool noGuessedDecor=true;
    for(const auto&r:recipes){std::string e;graphs&=ShipyardModuleSystem::ValidateAssemblyGraph(r,&e);noGuessedDecor&=(r.attachments.size()+1==r.modules.size());}
    TEST("Pass425R3V1 every visible Shipyard module has one certified path to ShipRoot",graphs&&noGuessedDecor);
    if(!recipes.empty()&&recipes.front().modules.size()>1){
        auto broken=recipes.front();broken.attachments.clear();std::string e;
        TEST("Pass425R3V1 orphaned visible modules fail closed",!ShipyardModuleSystem::ValidateAssemblyGraph(broken,&e)&&!e.empty());
    } else TEST("Pass425R3V1 orphaned visible modules fail closed",false);
}

int main(){
    Pass421_GovernedIntake();
    Pass422_ModuleCatalog();
    Pass423_AssemblyRules();
    Pass424_ShowcaseAssembly();
    Pass425_SourceFamilySelection();
    Pass425R2_PreserveAuthoredMeshDiagnostics();
    Pass425R2_Determinism();
    Pass425R3V1_MaterialBoundaryPreservation();
    Pass425R3V1_RobustSurfaceContacts();
    Pass425R3V1_CertificationArtifacts();
    Pass425R3V1_AttachmentGraph();
    std::cout<<"\n=== Pass421-425 Shipyard v0.7 + R2 + R3V1 Live Material/Connectivity Summary: "<<testsPassed<<" passed, "<<testsFailed<<" failed ===\n";
    return testsFailed?1:0;
}
