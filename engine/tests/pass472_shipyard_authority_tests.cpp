#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardPartTaxonomySystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include <algorithm>
#include <cmath>
#include <iostream>
using namespace subspace;
static int fail=0; static void Check(bool v,const char* m){if(!v){++fail;std::cerr<<"FAIL: "<<m<<"\n";}}
static VisualModuleSource Src(const char* id,float w=2,float l=4,float h=1){VisualModuleSource s;s.moduleId=id;s.halfWidth=w;s.halfLength=l;s.halfHeight=h;return s;}
int main(){
    auto hull=Src("shipyard_a_hull_001_hullcompact");auto engine=Src("shipyard_a_propulsion_001_enginecubeengine",1,1.5f,.8f);auto bridge=Src("shipyard_a_command_001_miscbridge1",1,1,.5f);
    hull.forwardSurface={{0,4,0},{0,1,0},1,.9f,true};hull.aftSurface={{0,-4,0},{0,-1,0},1,.9f,true};hull.portSurface={{-2,0,0},{-1,0,0},1,.9f,true};hull.starboardSurface={{2,0,0},{1,0,0},1,.9f,true};hull.dorsalSurface={{0,0,1},{0,0,1},1,.9f,true};hull.ventralSurface={{0,0,-1},{0,0,-1},1,.9f,true};
    engine.forwardSurface={{0,1.5f,0},{0,1,0},1,.9f,true};engine.aftSurface={{0,-1.5f,0},{0,-1,0},1,.9f,true};engine.portSurface={{-1,0,0},{-1,0,0},1,.9f,true};engine.starboardSurface={{1,0,0},{1,0,0},1,.9f,true};engine.dorsalSurface={{0,0,.8f},{0,0,1},1,.9f,true};engine.ventralSurface={{0,0,-.8f},{0,0,-1},1,.9f,true};
    bridge.forwardSurface={{0,1,0},{0,1,0},1,.9f,true};bridge.aftSurface={{0,-1,0},{0,-1,0},1,.9f,true};bridge.portSurface={{-1,0,0},{-1,0,0},1,.9f,true};bridge.starboardSurface={{1,0,0},{1,0,0},1,.9f,true};bridge.dorsalSurface={{0,0,.5f},{0,0,1},1,.9f,true};bridge.ventralSurface={{0,0,-.5f},{0,0,-1},1,.9f,true};
    auto catalog=ShipyardModuleSystem::BuildCatalog({hull,engine,bridge});
    const auto* hr=&*std::find_if(catalog.begin(),catalog.end(),[](auto& r){return r.moduleClass==ShipyardModuleClass::Hull;});
    const auto* er=&*std::find_if(catalog.begin(),catalog.end(),[](auto& r){return r.semantic==ShipyardModuleSemantic::MainEngine;});
    Check(!ShipyardModuleSystem::CanMate("lateral_surface","engine_mount"),"engine cannot certify on generic lateral surface");
    Check(std::any_of(hr->sockets.begin(),hr->sockets.end(),[](auto&s){return s.name=="dorsal_forward";}),"hull exposes authored dorsal forward socket");
    Check(std::any_of(hr->sockets.begin(),hr->sockets.end(),[](auto&s){return s.name=="port_aft";}),"hull exposes authored lateral station sockets");
    const auto ps=*std::find_if(hr->sockets.begin(),hr->sockets.end(),[](auto&s){return s.name=="engine_port";});
    const auto cs=*std::find_if(er->sockets.begin(),er->sockets.end(),[](auto&s){return s.name=="mount";});
    ProceduralShipVisualRecipe r;r.role="INDUSTRIAL";r.sourceFamily="SHIPYARD_V07_CC0";r.forwardVisualYawDegrees=180.0f;VisualModulePlacement hp;hp.moduleId=hr->source.moduleId;hp.scaleX=hp.scaleY=hp.scaleZ=.6f;r.modules.push_back(hp);
    using AttachFn=VisualModulePlacement (*)(const VisualModulePlacement&,const ShipyardAssemblySocket&,const ShipyardModuleRecord&,const ShipyardAssemblySocket&,float);AttachFn attach=&ShipyardModuleSystem::BuildAttachmentPlacement;
    auto ep=attach(hp,ps,*er,cs,.5f);r.modules.push_back(ep);r.attachments.push_back({0,1,"engine_port","mount",0,true});
    // Add command on exact dorsal socket so required roles are present.
    const auto* br=&*std::find_if(catalog.begin(),catalog.end(),[](auto& x){return x.moduleClass==ShipyardModuleClass::Command;});
    const auto d=*std::find_if(hr->sockets.begin(),hr->sockets.end(),[](auto&s){return s.name=="dorsal_forward";});const auto bm=*std::find_if(br->sockets.begin(),br->sockets.end(),[](auto&s){return s.name=="mount";});
    auto bp=attach(hp,d,*br,bm,.45f);r.modules.push_back(bp);r.attachments.push_back({0,2,"dorsal_forward","mount",0,true});
    Check(ShipyardModuleSystem::ValidateAssemblyGraph(catalog,r).valid,"exact socket-built recipe validates");
    r.modules[1].x+=3.0f;auto bad=ShipyardModuleSystem::ValidateAssemblyGraph(catalog,r);Check(!bad.valid,"detached socket child fails validation");
    Check(std::any_of(bad.errors.begin(),bad.errors.end(),[](auto&e){return e.find("Detached Shipyard attachment")!=std::string::npos;}),"detached validation reports spatial gap");
    Check(ShipyardPartTaxonomySystem::RoleFor(ShipyardModuleSemantic::Component,"miscTank")==ShipyardPartRole::Tank,"tank taxonomy");
    Check(ShipyardPartTaxonomySystem::RoleFor(ShipyardModuleSemantic::Component,"miscWindowBlockLowPro")==ShipyardPartRole::WindowCanopy,"window taxonomy");
    Check(ShipyardPartTaxonomySystem::RoleFor(ShipyardModuleSemantic::Sensor,"miscTelescope")==ShipyardPartRole::Telescope,"telescope taxonomy");
    Check(ShipyardPartTaxonomySystem::RoleFor(ShipyardModuleSemantic::Component,"miscCrabFace")==ShipyardPartRole::ReviewRequired,"ambiguous taxonomy is review-required");
    std::cout<<"Pass472 assertions: "<<(10-fail)<<" passed / "<<fail<<" failed\n";return fail?1:0;
}
