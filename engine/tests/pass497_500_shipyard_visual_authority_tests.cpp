#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardPartTaxonomySystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardOrientationConstraintSystem.h"
#include "ship_editor/ShipyardPaintZoneSystem.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;

namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}

VisualModuleSurfaceContact Surface(float x,float y,float z,float nx,float ny,float nz,float area){
    VisualModuleSurfaceContact c;c.point={x,y,z};c.normal={nx,ny,nz};c.supportingArea=area;c.confidence=1.0f;c.valid=true;return c;
}

ShipyardModuleRecord Wing149(){
    ShipyardModuleRecord r;
    r.source.moduleId="shipyard_a_wing_149_shipyard_wing_003_miscfinhanger";
    r.source.halfWidth=2.491647f*.5f;
    r.source.halfLength=7.111418f*.5f;
    r.source.halfHeight=6.254108f*.5f;
    r.source.portSurface=Surface(-r.source.halfWidth,0,0,-1,0,0,18);
    r.source.starboardSurface=Surface(r.source.halfWidth,0,0,1,0,0,18);
    r.source.forwardSurface=Surface(0,r.source.halfLength,0,0,1,0,24);
    r.source.aftSurface=Surface(0,-r.source.halfLength,0,0,-1,0,24);
    r.source.ventralSurface=Surface(0,0,-r.source.halfHeight,0,0,-1,794);
    r.source.dorsalSurface=Surface(0,0,r.source.halfHeight,0,0,1,60);
    r.moduleClass=ShipyardModuleSystem::Classify(r.source);
    r.semantic=ShipyardModuleSystem::SemanticClassify(r.source);
    r.size=ShipyardModuleSystem::SizeClassify(r.source);
    r.builderCategory=ShipyardPartTaxonomySystem::CategoryFor(r.moduleClass);
    r.partRole=ShipyardPartTaxonomySystem::RoleFor(r.semantic,r.source.moduleId);
    r.sockets=ShipyardModuleSystem::BuildSockets(r.source,r.semantic);
    r.preferredMountFace="ventral";
    r.mountFaceConfidence=1.0f;
    r.placementRole="LATERAL_STRUCTURE";
    r.pairedPlacement=true;
    return r;
}

ShipyardModuleRecord Hull(){
    ShipyardModuleRecord r;
    r.source.moduleId="shipyard_a_hull_094_shipyard_hull_001_test";
    r.source.halfWidth=3.0f;r.source.halfLength=5.0f;r.source.halfHeight=1.8f;
    r.source.portSurface=Surface(-3,0,0,-1,0,0,20);
    r.source.starboardSurface=Surface(3,0,0,1,0,0,20);
    r.source.forwardSurface=Surface(0,5,0,0,1,0,20);
    r.source.aftSurface=Surface(0,-5,0,0,-1,0,20);
    r.source.dorsalSurface=Surface(0,0,1.8f,0,0,1,20);
    r.source.ventralSurface=Surface(0,0,-1.8f,0,0,-1,20);
    r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::L;
    r.builderCategory=ShipyardPartCategory::Hull;r.partRole=ShipyardPartRole::PrimaryHull;r.primaryHull=true;r.generatorEligible=true;
    r.sockets=ShipyardModuleSystem::BuildSockets(r.source,r.semantic);
    return r;
}

const ShipyardAssemblySocket* Socket(const ShipyardModuleRecord& r,const std::string& name){
    for(const auto& s:r.sockets)if(s.name==name)return &s;return nullptr;
}
}

int main(){
    auto wing=Wing149();
    auto hull=Hull();

    Check(wing.moduleClass==ShipyardModuleClass::Wing,"wing149 remains in WING module class");
    Check(wing.semantic==ShipyardModuleSemantic::Wing,"wing149 has WING semantic");
    Check(wing.partRole==ShipyardPartRole::Wing,"miscfinhanger is a normal wing, not a vertical fin");

    const auto* mount=Socket(wing,"mount");
    Check(mount!=nullptr,"wing149 exposes certified root mount socket");
    if(mount){
        Check(mount->dirZ < -0.90f,"wing149 root uses user-verified ventral face");
        Check(std::fabs(mount->z + wing.source.halfHeight) < .01f,"wing149 root socket sits on ventral attachment face");
    }

    VisualModulePlacement upright;upright.moduleId=wing.source.moduleId;
    const auto bad=ShipyardOrientationConstraintSystem::Validate(wing,upright);
    Check(!bad.valid,"unrotated/upright wing149 fails horizontal-wing certification");

    const auto* starboard=Socket(hull,"starboard_mid");
    Check(starboard!=nullptr,"hull exposes starboard lateral socket");
    if(starboard&&mount){
        VisualModulePlacement parent;parent.moduleId=hull.source.moduleId;
        auto placed=ShipyardModuleSystem::BuildAttachmentPlacement(static_cast<const VisualModulePlacement&>(parent),*starboard,static_cast<const ShipyardModuleRecord&>(wing),*mount,1.0f);
        const auto good=ShipyardOrientationConstraintSystem::Validate(wing,placed);
        Check(good.valid,"root-authoritative socket placement lays wing149 horizontally");
        Check(std::fabs(placed.rollDegrees)>=80.0f || std::fabs(placed.pitchDegrees)>=80.0f,
              "wing149 receives required quarter-turn source-axis correction");
    }

    Check(ShipyardPaintZoneSystem::ForMaterial("Mat_Main")==ShipyardPaintZone::Primary,"Mat_Main maps to PRIMARY paint zone");
    Check(ShipyardPaintZoneSystem::ForMaterial("Mat_Seco")==ShipyardPaintZone::Secondary,"Mat_Seco maps to SECONDARY paint zone");
    Check(ShipyardPaintZoneSystem::ForMaterial("accent_stripe")==ShipyardPaintZone::Trim,"accent/stripe maps to TRIM paint zone");
    Check(ShipyardPaintZoneSystem::ForMaterial("Mat_Dark")==ShipyardPaintZone::StructuralDark,"Mat_Dark remains structural-dark zone");
    Check(ShipyardPaintZoneSystem::ForMaterial("Mat_MetalL")==ShipyardPaintZone::LightMetal,"light metal remains physical metal zone");
    Check(ShipyardPaintZoneSystem::ForMaterial("Mat_MetalD")==ShipyardPaintZone::DarkMetal,"dark metal remains physical metal zone");
    Check(ShipyardPaintZoneSystem::ForMaterial("Glow_Blu")==ShipyardPaintZone::EmissiveBlue,"blue glow remains emissive zone");
    Check(ShipyardPaintZoneSystem::ForMaterial("Glass")==ShipyardPaintZone::Glass,"glass remains non-paint glass zone");
    Check(ShipyardPaintZoneSystem::IsPaintable(ShipyardPaintZone::Primary),"primary is paintable");
    Check(!ShipyardPaintZoneSystem::IsPaintable(ShipyardPaintZone::Glass),"glass is not ordinary paint");

    ProceduralShipVisualRecipe recipe;recipe.role="INDUSTRIAL";recipe.seed=500;
    VisualModulePlacement hp;hp.moduleId=hull.source.moduleId;recipe.modules.push_back(hp);
    if(starboard&&mount){
        auto wp=ShipyardModuleSystem::BuildAttachmentPlacement(static_cast<const VisualModulePlacement&>(hp),*starboard,static_cast<const ShipyardModuleRecord&>(wing),*mount,1.0f);
        recipe.modules.push_back(wp);recipe.attachments.push_back({0,1,"starboard_mid","mount",0.0f,true});
    }
    ShipyardBuilderSystem builder;builder.Initialize({hull,wing},recipe);
    if(recipe.modules.size()>1){
        Check(builder.Activate(ShipyardBuilderCommand::SelectPlaced,1),"assembled-row selection succeeds");
        Check(builder.Model().selectedClass==ShipyardModuleClass::Wing,"assembled wing selection synchronizes left catalog class");
        const auto* selected=builder.SelectedCatalogModule();
        Check(selected&&selected->source.moduleId==wing.source.moduleId,"assembled wing selection synchronizes exact catalog module");
        builder.Activate(ShipyardBuilderCommand::InspectorAssembly);
        const auto controls=ShipyardBuilderSystem::BuildControls(builder.Model(),1848,900);
        bool friendly=false;
        for(const auto& c:controls)if(c.command==ShipyardBuilderCommand::SelectPlaced&&c.value==1&&c.label.find("Wing 149")!=std::string::npos)friendly=true;
        Check(friendly,"assembled module list exposes readable Wing 149 label");
    }

    // Explicitly invalid wing attachment must stop the builder from claiming PASS.
    auto invalidRecipe=recipe;
    if(invalidRecipe.modules.size()>1){
        invalidRecipe.modules[1]=upright;
        invalidRecipe.attachments[0].parentSocket="dorsal_mid";
        ShipyardBuilderSystem invalidBuilder;invalidBuilder.Initialize({hull,wing},invalidRecipe);
        const auto validation=invalidBuilder.Validate();
        Check(!validation.valid,"vertical/non-lateral wing can no longer return VALIDATION: PASS");
        bool hasWingError=false;for(const auto& e:validation.errors)if(e.find("Wing")!=std::string::npos||e.find("wing")!=std::string::npos)hasWingError=true;
        Check(hasWingError,"invalid wing reports an actionable validation error");
    }

    std::cout<<"Pass497-500 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
