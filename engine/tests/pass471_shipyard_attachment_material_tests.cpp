#include "content/ShipyardModuleSystem.h"
#include "core/resources/ObjAssetLoader.h"
#include "ship_editor/ShipyardBuilderSystem.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace subspace;
namespace fs=std::filesystem;
static int passed=0,failed=0;
#define TEST(n,e) do{if(e){++passed;std::cout<<"[PASS] "<<n<<"\n";}else{++failed;std::cout<<"[FAIL] "<<n<<"\n";}}while(0)

static Vector3 Rotate(float x,float y,float z,float yawDeg,float pitchDeg,float rollDeg){
    constexpr float d=3.14159265358979323846f/180.0f;
    const float yaw=yawDeg*d,pitch=pitchDeg*d,roll=rollDeg*d;
    const float cr=std::cos(roll),sr=std::sin(roll);const float x1=x*cr+z*sr,y1=y,z1=-x*sr+z*cr;
    const float cp=std::cos(pitch),sp=std::sin(pitch);const float x2=x1,y2=y1*cp-z1*sp,z2=y1*sp+z1*cp;
    const float cy=std::cos(yaw),sy=std::sin(yaw);return {x2*cy-y2*sy,x2*sy+y2*cy,z2};
}
static float Dist(const Vector3&a,const Vector3&b){return (a-b).length();}

int main(){
    VisualModuleSource hull{"shipyard_a_hull_001_hull_long",3.0f,5.0f,2.0f};
    hull.forwardSurface={{1.2f,7.4f,.4f},{0,1,0},8.0f,.9f,true};
    hull.aftSurface={{1.0f,-2.6f,.3f},{0,-1,0},7.5f,.9f,true};
    hull.portSurface={{-4.0f,2.2f,.2f},{-1,0,0},5.0f,.8f,true};
    hull.starboardSurface={{5.0f,2.1f,.2f},{1,0,0},5.2f,.8f,true};
    hull.dorsalSurface={{.5f,2.0f,3.6f},{0,0,1},6.0f,.85f,true};
    hull.ventralSurface={{.5f,2.0f,-1.1f},{0,0,-1},6.0f,.85f,true};
    const auto sockets=ShipyardModuleSystem::BuildSockets(hull,ShipyardModuleSemantic::HullMid);
    auto find=[&](const char* name)->const ShipyardAssemblySocket*{for(const auto&s:sockets)if(s.name==name)return &s;return nullptr;};
    const auto* forward=find("forward");const auto* dorsal=find("dorsal");const auto* enginePort=find("engine_port");
    TEST("Pass471 hull sockets use authored geometry contact coordinates",forward&&std::fabs(forward->x-1.2f)<.001f&&std::fabs(forward->y-7.4f)<.001f&&dorsal&&std::fabs(dorsal->z-3.6f)<.001f);
    TEST("Pass471 engine sockets compensate for non-centered authored pivots",enginePort&&enginePort->x<-1.0f&&enginePort->y>-2.6f&&enginePort->y<7.4f);

    ShipyardModuleRecord child;child.source={"shipyard_a_hull_002_hull_compact",1,1,1};child.moduleClass=ShipyardModuleClass::Hull;child.semantic=ShipyardModuleSemantic::HullMid;
    ShipyardAssemblySocket parentSocket{"forward","hull_forward",0,2,0,0,1,0,0};
    ShipyardAssemblySocket childSocket{"aft","hull_aft",0,-1,0,0,-1,0,0};
    VisualModulePlacement parent;parent.moduleId="parent";parent.yawDegrees=90.0f;
    using AttachFn=VisualModulePlacement (*)(const VisualModulePlacement&,const ShipyardAssemblySocket&,const ShipyardModuleRecord&,const ShipyardAssemblySocket&,float);
    AttachFn attach=&ShipyardModuleSystem::BuildAttachmentPlacement;
    const auto placed=attach(parent,parentSocket,child,childSocket,1.0f);
    const auto parentOff=Rotate(parentSocket.x,parentSocket.y,parentSocket.z,parent.yawDegrees,parent.pitchDegrees,parent.rollDegrees);
    const Vector3 parentWorld{parent.x+parentOff.x,parent.y+parentOff.y,parent.z+parentOff.z};
    const auto childOff=Rotate(childSocket.x,childSocket.y,childSocket.z,placed.yawDegrees,placed.pitchDegrees,placed.rollDegrees);
    const Vector3 childWorld{placed.x+childOff.x,placed.y+childOff.y,placed.z+childOff.z};
    TEST("Pass471 nested attachment transform honors rotated parent socket",Dist(parentWorld,childWorld)<.001f);

    VisualModuleSource engine{"shipyard_a_propulsion_001_engine5engine",1.0f,1.8f,1.0f};
    engine.forwardSurface={{0,2.2f,0},{0,1,0},3.0f,.9f,true};
    engine.aftSurface={{0,-1.4f,0},{0,-1,0},3.0f,.9f,true};
    auto catalog=ShipyardModuleSystem::BuildCatalog({hull,engine});
    ProceduralShipVisualRecipe recipe;VisualModulePlacement rootPlacement;rootPlacement.moduleId=hull.moduleId;rootPlacement.scaleX=rootPlacement.scaleY=rootPlacement.scaleZ=.72f;recipe.modules.push_back(rootPlacement);
    ShipyardBuilderSystem builder;builder.Initialize(catalog,recipe);builder.Activate(ShipyardBuilderCommand::SelectClass,static_cast<int>(ShipyardModuleClass::Propulsion));
    const bool added=builder.Activate(ShipyardBuilderCommand::AddModule);
    bool dedicated=false;if(added&&builder.Recipe().modules.size()>=2){const auto&p=builder.Recipe().modules[1];dedicated=std::fabs(p.x)>.20f;}
    TEST("Pass471 manual builder prefers dedicated engine sockets over generic lateral surfaces",added&&dedicated);

    const fs::path temp=fs::temp_directory_path()/"subspace_pass471_obj_material";std::error_code ec;fs::remove_all(temp,ec);fs::create_directories(temp,ec);
    {std::ofstream m(temp/"sample.mtl");m<<"newmtl Painted\nKd 0.20 0.40 0.80\nNs 500\nPr 0.33\nPm 0.72\nmap_Kd albedo.png\n";}
    {std::ofstream t(temp/"albedo.png",std::ios::binary);t<<"placeholder";}
    {std::ofstream o(temp/"sample.obj");o<<"mtllib sample.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0 0\nvt 1 0\nvt 0 1\nvn 0 0 1\nusemtl Painted\nf 1/1/1 2/2/1 3/3/1\n";}
    ObjMeshData mesh;std::string error;ObjAssetLoader loader;const bool loaded=loader.LoadFile((temp/"sample.obj").string(),mesh,&error);
    const bool materialOk=loaded&&mesh.materials.size()==1&&mesh.materials[0].resolvedFromMtl&&mesh.materials[0].hasDiffuseColor&&
        std::fabs(mesh.materials[0].diffuseB-.80f)<.001f&&mesh.materials[0].baseColorTexturePath.find("albedo.png")!=std::string::npos&&
        std::fabs(mesh.materials[0].roughness-.33f)<.001f&&std::fabs(mesh.materials[0].metallic-.72f)<.001f;
    TEST("Pass471 OBJ loader resolves authored MTL color/PBR/base-color sidecars",materialOk);
    TEST("Pass471 OBJ loader preserves UV and normal indices for textured rendering",loaded&&!mesh.triangles.empty()&&mesh.triangles[0].texcoord[1]==1&&mesh.triangles[0].normal[2]==0);
    fs::remove_all(temp,ec);

    std::cout<<"Pass471 assertions: "<<passed<<" passed / "<<failed<<" failed\n";return failed?1:0;
}
