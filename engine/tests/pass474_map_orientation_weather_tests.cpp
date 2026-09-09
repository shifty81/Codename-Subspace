#include "ships/ShipyardAuthoredShipSystem.h"
#include "rendering/PlanetPresentationSystem.h"
#include "integration/PlayerFacingIntegrationSystem.h"
#include "navigation/SystemMapSystem.h"
#include "ui/ContextActionSystem.h"
#include <cmath>
#include <iostream>
#include <string>

using namespace subspace;
static int failures=0;
static void Check(bool ok,const char* name){
    std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";
    if(!ok)++failures;
}

int main(){
    std::cout<<"[Pass474MapOrientationWeather]\n";

    for(const auto& def:ShipyardAuthoredShipSystem::Definitions()){
        const auto recipe=ShipyardAuthoredShipSystem::BuildRecipe(def);
        Check(std::fabs(recipe.forwardVisualYawDegrees-180.0f)<.001f,"authored reference ship preserves Greyoxide forward normalization");
        Check(recipe.forwardAuthority=="AUTHORED_COCKPIT","authored reference ship declares cockpit forward authority");
        Check(recipe.cockpitModuleIndex==0,"authored composite records its visual module as forward authority");
    }

    const float surface0=PlanetPresentationSystem::SurfaceRotationPhase(0.0f,17.0f);
    const float surface30=PlanetPresentationSystem::SurfaceRotationPhase(30.0f,17.0f);
    const float low0=PlanetPresentationSystem::CloudRotationPhase(0.0f,0,17.0f);
    const float low30=PlanetPresentationSystem::CloudRotationPhase(30.0f,0,17.0f);
    const float high30=PlanetPresentationSystem::CloudRotationPhase(30.0f,1,17.0f);
    Check(std::fabs((low30-low0)-(surface30-surface0))>.005f,"cloud layer clock visibly diverges from planet surface clock");
    Check(std::fabs(low30-high30)>.05f,"low and high cloud layers keep independent phase");
    Check(std::fabs(PlanetPresentationSystem::ProceduralWeatherLongitude(0.0f,30.0f,1.0f))>.30f,"procedural weather drifts longitudinally over thirty seconds");

    SystemMapSnapshot map;map.systemName="TEST";
    SystemMapNode star;star.id=1;star.kind=SystemMapNodeKind::Star;star.label="Helios";star.position={0,0,0};star.known=true;star.warpable=false;star.strategicRadius=10;
    SystemMapNode moon;moon.id=2;moon.kind=SystemMapNodeKind::Moon;moon.label="Selene";moon.position={120,0,0};moon.known=true;moon.warpable=true;moon.strategicRadius=4;
    map.nodes={star,moon};
    PlayerFacingIntegrationSystem ui;
    const auto layout=ui.LayoutSystemMap(map,nullptr,1600,900,1.0f,{});
    const float moonX=layout.centerX+moon.position.x*layout.scale;
    const float moonY=layout.centerY-moon.position.y*layout.scale;
    Check(ui.HitTestSystemMapNode(map,nullptr,1600,900,1.0f,{},moonX,moonY)==1,"system map LMB hit-test resolves exact moon node");
    Check(std::string(SystemMapSystem::KindName(SystemMapNodeKind::Moon))=="MOON","system map exposes semantic moon label");
    Check(std::string(SystemMapSystem::KindName(SystemMapNodeKind::Belt))=="ASTEROID BELT","system map exposes semantic belt label");

    InteractionContext context;context.kind=ContextObjectKind::MapDestination;context.discovered=true;
    const auto actions=ContextActionSystem{}.Resolve(context);
    Check(actions.size()==2&&actions[0].id=="vector"&&actions[1].id=="info","map destination context is travel/info only instead of local-space approach commands");

    std::cout<<"Pass474 assertions: "<<(failures?"FAIL":"PASS")<<"\n";
    return failures?1:0;
}
