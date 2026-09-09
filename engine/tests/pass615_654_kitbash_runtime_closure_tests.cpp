#include "appearance/FactionAppearanceFamilySystem.h"
#include "construction/UniversalConstructionSystem.h"
#include "content/KitbashReviewCatalogSystem.h"
#include "content/UniversalKitbashAuthority.h"
#include "editor/ProjectWideEditorNormalizationSystem.h"
#include "ship_editor/ShipyardDragDropSystem.h"
#include "ship_editor/ShipyardKitbashTransformSystem.h"
#include "ships/FactionShipDesignSystem.h"
#include "ships/PropulsionRoleSystem.h"
#include "ships/ShipFunctionalCoreSystem.h"
#include "ships/ShipPcgRuntimeClosureSystem.h"
#include "ships/ShipSpatialAssemblySystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(ok)std::cout<<"[PASS] "<<name<<"\n";else{++failures;std::cerr<<"[FAIL] "<<name<<"\n";}}
bool Near(float a,float b,float e=.001f){return std::fabs(a-b)<=e;}
ShipyardAssemblySocket Sock(std::string name,std::string type,float x,float y,float z,float dx,float dy,float dz){ShipyardAssemblySocket s;s.name=std::move(name);s.type=std::move(type);s.x=x;s.y=y;s.z=z;s.dirX=dx;s.dirY=dy;s.dirZ=dz;return s;}
ShipyardModuleRecord Mod(std::string id,ShipyardModuleClass cls,ShipyardModuleSemantic sem){ShipyardModuleRecord r;r.source.moduleId=std::move(id);r.source.halfWidth=.5f;r.source.halfLength=.6f;r.source.halfHeight=.4f;r.moduleClass=cls;r.semantic=sem;r.size=ShipyardModuleSize::M;r.generatorEligible=true;r.functional=cls!=ShipyardModuleClass::Detail;r.preferredMountFace="AFT";r.mountFaceConfidence=.95f;r.placementRole="MODULE";if(cls!=ShipyardModuleClass::Detail)r.sockets.push_back(Sock("mount","structural",0,-.6f,0,0,-1,0));return r;}
VisualModulePlacement P(const std::string&id,float x=0,float y=0,float z=0){VisualModulePlacement p;p.moduleId=id;p.x=x;p.y=y;p.z=z;return p;}
bool HasReview(const std::vector<KitbashReviewItem>&q,KitbashReviewKind k){return std::any_of(q.begin(),q.end(),[&](const auto&i){return i.kind==k;});}
}

int main(){
    auto hull=Mod("hull_mid",ShipyardModuleClass::Hull,ShipyardModuleSemantic::HullMid);
    auto hull2=Mod("hull_aft",ShipyardModuleClass::Hull,ShipyardModuleSemantic::HullAft);
    std::vector<ShipyardModuleRecord> spatialCatalog{hull,hull2};
    ProceduralShipVisualRecipe clustered;clustered.modules={P("hull_mid"),P("hull_aft")};
    auto reject=ShipPcgRuntimeClosureSystem::EvaluateCandidate(spatialCatalog,clustered);
    Check(!reject.accepted&&reject.reason==ShipPcgRejectReason::SpatialConflict,"Pass615 runtime PCG rejects whole-ship occupancy conflicts before catalog acceptance");
    auto repaired=clustered;Check(ShipPcgRuntimeClosureSystem::RepairCandidate(spatialCatalog,repaired,8),"Pass616 free/draft candidate repair can separate an otherwise overlapping assembly");

    auto cmd=Mod("command_cockpit",ShipyardModuleClass::Command,ShipyardModuleSemantic::CommandCockpit);
    std::vector<ShipyardModuleRecord> commandCatalog{hull,cmd};ProceduralShipVisualRecipe buried;buried.modules={P("hull_mid"),P("command_cockpit")};
    Check(ShipSpatialAssemblySystem::Validate(commandCatalog,buried).commandBuried,"Pass617 command/cockpit exposure is protected from unrelated enclosing structure");

    auto detail=Mod("surface_detail",ShipyardModuleClass::Detail,ShipyardModuleSemantic::SurfaceDetail);detail.surfaceOnly=true;detail.sockets.clear();
    std::vector<ShipyardModuleRecord> detailCatalog{detail};ProceduralShipVisualRecipe dense;for(int i=0;i<5;++i)dense.modules.push_back(P("surface_detail",.01f*i,0,0));
    Check(ShipSpatialAssemblySystem::Validate(detailCatalog,dense).detailClusterCells>0,"Pass618 coarse regional density budget rejects greeble clustering");
    ProceduralShipVisualRecipe recursive;recursive.modules={P("surface_detail"),P("surface_detail",1,0,0)};recursive.attachments.push_back({0,1,"a","b",0,true});
    Check(ShipSpatialAssemblySystem::Validate(detailCatalog,recursive).recursiveDetailAttachments>0,"Pass619 detail-on-detail recursive attachment is not ordinary PCG-certified");
    ProceduralShipVisualRecipe clean;clean.modules={P("hull_mid",0,-2,0),P("hull_aft",0,2,0)};
    Check(ShipPcgRuntimeClosureSystem::EvaluateCandidate(spatialCatalog,clean).accepted,"Pass620 non-conflicting whole-ship candidate reaches runtime acceptance lane");

    auto engine=Mod("main_engine",ShipyardModuleClass::Propulsion,ShipyardModuleSemantic::MainEngine);engine.sockets={Sock("mount","engine",0,.6f,0,0,1,0),Sock("exhaust","exhaust",0,-.6f,0,0,-1,0)};
    ProceduralShipVisualRecipe engineRecipe;engineRecipe.modules={P("main_engine",0,-3,0)};std::vector<ShipyardModuleRecord> engineCatalog{engine};
    auto exhaust=ShipPcgRuntimeClosureSystem::BuildExhaustClearance(engineCatalog,engineRecipe);
    Check(exhaust.size()==1&&exhaust[0].direction.y<-.9f,"Pass621 propulsion receives an explicit world-space exhaust clearance volume");
    auto blocker=Mod("blocking_hull",ShipyardModuleClass::Hull,ShipyardModuleSemantic::HullMid);std::vector<ShipyardModuleRecord> exhaustCatalog{engine,blocker};ProceduralShipVisualRecipe blocked;blocked.modules={P("main_engine",0,0,0),P("blocking_hull",0,-1.2f,0)};
    Check(ShipPcgRuntimeClosureSystem::EvaluateCandidate(exhaustCatalog,blocked).reason==ShipPcgRejectReason::ExhaustBlocked,"Pass622 exhaust corridors fail closed when another hull module blocks the plume");
    auto uncertain=Mod("rcs_unknown",ShipyardModuleClass::Propulsion,ShipyardModuleSemantic::RcsThruster);uncertain.sockets={Sock("mount","structural",0,0,0,1,0,0)};auto audit=ShipPcgRuntimeClosureSystem::AuditPropulsionCatalog({uncertain});
    Check(audit.size()==1&&audit[0].requiresReview&&!audit[0].generatorEligible,"Pass623 uncertain thruster axes route to review/manual placement instead of silent PCG");
    auto wrong=P("main_engine");wrong.yawDegrees=180;Check(!PropulsionRoleSystem::Validate(engine,wrong,PropulsionRole::MainDrive).valid&&PropulsionRoleSystem::ReorientForRole(engine,wrong,PropulsionRole::MainDrive)&&PropulsionRoleSystem::Validate(engine,wrong,PropulsionRole::MainDrive).valid,"Pass624 free propulsion placement can be reoriented into its certified functional role");

    Check(ShipPcgRuntimeClosureSystem::MaterialPcgEligible(KitbashMaterialCertification::Complete),"Pass625 complete material authority is eligible for ordinary PCG");
    Check(!ShipPcgRuntimeClosureSystem::MaterialPcgEligible(KitbashMaterialCertification::ReviewRequired),"Pass626 unresolved material authority is excluded from ordinary PCG");
    Check(ShipClassRoleSystem::Envelope(ShipClass::Cruiser).structuralSize==UniversalSizeClass::M,"Pass627 ship class resolves onto canonical XS-S-M-L-XL structural scale");
    auto commandProfile=UniversalKitbashAuthority::BuildProfile(cmd,KitbashMaterialCertification::Complete);Check(UniversalKitbashAuthority::ClampSizeToMorph(commandProfile,UniversalSizeClass::XL)==UniversalSizeClass::XL&&Near(UniversalKitbashAuthority::SafeUniformScale(commandProfile,UniversalSizeClass::XL),1.45f),"Pass628 morph-safe size selection clamps discrete functional families to certified scale bounds");
    auto structuralProfile=UniversalKitbashAuthority::BuildProfile(hull,KitbashMaterialCertification::Complete);auto drag=ShipyardDragDropSystem::Begin(hull,{hull},{},UniversalSizeClass::XL);
    Check(drag.resolvedSize==UniversalSizeClass::XL&&drag.ghost.scaleX>1.9f,"Pass629 cursor-carried ghost uses the requested derived XS-XL module scale");
    auto derived=ShipyardKitbashTransformSystem::DeriveSizeVariant(structuralProfile,P("hull_mid"),UniversalSizeClass::L);Check(derived.valid&&Near(derived.placement.scaleX,derived.placement.scaleY)&&Near(derived.placement.scaleY,derived.placement.scaleZ),"Pass630 derived size variants preserve uniform socket/material-safe baseline scale");
    Check(ShipPcgRuntimeClosureSystem::ModuleFitsClass(hull,ShipClass::Battleship,true),"Pass631 class filtering consumes the universal morph range instead of hard-coded module copies");

    auto dna=FactionShipDesignSystem::DefaultFactionDna("MERIDIAN");auto families=FactionShipDesignSystem::BuildClassFamilies("MERIDIAN",ShipClass::Cruiser,dna);
    Check(families.size()==4,"Pass632 every faction/class exposes four recognizable physical hull families");
    Check(families[0].chassisStyle!=families[2].chassisStyle,"Pass633 faction class families preserve distinct chassis philosophies instead of role-only skins");
    Check(std::count_if(families.begin(),families.end(),[](const auto&f){return ShipClassRoleSystem::SupportsRole(f,ShipRole::GeneralCombat);})>=2,"Pass634 multiple hull families may support the same specialized role with different chassis tradeoffs");
    std::vector<ShipyardModuleRecord> coreCatalog;
    auto bridge=Mod("bridge_life",ShipyardModuleClass::Command,ShipyardModuleSemantic::CommandBridge);bridge.placementRole="crew navigation communications life";coreCatalog.push_back(bridge);
    coreCatalog.push_back(engine);auto rcs=Mod("rcs",ShipyardModuleClass::Propulsion,ShipyardModuleSemantic::RcsThruster);coreCatalog.push_back(rcs);
    coreCatalog.push_back(hull);auto power=Mod("reactor_power",ShipyardModuleClass::Component,ShipyardModuleSemantic::Component);coreCatalog.push_back(power);auto thermal=Mod("radiator_thermal",ShipyardModuleClass::Component,ShipyardModuleSemantic::Component);coreCatalog.push_back(thermal);auto fuel=Mod("fuel_tank",ShipyardModuleClass::Component,ShipyardModuleSemantic::Component);coreCatalog.push_back(fuel);auto utility=Mod("utility_access",ShipyardModuleClass::Component,ShipyardModuleSemantic::Component);coreCatalog.push_back(utility);auto sensor=Mod("sensor_comm",ShipyardModuleClass::Component,ShipyardModuleSemantic::Sensor);coreCatalog.push_back(sensor);
    auto autofit=ShipFunctionalCoreSystem::BuildAutofitPlan(coreCatalog,{},true);Check(autofit.complete&&autofit.unresolved.empty(),"Pass635 mandatory functional-core autofit planning can resolve one implementation of every required ship capability");
    auto rolePlan=ShipPcgRuntimeClosureSystem::BuildRoleFitPlan(families[1],ShipRole::ElectronicWarfare);Check(rolePlan.compatible&&rolePlan.budget.sensors>1.5f&&!rolePlan.preferredModuleRoles.empty(),"Pass636 role fitting consumes real chassis budgets and preferred functional module families");
    Check(dna.factionId=="MERIDIAN"&&!dna.preferredStyleTags.empty(),"Pass637 faction DesignDNA deterministically establishes geometry/style preferences above hull families");
    auto runtimeFamily=ShipPcgRuntimeClosureSystem::BuildHullFamilyProfile(families[1]);ProceduralShipVisualRecipe lineage;ShipPcgRuntimeClosureSystem::ApplyLineage(lineage,runtimeFamily,ShipRole::ElectronicWarfare,"EW_EXEMPLAR");
    HullFamilyCompiledGrammar compiled;compiled.runtime=runtimeFamily;Check(FactionShipDesignSystem::VariantPreservesLineage(lineage,compiled,ShipRole::ElectronicWarfare),"Pass638 generated variants preserve faction/class/hull-family/role lineage authority");

    auto appearance=FactionAppearanceFamilySystem::Build("MERIDIAN","MILITARY");Check(appearance.shipPresetId.find("MERIDIAN_SHIP")!=std::string::npos&&appearance.stationPresetId.find("MERIDIAN_STATION")!=std::string::npos,"Pass639 one faction appearance family spans ships, stations, planetary and weapon domains");
    ProceduralShipVisualRecipe surfaceRecipe;surfaceRecipe.modules={P("command_cockpit",0,3,0),P("main_engine",0,-3,0)};std::vector<ShipyardModuleRecord> surfaceCatalog{cmd,engine};auto regions=FactionAppearanceFamilySystem::SegmentShip(surfaceCatalog,surfaceRecipe,appearance.shipPreset);
    Check(regions.size()==2&&regions[0].zone==ShipAppearanceZone::Command&&regions[1].zone==ShipAppearanceZone::MainPropulsion,"Pass640 appearance segmentation separates command and propulsion functional regions");
    AppearancePresetDefinition child;child.id="MERIDIAN_CUSTOM";child.rules.push_back({ShipSpatialRegion::Command,assets::SurfaceSemantic::HullPrimary,AppearanceChannel::Accent,true,true,true});auto inherited=FactionAppearanceFamilySystem::Inherit(appearance.shipPreset,child);
    Check(inherited.parentId==appearance.shipPreset.id&&inherited.id=="MERIDIAN_CUSTOM","Pass641 appearance presets support inheritance instead of duplicating complete liveries");
    Check(FactionAppearanceFamilySystem::ProjectionFor(ShipAppearanceZone::MidHull)==PatternProjectionMode::ShipSpace&&FactionAppearanceFamilySystem::ProjectionFor(ShipAppearanceZone::MainPropulsion)==PatternProjectionMode::ModuleLocal,"Pass642 pattern projection keeps hull-wide liveries coherent while functional modules remain locally oriented");

    Check(UniversalConstructionSystem::Domain(ConstructionWorkspaceMode::Planetary)==ConstructionDomain::Planetary&&std::string(UniversalConstructionSystem::ModeName(ConstructionWorkspaceMode::Weapon))=="WEAPONS","Pass643 shared construction mode selector drives universal ship/station/planetary/weapon editor domains");
    auto stationMorph=UniversalConstructionSystem::StationMorphProfile(UniversalSizeClass::M,true);Check(stationMorph.policy==KitbashScalingPolicy::StructuralFree&&stationMorph.maxLengthScale>=5.0f,"Pass644 station structural kitbash uses the same XS-XL authority with domain-specific safe morph rules");
    UniversalKitbashProfile planetaryProfile;planetaryProfile.assetId="refinery_factory";Check(UniversalConstructionSystem::InferPlanetaryRole(planetaryProfile,planetaryProfile.assetId)==PlanetaryModuleRole::Refinery,"Pass645 planetary kitbash classification recognizes factory/refinery functional roles");
    auto requiredPlanetary=UniversalConstructionSystem::RequiredPlanetaryCapabilities(PlanetaryModuleRole::Refinery);Check(std::find(requiredPlanetary.begin(),requiredPlanetary.end(),PlanetaryFacilityCapability::Production)!=requiredPlanetary.end()&&std::find(requiredPlanetary.begin(),requiredPlanetary.end(),PlanetaryFacilityCapability::LogisticsAccess)!=requiredPlanetary.end(),"Pass646 planetary facilities require production/storage/logistics capabilities rather than decorative labels");
    auto turret=Mod("turret_base_barrel",ShipyardModuleClass::Hardpoint,ShipyardModuleSemantic::WeaponMount);auto turretProfile=UniversalKitbashAuthority::BuildProfile(turret,KitbashMaterialCertification::Complete);Check(UniversalConstructionSystem::NormalizeWeaponRole(turretProfile)==WeaponAssemblyRole::CompleteTurret,"Pass647 weapon construction mode recognizes complete turret kitbash instead of generic mounts only");

    ProceduralShipVisualRecipe bpRecipe;bpRecipe.recipeId="test_ship";bpRecipe.factionId="MERIDIAN";bpRecipe.hullFamilyId=families[1].familyId;bpRecipe.roleVariantId="ELECTRONIC WARFARE";bpRecipe.modules={P("hull_mid")};bpRecipe.acceptedByArtDirector=true;auto bp=UniversalConstructionSystem::FromShipRecipe(bpRecipe,{hull},"MERIDIAN_EW_CRUISER");
    Check(bp.domain==ConstructionBlueprintDomain::Ship&&bp.certified&&bp.familyId==families[1].familyId,"Pass648 ship recipes promote into one universal ConstructionBlueprint authority");
    auto invalidBp=bp;invalidBp.attachments.push_back({0,9,"x","y",0,true});Check(!UniversalConstructionSystem::ValidateBlueprint(invalidBp).valid,"Pass649 blueprint certification fails closed on invalid cross-domain attachment references");
    auto smallRecipe=UniversalConstructionSystem::BuildManufacturingRecipe(structuralProfile,ConstructionDomain::Ship,UniversalSizeClass::XS,1.0f);auto bigRecipe=UniversalConstructionSystem::BuildManufacturingRecipe(structuralProfile,ConstructionDomain::Ship,UniversalSizeClass::XL,1.0f);Check(bigRecipe.estimatedMass>smallRecipe.estimatedMass&&bigRecipe.manufacturingHours>smallRecipe.manufacturingHours,"Pass650 manufacturing cost/time derive from canonical size/geometry rather than arbitrary class bonuses");
    auto partial=UniversalConstructionSystem::AnalyzeCapturedAsset("foreign_engine","ALIEN","ALIEN_BP",.75f);auto full=UniversalConstructionSystem::AnalyzeCapturedAsset("foreign_engine","ALIEN","ALIEN_BP",1.0f);Check(!partial.manufacturingUnlocked&&full.manufacturingUnlocked&&full.sourceFactionId=="ALIEN","Pass651 captured kitbash retains provenance and requires completed reverse engineering before manufacturing unlock");

    auto uncertainReview=uncertain;uncertainReview.semantic=ShipyardModuleSemantic::Component;uncertainReview.partRole=ShipyardPartRole::ReviewRequired;auto queue=KitbashReviewCatalogSystem::BuildReviewQueue({uncertainReview},KitbashMaterialCertification::ReviewRequired);Check(HasReview(queue,KitbashReviewKind::Classification)&&HasReview(queue,KitbashReviewKind::Material),"Pass652 uncertain kitbash routes into one blocking Authoring review queue");
    auto catalogs=KitbashReviewCatalogSystem::Materialize({hull,turret},KitbashMaterialCertification::Complete);Check(std::find(catalogs.ship.begin(),catalogs.ship.end(),"hull_mid")!=catalogs.ship.end()&&std::find(catalogs.weapon.begin(),catalogs.weapon.end(),"turret_base_barrel")!=catalogs.weapon.end(),"Pass653 runtime catalogs materialize certified assets by universal domain roles");
    auto cert=KitbashReviewCatalogSystem::Certify({hull,turret},KitbashMaterialCertification::Complete);Check(cert.total==2&&cert.quarantined==0&&cert.fullGateReady,"Pass654 Full Gate kitbash certification produces a deterministic fail-closed runtime summary");

    auto normalization=ProjectWideEditorNormalizationSystem::Audit();Check(normalization.runtimeNormalized&&!normalization.repositoryLayoutNormalized&&normalization.entries.size()>=27,"Pass615-654 normalization audit confirms one active runtime authority while retaining explicit deletion-aware repository migration debt");

    std::cout<<"Pass615-654 assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
