#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardCapabilitySystem.h"
#include "ship_editor/ShipyardWorkspaceSystem.h"
#include "developer/ShipyardDevWorldSystem.h"
#include "character/CharacterAnimationLibrarySystem.h"
#include "interior/ModularInteriorKitSystem.h"
#include "interior/ShipModuleInteriorLinkSystem.h"
#include "procedural/ProceduralEncounterGenerator.h"
#include "world/WorldScaleAuthoritySystem.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
int passed=0,failed=0;
void Check(bool condition,const std::string& name){if(condition){++passed;std::cout<<"[PASS] "<<name<<"\n";}else{++failed;std::cout<<"[FAIL] "<<name<<"\n";}}
bool Near(float a,float b,float e=.001f){return std::fabs(a-b)<=e;}

ShipyardModuleRecord Hull(const std::string& id, ShipyardModuleSemantic semantic){
    ShipyardModuleRecord r;
    r.source.moduleId=id;
    r.source.halfWidth=2.0f;r.source.halfLength=3.0f;r.source.halfHeight=1.5f;
    r.moduleClass=ShipyardModuleClass::Hull;r.semantic=semantic;r.size=ShipyardModuleSize::M;
    r.builderCategory=ShipyardPartCategory::Hull;r.partRole=ShipyardPartRole::PrimaryHull;
    r.primaryHull=true;r.generatorEligible=true;
    r.sockets.push_back({"front","hull",0,3,0,0,1,0,.05f});
    r.sockets.push_back({"rear","hull",0,-3,0,0,-1,0,.05f});
    return r;
}
}

int main(){
    // 735: preserve the historical aggregate initializer shape while extending
    // interior metadata append-only.
    InteriorModuleAssetDef historical{"legacy.wall","legacy_source",InteriorModuleKind::Wall,1,2,2.0,3.0,true,false,
        {{"west",InteriorSocketDirection::West,0,0,0,"interior"}}};
    Check(historical.sockets.size()==1 && historical.sourcePackId.empty(),"735 historical InteriorModuleAssetDef aggregate initialization remains source compatible");

    const auto playerCaps=ShipyardCapabilitySystem::For(ShipyardAccessMode::PlayerDocked);
    const auto devCaps=ShipyardCapabilitySystem::For(ShipyardAccessMode::MainMenuStudio);
    Check(!playerCaps.model&&!playerCaps.interior&&!playerCaps.character&&!playerCaps.devWorld,"736 player-docked Shipyard keeps advanced authoring hidden");
    Check(devCaps.model&&devCaps.interior&&devCaps.character&&devCaps.pcgStudio&&devCaps.world&&devCaps.devWorld&&devCaps.rawAuthoring,"736 main-menu Shipyard is the cumulative Dev Studio authority");

    auto a=Hull("shipyard_a_hull_mid_a",ShipyardModuleSemantic::HullMid);
    auto b=Hull("shipyard_a_hull_mid_b",ShipyardModuleSemantic::HullMid);
    ProceduralShipVisualRecipe recipe;recipe.recipeId="dev.fixture";recipe.role="INDUSTRIAL";
    recipe.modules.push_back({a.source.moduleId,0,0,0});
    recipe.modules.push_back({b.source.moduleId,0,6,0});
    recipe.attachments.push_back({0,1,"front","rear",0,true});

    ShipyardBuilderSystem builder;builder.Initialize({a,b},recipe);
    Check(builder.Activate(ShipyardBuilderCommand::WorkspaceInterior)&&builder.Model().workspaceMode==ShipyardWorkspaceMode::Interior,"736 INTERIOR workspace is live in the existing Shipyard");
    Check(builder.Model().interiorPlan.valid&&builder.Model().interiorPlan.bindings.size()==2,"736 Shipyard interior workspace maps exterior modules into the shared interior authority");
    Check(builder.Activate(ShipyardBuilderCommand::WorkspaceCharacter)&&builder.Model().workspaceMode==ShipyardWorkspaceMode::Character,"737 CHARACTER workspace is live in the existing Shipyard");
    const float oldHeight=builder.Model().worldScale.referencePlayerHeightMeters;
    Check(builder.Activate(ShipyardBuilderCommand::ScalePlayerUp)&&builder.Model().worldScale.referencePlayerHeightMeters>oldHeight,"737 player reference scale is authorable from Shipyard Dev Mode");
    Check(builder.Model().worldScale.referenceDoorHeightMeters>WorldScaleAuthoritySystem::DefaultProfile().referenceDoorHeightMeters,"737 player scale coherently drives human-scale world dimensions");
    Check(builder.Activate(ShipyardBuilderCommand::WorkspaceDevWorld)&&builder.Model().workspaceMode==ShipyardWorkspaceMode::DevWorld,"738 DEV WORLD workspace is live in the existing Shipyard");
    const auto backdrop=builder.Model().devWorld.backdrop;
    Check(builder.Activate(ShipyardBuilderCommand::DevWorldNextBackdrop)&&builder.Model().devWorld.backdrop!=backdrop,"738 Dev World backdrop can be changed from Shipyard");

    const auto scale=WorldScaleAuthoritySystem::DefaultProfile();
    const auto dev=ShipyardDevWorldSystem::CreateDefault(scale);
    bool animation=false,catalog=false,ships=false,interior=false,pcg=false,terraform=false;
    for(const auto& z:dev.zones){animation|=z.kind==DevWorldZoneKind::AnimationViewer;catalog|=z.kind==DevWorldZoneKind::KitbashCatalog;ships|=z.kind==DevWorldZoneKind::CertifiedShips;interior|=z.kind==DevWorldZoneKind::InteriorKit;pcg|=z.kind==DevWorldZoneKind::PcgProvingGround;terraform|=z.kind==DevWorldZoneKind::Terraforming;}
    Check(dev.enabled&&dev.backdrop==DevWorldBackdrop::Checkerboard&&Near(dev.chunkSizeMeters,4096.0f),"738 checkerboard developer chunk is the default in-game proving ground");
    Check(animation&&catalog&&ships&&interior&&pcg&&terraform,"738 Dev World lays out character, kitbash, certified ship, interior, PCG and terraforming lanes");

    const auto anim=CharacterAnimationLibrarySystem::QuaterniusUniversalAnimationLibrary2();
    Check(anim.libraryId.find("quaternius")!=std::string::npos&&anim.expectedMinimumClipCount>=100,"737 Quaternius animation library is a governed character-lane source profile");
    Check(CharacterAnimationLibrarySystem::IsNetworkSafeLocomotion(anim),"737 ordinary locomotion remains server-authoritative/non-root-motion safe");
    Check(CharacterAnimationLibrarySystem::ClassifyClipName("Rifle_Strafe_Run")==CharacterAnimationCategory::Combat,"737 animation classification recognizes combat clips before general locomotion");
    const auto retarget=CharacterAnimationLibrarySystem::BuildRetargetPlan(anim,1.8f,1.0f,scale);
    Check(retarget.valid&&Near(retarget.uniformScale,1.0f),"737 humanoid retarget plan uses canonical player scale");

    const auto bindA=ShipModuleInteriorLinkSystem::InferBinding(a,scale);
    auto engine=Hull("shipyard_a_main_engine",ShipyardModuleSemantic::MainEngine);engine.moduleClass=ShipyardModuleClass::Propulsion;
    const auto bindEngine=ShipModuleInteriorLinkSystem::InferBinding(engine,scale);
    Check(bindA.walkable&&bindA.capability==ExteriorInteriorCapability::WalkableRoom,"736 hull-capable exterior module receives walkable interior semantics");
    Check(bindEngine.interactionOnly&&bindEngine.capability==ExteriorInteriorCapability::ServiceAccess,"736 engine module exposes service interaction instead of inventing a walkable room");

    EncounterSpawnTable threats;threats.sectorId="test";threats.encounters.push_back({"raid","Pirate Raid",EncounterDisposition::Pirate,10,4,{"threat"}});
    EncounterDirectorState tense;tense.tension=.95;tense.daysSinceMajorThreat=4;tense.fleetPower=8;tense.crewCount=6;tense.economicWealth=12000;
    EncounterDirectorContext context;context.locationId="test.space";context.deepSpace=true;
    ProceduralEncounterGenerator generator;const auto directed=generator.GenerateDirectedEncounter(threats,tense,context,42);
    Check(directed.shouldSpawn&&directed.adjustedThreat>=4,"739 storyteller director scales a legal sandbox encounter from tension/power/wealth context");
    Check(!directed.reasons.empty(),"739 storyteller produces auditable encounter-selection reasoning");

    // Workspace normalization: existing model/PCG/world lanes remain alongside
    // the newly exposed interior/character/dev-world lanes.
    Check(builder.Activate(ShipyardBuilderCommand::WorkspaceModel)&&builder.Model().workspaceMode==ShipyardWorkspaceMode::Model,"735-744 existing MODEL lane remains available after cumulative workspace expansion");
    Check(builder.Activate(ShipyardBuilderCommand::WorkspacePcg)&&builder.Model().workspaceMode==ShipyardWorkspaceMode::Pcg,"735-744 existing PCG lane remains available after cumulative workspace expansion");
    Check(builder.Activate(ShipyardBuilderCommand::WorkspaceWorld)&&builder.Model().workspaceMode==ShipyardWorkspaceMode::World,"735-744 existing WORLD lane remains available after cumulative workspace expansion");

    auto controls=ShipyardBuilderSystem::BuildControls(builder.Model(),1920,1080);
    bool interiorButton=false,characterButton=false,devWorldButton=false;
    for(const auto& c:controls){interiorButton|=c.command==ShipyardBuilderCommand::WorkspaceInterior;characterButton|=c.command==ShipyardBuilderCommand::WorkspaceCharacter;devWorldButton|=c.command==ShipyardBuilderCommand::WorkspaceDevWorld;}
    Check(interiorButton&&characterButton&&devWorldButton,"735-744 visible Shipyard Dev Studio controls expose INTERIOR, CHARACTER and DEV WORLD");

    std::cout<<"Pass735-744 assertions: "<<passed<<" passed / "<<failed<<" failed\n";
    return failed==0?0:1;
}
