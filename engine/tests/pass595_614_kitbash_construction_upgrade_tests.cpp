#include "appearance/KitbashAppearanceSystem.h"
#include "content/KitbashIntakeSystem.h"
#include "content/UniversalKitbashAuthority.h"
#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardDragDropSystem.h"
#include "ship_editor/ShipyardKitbashTransformSystem.h"
#include "ships/PropulsionRoleSystem.h"
#include "ships/ShipClassRoleSystem.h"
#include "ships/ShipFunctionalCoreSystem.h"
#include "ships/ShipSpatialAssemblySystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;

namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}
bool Near(float a,float b,float eps=.001f){return std::fabs(a-b)<=eps;}

ShipyardAssemblySocket Socket(std::string name,std::string type,float x,float y,float z,float dx,float dy,float dz){
    ShipyardAssemblySocket s;s.name=std::move(name);s.type=std::move(type);s.x=x;s.y=y;s.z=z;s.dirX=dx;s.dirY=dy;s.dirZ=dz;return s;
}
ShipyardModuleRecord Module(std::string id,ShipyardModuleClass cls,ShipyardModuleSemantic sem){
    ShipyardModuleRecord r;r.source.moduleId=std::move(id);r.source.halfWidth=.5f;r.source.halfLength=1.0f;r.source.halfHeight=.5f;
    r.moduleClass=cls;r.semantic=sem;r.size=ShipyardModuleSize::M;r.generatorEligible=true;r.functional=cls!=ShipyardModuleClass::Detail;
    r.preferredMountFace="AFT";r.mountFaceConfidence=.95f;
    if(cls!=ShipyardModuleClass::Detail)r.sockets.push_back(Socket("mount","structural",0,-1,0,0,-1,0));
    return r;
}
VisualModulePlacement Placed(const std::string& id,float x=0,float y=0,float z=0){VisualModulePlacement p;p.moduleId=id;p.x=x;p.y=y;p.z=z;return p;}
}

int main(){
    // Pass595: true mirror is reflection, not a hidden 180-degree flip.
    VisualModulePlacement a=Placed("antenna",3,4,2);a.yawDegrees=30;a.pitchDegrees=10;a.rollDegrees=15;
    const auto mirrored=ShipyardKitbashTransformSystem::MirrorPlacementX(a);
    Check(Near(mirrored.x,-3)&&Near(mirrored.y,4)&&mirrored.mirrorX,"Pass595 true mirror reflects geometry across ship-local X");
    Check(Near(mirrored.yawDegrees,-30)&&Near(mirrored.pitchDegrees,10)&&Near(mirrored.rollDegrees,-15),"Pass595 mirror conjugates orientation instead of yaw-flipping 180 degrees");
    auto port=Socket("port_antenna","structural",-2,0,0,-1,0,0);const auto star=ShipyardKitbashTransformSystem::MirrorSocketX(port);
    Check(Near(star.x,2)&&Near(star.dirX,1)&&star.name.find("starboard")!=std::string::npos,"Pass595 sockets and lateral naming mirror with the asset");

    // Pass596-597: inventory dragging always has a real ghost; snapping is optional.
    auto child=Module("wing_child",ShipyardModuleClass::Wing,ShipyardModuleSemantic::Wing);
    std::vector<ShipyardModuleRecord> loneCatalog{child};ProceduralShipVisualRecipe emptyRecipe;
    auto freeDrag=ShipyardDragDropSystem::Begin(child,loneCatalog,emptyRecipe);
    Check(freeDrag.active&&freeDrag.ghost.moduleId=="wing_child"&&freeDrag.freePlacement,"Pass596 cursor drag carries the actual module even without a snap target");
    auto parent=Module("hull_parent",ShipyardModuleClass::Hull,ShipyardModuleSemantic::HullMid);parent.sockets={Socket("hull_aft","hull_aft",1,0,0,1,0,0)};
    child.sockets={Socket("hull_forward","hull_forward",-.5f,0,0,-1,0,0)};
    std::vector<ShipyardModuleRecord> snapCatalog{parent,child};ProceduralShipVisualRecipe snapRecipe;snapRecipe.modules.push_back(Placed("hull_parent"));
    auto snapDrag=ShipyardDragDropSystem::Begin(child,snapCatalog,snapRecipe);
    Check(!snapDrag.candidates.empty(),"Pass597 smart placement exposes compatible socket candidates before commit");

    // Pass598-600: reusable variants preserve source identity and safe scale policy.
    auto structural=Module("structural_truss",ShipyardModuleClass::Hull,ShipyardModuleSemantic::StructuralFrame);
    auto structuralProfile=UniversalKitbashAuthority::BuildProfile(structural,KitbashMaterialCertification::Complete);
    Check(structuralProfile.morph.policy==KitbashScalingPolicy::StructuralFree,"Pass599 structural kitbash receives scalable morph policy");
    auto xl=ShipyardKitbashTransformSystem::DeriveSizeVariant(structuralProfile,Placed("structural_truss"),UniversalSizeClass::XL);
    Check(xl.valid&&xl.variantId.find("@XL")!=std::string::npos&&xl.sourceAssetId=="structural_truss","Pass600 derived size variants retain provenance to the source CanonicalAsset");

    // Pass601-603: propulsion roles are semantic and axis-driven.
    auto engine=Module("main_engine_130",ShipyardModuleClass::Propulsion,ShipyardModuleSemantic::MainEngine);
    engine.sockets={Socket("main_exhaust","exhaust",0,-1,0,0,-1,0),Socket("aft_mount","engine",0,1,0,0,1,0)};
    Check(UniversalKitbashAuthority::InferPropulsionRole(engine)==PropulsionRole::MainDrive,"Pass601 main drive is a classified propulsion role");
    auto vtol=Module("vtol_lift_thruster",ShipyardModuleClass::Propulsion,ShipyardModuleSemantic::RcsThruster);
    Check(UniversalKitbashAuthority::InferPropulsionRole(vtol)==PropulsionRole::VtolLift,"Pass601 scaled maneuver/VTOL thrusters retain explicit role taxonomy");
    const auto axis=PropulsionRoleSystem::Infer(engine);
    Check(axis.localExhaustAxis.y<-.9f&&axis.localThrustAxis.y>.9f,"Pass602 nozzle exhaust and thrust axes are opposite authorities");
    auto goodMain=PropulsionRoleSystem::Validate(engine,Placed("main_engine_130",0,-4,0),PropulsionRole::MainDrive);
    Check(goodMain.valid&&goodMain.shipThrustAxis.y>.7f,"Pass603 correctly oriented aft engine certifies as forward main thrust");

    // Pass604: weapon kitbash differentiates hardpoints and actual turret assemblies.
    auto completeTurret=Module("large_turret_base_barrel_housing",ShipyardModuleClass::Hardpoint,ShipyardModuleSemantic::WeaponMount);
    Check(UniversalKitbashAuthority::InferWeaponRole(completeTurret)==WeaponAssemblyRole::CompleteTurret,"Pass604 weapon mount classification recognizes complete turret geometry");

    // Pass605-606: whole-ship spatial validation exists above socket validity.
    auto hull=Module("spatial_hull",ShipyardModuleClass::Hull,ShipyardModuleSemantic::HullMid);
    auto cockpit=Module("spatial_cockpit",ShipyardModuleClass::Command,ShipyardModuleSemantic::CommandCockpit);
    std::vector<ShipyardModuleRecord> spatialCatalog{hull,cockpit};ProceduralShipVisualRecipe separated;
    separated.modules={Placed("spatial_hull",0,0,0),Placed("spatial_cockpit",0,3,0)};
    Check(ShipSpatialAssemblySystem::Validate(spatialCatalog,separated).valid,"Pass605 separated structural/command assembly passes whole-ship occupancy validation");
    ProceduralShipVisualRecipe clustered=separated;clustered.modules[1].x=0;clustered.modules[1].y=0;clustered.modules[1].z=0;
    auto clusteredReport=ShipSpatialAssemblySystem::Validate(spatialCatalog,clustered);
    Check(!clusteredReport.valid&&clusteredReport.unintendedOverlapPairs>0,"Pass606 unrelated modules clustered into one volume fail spatial certification");

    // Pass607-611: semantic appearance is independent from source texture filenames.
    const auto preset=KitbashAppearanceSystem::BuildFactionPreset("TEST_FACTION","MILITARY");
    Check(KitbashAppearanceSystem::Resolve(preset,ShipSpatialRegion::Command,assets::SurfaceSemantic::HullPrimary)==AppearanceChannel::Secondary,"Pass607 command-region appearance can resolve differently from generic primary hull");
    Check(KitbashAppearanceSystem::Resolve(preset,ShipSpatialRegion::Propulsion,assets::SurfaceSemantic::Nozzle)==AppearanceChannel::Functional,"Pass608 functional surface segmentation preserves nozzle material identity");
    Check(preset.hullPatternProjection==PatternProjectionMode::ShipSpace,"Pass610 faction appearance uses ship-space pattern authority across neighboring kitbash modules");
    Check(!KitbashAppearanceSystem::MirrorsDecalGlyphs(true)&&KitbashAppearanceSystem::MirrorsDecalGlyphs(false),"Pass611 mirror appearance preserves readable text while geometric markings may reflect");

    // Pass609/612-614: governed intake/certification and project-wide class/role normalization.
    auto certified=KitbashIntakeSystem::Classify("greyoxide_shipyard_v07","v0.7-certified-r5",structural,KitbashMaterialCertification::Complete);
    Check(certified.state==KitbashCertificationState::Certified,"Pass609 complete material/orientation/socket asset can enter certified PCG catalog");
    auto broken=KitbashIntakeSystem::Classify("broken","r1",structural,KitbashMaterialCertification::BrokenDependency);
    Check(broken.state==KitbashCertificationState::Quarantined,"Pass612 broken source/material dependency fails closed into quarantine");

    const auto frigate=ShipClassRoleSystem::Envelope(ShipClass::Frigate);const auto battleship=ShipClassRoleSystem::Envelope(ShipClass::Battleship);
    Check(frigate.structuralSize==UniversalSizeClass::XS&&Near(frigate.minimumLengthMeters,40)&&Near(frigate.maximumLengthMeters,90),"Pass613 frigates normalize to XS 40-90m structural envelope");
    Check(battleship.structuralSize==UniversalSizeClass::XL&&Near(battleship.minimumLengthMeters,450)&&Near(battleship.maximumLengthMeters,750),"Pass613 battleships normalize to XL 450-750m structural envelope");
    const auto cruiserFamilies=ShipClassRoleSystem::BuildDefaultHullFamilies("TEST_FACTION",ShipClass::Cruiser);
    Check(cruiserFamilies.size()==4,"Pass613 every faction/class receives four physical hull design families");
    Check(std::count_if(cruiserFamilies.begin(),cruiserFamilies.end(),[](const auto& family){return ShipClassRoleSystem::SupportsRole(family,ShipRole::GeneralCombat);})>=2,"Pass613 hull families can support overlapping role-specialized variants");

    const auto required=ShipFunctionalCoreSystem::Required(true);
    Check(required.size()>=12&&std::find(required.begin(),required.end(),ShipFunctionalCapability::MainPropulsion)!=required.end(),"Pass614 production ship certification declares a mandatory functional core");

    std::cout<<"Pass595-614 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
