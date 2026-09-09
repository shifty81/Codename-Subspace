#include "content/ShipyardModuleSystem.h"
#include "ship_editor/ShipBlueprintLibrarySystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "rendering/CelestialEnvironmentSystem.h"
#include "rendering/PlanetPresentationSystem.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

using namespace subspace;

static int fail=0;
static void Check(bool value,const char* message){if(!value){++fail;std::cerr<<"FAIL: "<<message<<"\n";}}

static VisualModuleSource Src(const char* id,float w=2.0f,float l=4.0f,float h=1.0f){
    VisualModuleSource s;s.moduleId=id;s.halfWidth=w;s.halfLength=l;s.halfHeight=h;return s;
}
static void Surfaces(VisualModuleSource& s){
    s.forwardSurface={{0,s.halfLength,0},{0,1,0},1,.95f,true};
    s.aftSurface={{0,-s.halfLength,0},{0,-1,0},1,.95f,true};
    s.portSurface={{-s.halfWidth,0,0},{-1,0,0},1,.95f,true};
    s.starboardSurface={{s.halfWidth,0,0},{1,0,0},1,.95f,true};
    s.dorsalSurface={{0,0,s.halfHeight},{0,0,1},1,.95f,true};
    s.ventralSurface={{0,0,-s.halfHeight},{0,0,-1},1,.95f,true};
}
static Vector3 ForwardFrom(const VisualModulePlacement& p){
    constexpr float d=3.14159265358979323846f/180.0f;
    const float yaw=p.yawDegrees*d,pitch=p.pitchDegrees*d,roll=p.rollDegrees*d;
    Vector3 v{0,1,0};
    const float cr=std::cos(roll),sr=std::sin(roll);Vector3 a{v.x*cr+v.z*sr,v.y,-v.x*sr+v.z*cr};
    const float cp=std::cos(pitch),sp=std::sin(pitch);Vector3 b{a.x,a.y*cp-a.z*sp,a.y*sp+a.z*cp};
    const float cy=std::cos(yaw),sy=std::sin(yaw);return {b.x*cy-b.y*sy,b.x*sy+b.y*cy,b.z};
}

int main(){
    auto hull=Src("shipyard_a_hull_098_shipyard_hull_001_hullcompact",2.2f,4.6f,1.0f);
    auto bridge=Src("shipyard_a_command_005_shipyard_command_004_miscbridgecompact",1.0f,1.4f,.55f);
    auto engine=Src("shipyard_a_propulsion_129_shipyard_propulsion_012_enginecubeengine",1.0f,1.5f,.8f);
    auto wing=Src("shipyard_a_wing_156_shipyard_wing_010_miscwing2",2.4f,1.7f,.25f);
    Surfaces(hull);Surfaces(bridge);Surfaces(engine);Surfaces(wing);

    const auto catalog=ShipyardModuleSystem::BuildCatalog({hull,bridge,engine,wing});
    const auto recipes=ShipyardModuleSystem::BuildShowcaseRecipes({hull,bridge,engine,wing},0x473u);
    Check(!recipes.empty(),"Shipyard showcase generator produced recipes");
    if(!recipes.empty()){
        const auto& r=recipes.front();
        Check(std::fabs(r.forwardVisualYawDegrees-180.0f)<.001f,"Greyoxide recipe normalizes visual forward by 180 degrees");
        Check(r.forwardAuthority=="COCKPIT","generated recipe uses cockpit forward authority");
        Check(r.cockpitModuleIndex>=0&&static_cast<std::size_t>(r.cockpitModuleIndex)<r.modules.size(),"generated recipe records cockpit module index");

        auto ports=ShipyardModuleSystem::BuildPropulsionPorts(catalog,r);
        auto mainIt=std::find_if(ports.begin(),ports.end(),[](const auto& p){return p.semantic==ShipyardModuleSemantic::MainEngine||p.semantic==ShipyardModuleSemantic::EngineNozzle;});
        Check(mainIt!=ports.end(),"generated ship exposes authored main propulsion port");
        if(mainIt!=ports.end()){
            constexpr float pi=3.14159265358979323846f;
            const float yaw=r.forwardVisualYawDegrees*pi/180.0f;
            const float cy=std::cos(yaw),sy=std::sin(yaw);
            const float worldY=mainIt->exhaustDirection.x*sy+mainIt->exhaustDirection.y*cy;
            Check(worldY<-.85f,"forward-thrust plume points gameplay aft after visual normalization");
        }

        ShipBlueprintDocument doc;doc.name="Pass473 Orientation";doc.recipe=r;
        const auto path=(std::filesystem::temp_directory_path()/"subspace_pass473_orientation.subspace_ship").string();
        std::string error;
        Check(ShipBlueprintLibrarySystem::Save(doc,path,&error),"blueprint orientation save succeeds");
        ShipBlueprintDocument loaded;
        Check(ShipBlueprintLibrarySystem::Load(path,loaded,&error),"blueprint orientation load succeeds");
        Check(std::fabs(loaded.recipe.forwardVisualYawDegrees-r.forwardVisualYawDegrees)<.001f,"blueprint preserves visual forward yaw");
        Check(loaded.recipe.forwardAuthority==r.forwardAuthority,"blueprint preserves forward authority");
        Check(loaded.recipe.cockpitModuleIndex==r.cockpitModuleIndex,"blueprint preserves cockpit module index");
        std::error_code ec;std::filesystem::remove(path,ec);
    }

    // Explicit lateral socket symmetry: both sides must preserve the same
    // ship-relative forward direction rather than one wing flipping 180°.
    const auto* hr=&*std::find_if(catalog.begin(),catalog.end(),[](const auto& r){return r.moduleClass==ShipyardModuleClass::Hull;});
    const auto* wr=&*std::find_if(catalog.begin(),catalog.end(),[](const auto& r){return r.moduleClass==ShipyardModuleClass::Wing;});
    const auto* port=&*std::find_if(hr->sockets.begin(),hr->sockets.end(),[](const auto& s){return s.name=="port_aft";});
    const auto* starboard=&*std::find_if(hr->sockets.begin(),hr->sockets.end(),[](const auto& s){return s.name=="starboard_aft";});
    const auto* mount=&*std::find_if(wr->sockets.begin(),wr->sockets.end(),[](const auto& s){return s.name=="mount";});
    VisualModulePlacement hp;hp.moduleId=hr->source.moduleId;hp.scaleX=hp.scaleY=hp.scaleZ=.6f;
    using AttachFn=VisualModulePlacement (*)(const VisualModulePlacement&,const ShipyardAssemblySocket&,const ShipyardModuleRecord&,const ShipyardAssemblySocket&,float);
    AttachFn attach=&ShipyardModuleSystem::BuildAttachmentPlacement;
    auto wp=attach(hp,*port,*wr,*mount,.5f);
    auto ws=attach(hp,*starboard,*wr,*mount,.5f);
    const auto fp=ForwardFrom(wp),fs=ForwardFrom(ws);
    Check((fp.x*fs.x+fp.y*fs.y+fp.z*fs.z)>.85f,"mirrored lateral pair preserves common forward orientation");

    // The canonical validator must catch a manually inverted mirror partner.
    const auto* br=&*std::find_if(catalog.begin(),catalog.end(),[](const auto& r){return r.moduleClass==ShipyardModuleClass::Command;});
    const auto* er=&*std::find_if(catalog.begin(),catalog.end(),[](const auto& r){return r.semantic==ShipyardModuleSemantic::MainEngine;});
    const auto* dorsal=&*std::find_if(hr->sockets.begin(),hr->sockets.end(),[](const auto& s){return s.name=="dorsal_forward";});
    const auto* enginePort=&*std::find_if(hr->sockets.begin(),hr->sockets.end(),[](const auto& s){return s.name=="engine_port";});
    const auto* bridgeMount=&*std::find_if(br->sockets.begin(),br->sockets.end(),[](const auto& s){return s.name=="mount";});
    const auto* engineMount=&*std::find_if(er->sockets.begin(),er->sockets.end(),[](const auto& s){return s.name=="mount";});
    auto bp=attach(hp,*dorsal,*br,*bridgeMount,.45f);
    auto ep=attach(hp,*enginePort,*er,*engineMount,.45f);

    ProceduralShipVisualRecipe pairRecipe;pairRecipe.role="INDUSTRIAL";
    pairRecipe.modules={hp,wp,ws,bp,ep};
    pairRecipe.attachments.push_back({0,1,"port_aft","mount",0,true});
    pairRecipe.attachments.push_back({0,2,"starboard_aft","mount",0,true});
    pairRecipe.attachments.push_back({0,3,"dorsal_forward","mount",0,true});
    pairRecipe.attachments.push_back({0,4,"engine_port","mount",0,true});
    pairRecipe.modules[2].yawDegrees+=180.0f;
    const auto badMirror=ShipyardModuleSystem::ValidateAssemblyGraph(catalog,pairRecipe);
    Check(std::any_of(badMirror.errors.begin(),badMirror.errors.end(),[](const auto& e){return e.find("Mirrored pair orientation mismatch")!=std::string::npos;}),"validator rejects forward-reversed mirror partner");

    PlanetData ocean;ocean.type=PlanetType::Oceanic;
    const auto oceanProfile=CelestialEnvironmentSystem{}.ProfileFor(ocean);
    Check(oceanProfile.hasCloudLayer&&oceanProfile.cloudOpacity>=.28f,"ocean planet presentation requires visible clouds");
    Check(PlanetPresentationSystem::AtmosphereLimb(.05f)>PlanetPresentationSystem::AtmosphereLimb(.80f)*20.0f,"atmosphere is strongly limb weighted instead of a full-disc bubble");

    std::cout<<"Pass473 assertions: "<<(14-fail)<<" passed / "<<fail<<" failed\n";
    return fail?1:0;
}
