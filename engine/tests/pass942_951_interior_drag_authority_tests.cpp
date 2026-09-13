#include "editor/ShipyardScreenSpaceTransformSystem.h"
#include "interior/ShipInteriorCarvingSystem.h"
#include "interior/ShipInteriorLayoutSystem.h"
#include "rendering/StrategicCamera.h"
#include "rendering/StrategicViewProjection.h"

#include <cmath>
#include <iostream>
#include <vector>

using namespace subspace;
namespace {
int failures=0,assertions=0;
void Check(bool ok,const char* n){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<n<<"\n";if(!ok)++failures;}
bool Near(float a,float b,float e=.01f){return std::fabs(a-b)<=e;}
ShipyardModuleRecord Module(const char* id,ShipyardPartRole role,ShipyardModuleSemantic semantic,float hw=2,float hl=2,float hh=1.8f){ShipyardModuleRecord r;r.source.moduleId=id;r.source.halfWidth=hw;r.source.halfLength=hl;r.source.halfHeight=hh;r.partRole=role;r.semantic=semantic;r.generatorEligible=true;return r;}
VisualModulePlacement Place(const char* id,float x,float y,float z=0){VisualModulePlacement p;p.moduleId=id;p.x=x;p.y=y;p.z=z;return p;}
}

int main(){
    std::cout<<"[Pass942-951 Interior Carving + Cursor Drag Authority]\n";

    std::vector<ShipyardModuleRecord> catalog{
        Module("bridge",ShipyardPartRole::Bridge,ShipyardModuleSemantic::CommandBridge),
        Module("hull_mid",ShipyardPartRole::PrimaryHull,ShipyardModuleSemantic::HullMid),
        Module("cargo",ShipyardPartRole::Cargo,ShipyardModuleSemantic::Component),
        Module("connector",ShipyardPartRole::StructuralAttachment,ShipyardModuleSemantic::StructuralFrame,1.2f,1.5f,1.5f),
        Module("engine",ShipyardPartRole::MainEngine,ShipyardModuleSemantic::MainEngine),
        Module("wing",ShipyardPartRole::Wing,ShipyardModuleSemantic::Wing)
    };

    ProceduralShipVisualRecipe recipe;recipe.role="INDUSTRIAL";recipe.forwardVisualYawDegrees=180.0f;
    recipe.modules={Place("bridge",0,4),Place("hull_mid",0,1),Place("connector",0,-1.5f),Place("cargo",0,-4),Place("engine",0,-7),Place("wing",4,0)};
    recipe.attachments={{0,1,"auto","auto",0,true},{1,2,"auto","auto",0,true},{2,3,"auto","auto",0,true},{3,4,"auto","auto",0,true},{1,5,"auto","auto",0,true}};

    const auto carve=ShipInteriorCarvingSystem::Carve(catalog,recipe);
    Check(carve.valid,"assembled bridge/hull/connector/cargo produce one cohesive carved interior");
    Check(carve.walkableModuleCount==4&&carve.connectedWalkableCount==4,"carving uses actual walkable module graph rather than generic room counts");
    Check(carve.exclusions.size()>=2,"engine and wing become non-habitable exclusion volumes");
    Check(carve.portals.size()>=3,"attached carved volumes are stitched by real assembly portals");
    Check(carve.volumes.front().center.y>3.5f,"carved volume preserves placed module transform");

    ShipInteriorSystem interiors;ShipInteriorLayoutSystem layoutSystem;const auto layoutPlan=layoutSystem.Materialize(942,catalog,recipe,interiors);const auto* runtimeLayout=interiors.GetLayout(942);
    Check(runtimeLayout&&runtimeLayout->rooms.size()==static_cast<std::size_t>(layoutPlan.rooms),"runtime interior materializes from carved volumes and stitched portals");
    Check(layoutPlan.carve.walkableModuleCount==4,"layout exposes carve authority for diagnostics/editor preview");

    auto disconnected=recipe;disconnected.attachments.erase(disconnected.attachments.begin()+2);disconnected.modules[3].y=-20.0f;
    const auto broken=ShipInteriorCarvingSystem::Carve(catalog,disconnected);
    Check(!broken.valid&&broken.connectedWalkableCount<broken.walkableModuleCount,"disconnected pressure-hull cavity fails cohesion certification");

    // Renderer/editor transform parity.  The certified source family often has
    // a 180-degree forwardVisualYaw; this was the reason pointer drag felt
    // almost exactly inverted before this pass.
    const auto tx=ShipyardScreenSpaceTransformSystem::Build(recipe,{0,0,0},0.0f,.24f,1.1f,1.25f);
    const Vector3 local{2.0f,3.0f,.5f};const auto world=ShipyardScreenSpaceTransformSystem::RecipeLocalToWorldPoint(local,tx);const auto round=ShipyardScreenSpaceTransformSystem::WorldPointToRecipeLocal(world,tx);
    Check(Near(round.x,local.x)&&Near(round.y,local.y)&&Near(round.z,local.z),"screen manipulation inverse matches full renderer yaw and axis scale");
    Check(world.x<0.0f&&world.y<0.0f,"180-degree visual normalization is represented in editor mapping instead of ignored");

    StrategicCamera cam;cam.SetEditorView({0,-30,18},{0,0,0});
    const auto startWorld=ShipyardScreenSpaceTransformSystem::RecipeLocalToWorldPoint({0,0,0},tx);
    const auto startScreen=StrategicViewProjection::WorldToScreen(startWorld,1280,720,cam);
    const float targetX=startScreen.x+80.0f,targetY=startScreen.y+35.0f;
    const auto cursorWorld=StrategicViewProjection::ScreenToGameplayPlane(targetX,targetY,1280,720,cam,startWorld.z);
    const auto draggedLocal=ShipyardScreenSpaceTransformSystem::WorldPointToRecipeLocal(cursorWorld,tx);
    const auto draggedWorld=ShipyardScreenSpaceTransformSystem::RecipeLocalToWorldPoint(draggedLocal,tx);
    const auto draggedScreen=StrategicViewProjection::WorldToScreen(draggedWorld,1280,720,cam);
    Check(Near(draggedScreen.x,targetX,.2f)&&Near(draggedScreen.y,targetY,.2f),"dragged module projects back under the cursor instead of moving opposite");

    std::cout<<"Assertions: "<<assertions<<" failures: "<<failures<<"\n";
    return failures==0?0:1;
}
