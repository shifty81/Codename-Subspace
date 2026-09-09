#include "assets/CanonicalAssetValidation.h"
#include "assets/CanonicalAssetRegistry.h"
#include "content/ShipyardCanonicalAssetBridge.h"
#include "content/ShipyardModuleSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using namespace subspace;

namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}

VisualModuleSurfaceContact Surface(float x,float y,float z,float nx,float ny,float nz){
    return {{x,y,z},{nx,ny,nz},1.0f,1.0f,true};
}
VisualModuleSource Source(const char* id,float w,float l,float h){
    VisualModuleSource s;s.moduleId=id;s.halfWidth=w;s.halfLength=l;s.halfHeight=h;
    s.forwardSurface=Surface(0,l,0,0,1,0);s.aftSurface=Surface(0,-l,0,0,-1,0);
    s.portSurface=Surface(-w,0,0,-1,0,0);s.starboardSurface=Surface(w,0,0,1,0,0);
    s.dorsalSurface=Surface(0,0,h,0,0,1);s.ventralSurface=Surface(0,0,-h,0,0,-1);
    return s;
}
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>& c,ShipyardModuleSemantic semantic){
    for(const auto& r:c)if(r.semantic==semantic)return &r;return nullptr;
}
const ShipyardAssemblySocket* Socket(const ShipyardModuleRecord& r,const char* name){for(const auto& s:r.sockets)if(s.name==name)return &s;return nullptr;}
}

int main(){
    auto hull=Source("shipyard_a_hull_098_shipyard_hull_001_hullcompact",2.4f,4.8f,1.0f);
    auto command=Source("shipyard_a_command_005_shipyard_command_004_miscbridgecompact",.9f,1.2f,.5f);
    auto engine=Source("shipyard_a_propulsion_129_shipyard_propulsion_012_enginecubeengine",.8f,1.4f,.6f);
    auto rcs=Source("shipyard_a_propulsion_999_rcs_thruster",.25f,.35f,.25f);
    const auto catalog=ShipyardModuleSystem::BuildCatalog({hull,command,engine,rcs});
    const auto* hr=Find(catalog,ShipyardModuleSemantic::HullMid);
    const auto* cr=Find(catalog,ShipyardModuleSemantic::CommandBridge);
    const auto* er=Find(catalog,ShipyardModuleSemantic::MainEngine);
    Check(hr&&cr&&er,"synthetic certified catalog resolves hull/command/main engine");
    if(!hr||!cr||!er)return 1;

    const auto* rear=Socket(*hr,"engine_port");
    const auto* engineMount=Socket(*er,"mount");
    const auto* commandMount=Socket(*cr,"mount");
    const auto* dorsal=Socket(*hr,"dorsal_forward");
    Check(rear&&engineMount&&commandMount&&dorsal,"required assembly sockets exist");
    if(rear) Check(rear->dirY>0.90f,"Greyoxide canonical rear-drive socket compensates 180-degree source-family yaw");
    Check(ShipyardModuleSystem::IsRearDriveSocketName("engine_port")&&ShipyardModuleSystem::IsRearDriveSocketName("engine_wing_aft"),"rear-drive socket taxonomy includes hull and wing-aft mounts");
    Check(!ShipyardModuleSystem::IsRearDriveSocketName("dorsal"),"ordinary surface is not a rear-drive socket");

    ProceduralShipVisualRecipe recipe;recipe.role="INDUSTRIAL";recipe.sourceFamily="SHIPYARD_V07_CC0";recipe.forwardVisualYawDegrees=180.0f;recipe.forwardAuthority="FORWARD_MARKER";
    VisualModulePlacement hp;hp.moduleId=hr->source.moduleId;hp.scaleX=hp.scaleY=hp.scaleZ=.6f;recipe.modules.push_back(hp);
    using AttachFn=VisualModulePlacement (*)(const VisualModulePlacement&,
                                             const ShipyardAssemblySocket&,
                                             const ShipyardModuleRecord&,
                                             const ShipyardAssemblySocket&,
                                             float);
    AttachFn attach=&ShipyardModuleSystem::BuildAttachmentPlacement;
    auto cp=attach(hp,*dorsal,*cr,*commandMount,.45f);recipe.modules.push_back(cp);recipe.attachments.push_back({0,1,"dorsal_forward","mount",0,true});
    auto ep=attach(hp,*rear,*er,*engineMount,.45f);recipe.modules.push_back(ep);recipe.attachments.push_back({0,2,"engine_port","mount",0,true});

    std::string propulsionError;
    Check(ShipyardModuleSystem::ValidatePropulsionPlacement(catalog,recipe,2,&propulsionError),"main engine validates through canonical rear-drive socket");
    const auto graph=ShipyardModuleSystem::ValidateAssemblyGraph(catalog,recipe);
    Check(graph.valid,"assembly graph accepts valid hard rear propulsion");

    const auto ports=ShipyardModuleSystem::BuildPropulsionPorts(catalog,recipe);
    const auto it=std::find_if(ports.begin(),ports.end(),[](const auto& p){return p.semantic==ShipyardModuleSemantic::MainEngine;});
    Check(it!=ports.end(),"authored main engine exposes propulsion port");
    if(it!=ports.end()){
        const float yaw=recipe.forwardVisualYawDegrees*3.14159265358979323846f/180.0f;
        const float worldY=it->exhaustDirection.x*std::sin(yaw)+it->exhaustDirection.y*std::cos(yaw);
        Check(worldY<-.85f,"actual transformed nozzle exhaust points canonical ship-aft without renderer override");
    }

    auto inverted=recipe;inverted.modules[2].yawDegrees+=180.0f;
    Check(!ShipyardModuleSystem::ValidatePropulsionPlacement(catalog,inverted,2,&propulsionError),"hard validator rejects forward-facing main-engine exhaust");
    const auto invertedGraph=ShipyardModuleSystem::ValidateAssemblyGraph(catalog,inverted);
    Check(!invertedGraph.valid,"assembly certification fails an inverted main engine");

    auto wrongChain=recipe;wrongChain.attachments.back().parentSocket="dorsal";
    Check(!ShipyardModuleSystem::ValidatePropulsionPlacement(catalog,wrongChain,2,&propulsionError),"hard validator rejects main engine outside rear-drive ancestry");

    const auto canonical=ShipyardCanonicalAssetBridge::BuildAsset(*er);
    const auto report=assets::CanonicalAssetValidator::Validate(canonical);
    Check(report.Passed(),"Shipyard module projects into validator-clean CanonicalAsset identity");
    Check(!canonical.modules.empty()&&!canonical.sockets.empty(),"canonical projection carries module and mating-socket metadata");
    bool hasExhaust=false;for(const auto& p:canonical.proxies)if(p.type==assets::ProxyType::Exhaust)hasExhaust=true;
    Check(hasExhaust,"canonical projection preserves exhaust as functional proxy");

    assets::CanonicalAssetRegistry registry;
    const auto registered=ShipyardCanonicalAssetBridge::PopulateRegistry(registry,catalog);
    Check(registered==catalog.size()&&registry.Size()==catalog.size(),"runtime CanonicalAsset registry receives the complete Shipyard catalog");
    Check(registry.Find(er->source.moduleId)!=nullptr,"runtime canonical registry resolves main engine by stable module id");

    std::cout<<"Pass533A assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
