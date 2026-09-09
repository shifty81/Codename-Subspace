#include "ships/ShipPcgRuntimeClosureSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace subspace {
namespace {
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>& catalog,const std::string& id){
    for(const auto& r:catalog) if(r.source.moduleId==id) return &r;
    return nullptr;
}
float Dot(const Vector3&a,const Vector3&b){return a.x*b.x+a.y*b.y+a.z*b.z;}
float Len(const Vector3&v){return std::sqrt(std::max(0.0f,Dot(v,v)));}
Vector3 Norm(const Vector3&v){const float l=Len(v);return l>.0001f?Vector3{v.x/l,v.y/l,v.z/l}:Vector3{};}
float DistPointRaySegment(const Vector3&p,const Vector3&o,const Vector3&dir,float length){
    const Vector3 d=Norm(dir);const Vector3 op=p-o;const float t=std::clamp(Dot(op,d),0.0f,length);const Vector3 q=o+d*t;return Len(p-q);
}
UniversalSizeClass AdjacentLower(UniversalSizeClass s){int v=static_cast<int>(s);return static_cast<UniversalSizeClass>(std::max(0,v-1));}
UniversalSizeClass AdjacentHigher(UniversalSizeClass s){int v=static_cast<int>(s);return static_cast<UniversalSizeClass>(std::min(4,v+1));}
bool ParticipatesInAttachment(const ProceduralShipVisualRecipe& recipe,std::size_t moduleIndex){
    for(const auto& edge:recipe.attachments) if(edge.parentModuleIndex==moduleIndex||edge.childModuleIndex==moduleIndex) return true;
    return false;
}
const ShipyardAssemblySocket* FindSocket(const ShipyardModuleRecord& rec,const std::string& name){
    for(const auto& socket:rec.sockets) if(socket.name==name) return &socket;
    return nullptr;
}
bool RebuildChildFromCertifiedAttachment(const std::vector<ShipyardModuleRecord>& catalog,ProceduralShipVisualRecipe& recipe,std::size_t childIndex){
    if(childIndex>=recipe.modules.size()) return false;
    for(const auto& edge:recipe.attachments){
        if(edge.childModuleIndex!=childIndex||edge.parentModuleIndex>=recipe.modules.size()) continue;
        const auto* parent=Find(catalog,recipe.modules[edge.parentModuleIndex].moduleId);
        const auto* child=Find(catalog,recipe.modules[childIndex].moduleId);
        if(!parent||!child) return false;
        const auto* parentSocket=FindSocket(*parent,edge.parentSocket);
        const auto* childSocket=FindSocket(*child,edge.childSocket);
        if(!parentSocket||!childSocket) return false;
        const float scale=std::max(.01f,recipe.modules[childIndex].scaleX);
        using PlacementFn=VisualModulePlacement (*)(const VisualModulePlacement&,const ShipyardAssemblySocket&,const ShipyardModuleRecord&,const ShipyardAssemblySocket&,float);
        PlacementFn placementFn=&ShipyardModuleSystem::BuildAttachmentPlacement;
        recipe.modules[childIndex]=placementFn(recipe.modules[edge.parentModuleIndex],*parentSocket,*child,*childSocket,scale);
        return true;
    }
    return false;
}
}

std::vector<ExhaustClearanceVolume> ShipPcgRuntimeClosureSystem::BuildExhaustClearance(const std::vector<ShipyardModuleRecord>&catalog,
                                                                                       const ProceduralShipVisualRecipe&recipe,
                                                                                       float lengthMultiplier){
    std::vector<ExhaustClearanceVolume> out;
    const auto occupancy=ShipSpatialAssemblySystem::BuildOccupancy(catalog,recipe);
    for(std::size_t i=0;i<recipe.modules.size();++i){
        const auto* rec=Find(catalog,recipe.modules[i].moduleId);if(!rec)continue;
        const auto role=UniversalKitbashAuthority::InferPropulsionRole(*rec);if(role==PropulsionRole::None)continue;
        const auto axes=PropulsionRoleSystem::Infer(*rec);
        ExhaustClearanceVolume v;v.moduleIndex=i;v.origin={recipe.modules[i].x,recipe.modules[i].y,recipe.modules[i].z};
        v.direction=Norm(PropulsionRoleSystem::TransformDirection(recipe.modules[i],axes.localExhaustAxis));
        const float base=std::max({rec->source.halfWidth,rec->source.halfHeight,.18f});v.radius=std::max(.18f,base*.42f);
        v.length=std::max(1.0f,rec->source.halfLength*lengthMultiplier);
        for(const auto& b:occupancy){
            if(b.moduleIndex==i)continue;
            const float approxRadius=std::sqrt(b.halfExtents.x*b.halfExtents.x+b.halfExtents.z*b.halfExtents.z);
            const Vector3 rel=b.center-v.origin;
            const float along=Dot(rel,v.direction);
            if(along<=.05f||along>v.length)continue;
            const float d=DistPointRaySegment(b.center,v.origin,v.direction,v.length);
            if(d < v.radius+approxRadius*.62f){v.blocked=true;v.blockingModuleIndex=b.moduleIndex;break;}
        }
        out.push_back(v);
    }
    return out;
}

ShipGenerationCandidateReport ShipPcgRuntimeClosureSystem::EvaluateCandidate(const std::vector<ShipyardModuleRecord>&catalog,
                                                                              const ProceduralShipVisualRecipe&recipe,
                                                                              bool requireFunctionalCore,
                                                                              bool biologicalCrew){
    ShipGenerationCandidateReport report;
    report.spatial=ShipSpatialAssemblySystem::Validate(catalog,recipe,.42f);
    if(!report.spatial.valid){
        report.retrySuggested=true;
        if(report.spatial.commandBuried){report.reason=ShipPcgRejectReason::CommandBuried;report.messages.push_back("Command/cockpit exposure volume is buried by unrelated structure.");}
        else if(report.spatial.detailClusterCells>0){report.reason=ShipPcgRejectReason::DetailDensity;report.messages.push_back("Surface/detail density exceeds the certified regional budget.");}
        else {report.reason=ShipPcgRejectReason::SpatialConflict;report.messages.push_back("Unrelated module occupancy volumes intersect beyond mating allowance.");}
        return report;
    }
    report.exhaust=BuildExhaustClearance(catalog,recipe);
    for(const auto& v:report.exhaust) if(v.blocked){report.retrySuggested=true;report.reason=ShipPcgRejectReason::ExhaustBlocked;report.messages.push_back("Propulsion exhaust clearance intersects another ship module.");return report;}
    for(std::size_t i=0;i<recipe.modules.size();++i){
        const auto* rec=Find(catalog,recipe.modules[i].moduleId);if(!rec)continue;
        const auto role=UniversalKitbashAuthority::InferPropulsionRole(*rec);if(role==PropulsionRole::None)continue;
        const auto p=PropulsionRoleSystem::Validate(*rec,recipe.modules[i],role);
        if(!p.valid){report.retrySuggested=true;report.reason=ShipPcgRejectReason::PropulsionOrientation;report.messages.push_back(p.message.empty()?"Propulsion orientation is not certified for its requested role.":p.message);return report;}
    }
    if(requireFunctionalCore){report.functional=ShipFunctionalCoreSystem::Validate(catalog,recipe,biologicalCrew);if(!report.functional.valid){report.retrySuggested=true;report.reason=ShipPcgRejectReason::MissingFunctionalCore;report.messages=report.functional.messages;return report;}}
    report.accepted=true;report.retrySuggested=false;report.reason=ShipPcgRejectReason::None;return report;
}

bool ShipPcgRuntimeClosureSystem::RepairSpatialCandidate(const std::vector<ShipyardModuleRecord>&catalog,ProceduralShipVisualRecipe&recipe,int maxIterations){
    maxIterations=std::max(1,maxIterations);
    for(int iteration=0;iteration<maxIterations;++iteration){
        auto report=ShipSpatialAssemblySystem::Validate(catalog,recipe,.42f);
        if(report.valid)return true;
        bool changed=false;
        for(const auto pair:report.overlapPairs){
            if(pair.second>=recipe.modules.size())continue;
            // Never "repair" a socket-authored assembly by translating one
            // attached child away from its certified mating point.  Runtime
            // generation can reject/retry a candidate; authoring can correct
            // the socket.  Free/draft placements may still be nudged.
            if(ParticipatesInAttachment(recipe,pair.second)) continue;
            auto& p=recipe.modules[pair.second];const auto* r=Find(catalog,p.moduleId);if(!r)continue;
            if(r->moduleClass==ShipyardModuleClass::Command){p.y+=std::max(.5f,r->source.halfLength*.75f);p.z+=std::max(.18f,r->source.halfHeight*.25f);}
            else {const float sign=(p.x<0.0f?-1.0f:1.0f);p.x+=sign*std::max(.35f,r->source.halfWidth*.55f);}
            changed=true;
        }
        if(report.detailClusterCells>0){
            for(std::size_t i=0;i<recipe.modules.size();++i){const auto* r=Find(catalog,recipe.modules[i].moduleId);if(r&&(r->surfaceOnly||r->semantic==ShipyardModuleSemantic::SurfaceDetail)&&!ParticipatesInAttachment(recipe,i)){recipe.modules[i].x+=(i&1?1.0f:-1.0f)*.10f*static_cast<float>(1+iteration);recipe.modules[i].y+=.07f*static_cast<float>(i%3);changed=true;}}
        }
        if(!changed)break;
    }
    return ShipSpatialAssemblySystem::Validate(catalog,recipe,.42f).valid;
}

bool ShipPcgRuntimeClosureSystem::RepairCandidate(const std::vector<ShipyardModuleRecord>&catalog,
                                                   ProceduralShipVisualRecipe&recipe,
                                                   int maxIterations,
                                                   bool requireFunctionalCore,
                                                   bool biologicalCrew){
    maxIterations=std::max(1,maxIterations);
    for(int iteration=0;iteration<maxIterations;++iteration){
        auto report=EvaluateCandidate(catalog,recipe,requireFunctionalCore,biologicalCrew);
        if(report.accepted)return true;
        bool changed=false;
        if(report.reason==ShipPcgRejectReason::SpatialConflict||report.reason==ShipPcgRejectReason::CommandBuried||report.reason==ShipPcgRejectReason::DetailDensity){
            // Give the spatial repair lane the caller's full retry budget. A
            // two-step nudge can leave a draft partially separated while the
            // outer repair loop then mistakes the still-invalid return as
            // "unchanged" and exits early.
            changed=RepairSpatialCandidate(catalog,recipe,maxIterations);
        }else if(report.reason==ShipPcgRejectReason::PropulsionOrientation){
            for(std::size_t i=0;i<recipe.modules.size();++i){
                const auto* rec=Find(catalog,recipe.modules[i].moduleId);
                if(!rec)continue;const auto role=UniversalKitbashAuthority::InferPropulsionRole(*rec);if(role==PropulsionRole::None)continue;
                const auto before=PropulsionRoleSystem::Validate(*rec,recipe.modules[i],role);
                if(before.valid)continue;
                if(ParticipatesInAttachment(recipe,i)){
                    // First restore the child's exact certified mating transform.
                    // If that transform still fails role orientation the recipe
                    // itself needs another socket/candidate; rotating the child
                    // in-place would corrupt the assembly graph.
                    const auto original=recipe.modules[i];
                    if(RebuildChildFromCertifiedAttachment(catalog,recipe,i) && PropulsionRoleSystem::Validate(*rec,recipe.modules[i],role).valid){changed=true;break;}
                    recipe.modules[i]=original;
                    continue;
                }
                changed=PropulsionRoleSystem::ReorientForRole(*rec,recipe.modules[i],role);
                if(changed)break;
            }
        }else if(report.reason==ShipPcgRejectReason::ExhaustBlocked){
            for(const auto& v:report.exhaust){
                if(!v.blocked||v.moduleIndex>=recipe.modules.size())continue;
                // Moving an attached engine to clear exhaust breaks its socket
                // graph.  Leave that candidate intact and let the generator
                // retry another certified mating location.
                if(ParticipatesInAttachment(recipe,v.moduleIndex)) continue;
                auto& p=recipe.modules[v.moduleIndex];const auto* rec=Find(catalog,p.moduleId);if(!rec)continue;
                const auto role=UniversalKitbashAuthority::InferPropulsionRole(*rec);
                const float step=std::max(.35f,rec->source.halfLength*.55f);
                if(role==PropulsionRole::MainDrive){p.y-=step;}
                else if(role==PropulsionRole::RetroBrake){p.y+=step;}
                else if(role==PropulsionRole::VtolLift||role==PropulsionRole::VtolControl||role==PropulsionRole::LandingThruster){p.z-=step;}
                else if(role==PropulsionRole::LateralRcs){p.x+=(p.x<0.0f?-step:step);}
                else {p.z+=(p.z<0.0f?-step:step);}
                changed=true;break;
            }
        }else if(report.reason==ShipPcgRejectReason::MissingFunctionalCore){
            // Planning is deterministic, but attaching missing modules requires
            // available compatible sockets. Do not invent disconnected modules
            // in the repair layer; return review/failure to the generator.
            const auto plan=ShipFunctionalCoreSystem::BuildAutofitPlan(catalog,recipe,biologicalCrew);
            if(!plan.complete)return false;
            return false;
        }else{
            return false;
        }
        if(!changed)break;
    }
    return EvaluateCandidate(catalog,recipe,requireFunctionalCore,biologicalCrew).accepted;
}

std::vector<PropulsionCatalogAuditEntry> ShipPcgRuntimeClosureSystem::AuditPropulsionCatalog(const std::vector<ShipyardModuleRecord>&catalog){
    std::vector<PropulsionCatalogAuditEntry> out;
    for(const auto& rec:catalog){
        const auto role=UniversalKitbashAuthority::InferPropulsionRole(rec);if(role==PropulsionRole::None)continue;
        const auto axis=PropulsionRoleSystem::Infer(rec);PropulsionCatalogAuditEntry e;e.moduleId=rec.source.moduleId;e.role=role;e.confidence=axis.confidence;e.knownOrientation=axis.confidence>=.72f;e.generatorEligible=rec.generatorEligible&&e.knownOrientation&&!rec.sockets.empty();e.requiresReview=!e.generatorEligible;
        if(!e.knownOrientation)e.reason="THRUST_AXIS_REVIEW";else if(rec.sockets.empty())e.reason="SOCKET_REVIEW";else if(!rec.generatorEligible)e.reason="GENERATOR_DISABLED";else e.reason="CERTIFIED";out.push_back(e);
    }
    return out;
}

bool ShipPcgRuntimeClosureSystem::MaterialPcgEligible(KitbashMaterialCertification s){return s==KitbashMaterialCertification::Complete||s==KitbashMaterialCertification::NormalizedFallback;}

bool ShipPcgRuntimeClosureSystem::ModuleFitsClass(const ShipyardModuleRecord&module,ShipClass c,bool auxiliary){
    const auto target=ShipClassRoleSystem::Envelope(c).structuralSize;const auto profile=UniversalKitbashAuthority::BuildProfile(module,KitbashMaterialCertification::NormalizedFallback);
    if(!UniversalKitbashAuthority::SizeWithin(target,profile.morph.minimumSize,profile.morph.maximumSize)){
        if(!auxiliary)return false;
        const auto lower=AdjacentLower(target),higher=AdjacentHigher(target);
        return UniversalKitbashAuthority::SizeWithin(lower,profile.morph.minimumSize,profile.morph.maximumSize)||UniversalKitbashAuthority::SizeWithin(higher,profile.morph.minimumSize,profile.morph.maximumSize);
    }
    return true;
}

std::vector<std::size_t> ShipPcgRuntimeClosureSystem::FilterForClass(const std::vector<ShipyardModuleRecord>&catalog,ShipClass c,bool auxiliary){std::vector<std::size_t> out;for(std::size_t i=0;i<catalog.size();++i)if(ModuleFitsClass(catalog[i],c,auxiliary))out.push_back(i);return out;}

HullFamilyRuntimeProfile ShipPcgRuntimeClosureSystem::BuildHullFamilyProfile(const FactionHullFamilyDefinition&family){
    HullFamilyRuntimeProfile p;p.factionId=family.factionId;p.familyId=family.familyId;p.shipClass=family.shipClass;p.structuralSize=ShipClassRoleSystem::Envelope(family.shipClass).structuralSize;p.chassisStyle=family.chassisStyle;p.allowedRoles=family.allowedRoles;p.preferredRoles=family.preferredRoles;
    const auto e=ShipClassRoleSystem::Envelope(family.shipClass);p.targetLengthMeters=e.nominalLengthMeters;
    const float classWidth=(family.chassisStyle=="FAST_NARROW"?.22f:family.chassisStyle=="HEAVY_ARMORED"?.42f:family.chassisStyle=="MODULAR_UTILITY"?.38f:.32f);p.targetWidthMeters=e.nominalLengthMeters*classWidth;p.targetHeightMeters=e.nominalLengthMeters*(family.chassisStyle=="HEAVY_ARMORED"?.20f:.15f);
    p.commandExposure=family.chassisStyle=="HEAVY_ARMORED"?.18f:.27f;p.propulsionReserve=family.speedBias>1.1f?.28f:.20f;p.detailDensityBudget=family.chassisStyle=="MODULAR_UTILITY"?.28f:.20f;return p;
}

ShipRoleFitPlan ShipPcgRuntimeClosureSystem::BuildRoleFitPlan(const FactionHullFamilyDefinition&family,ShipRole role){
    ShipRoleFitPlan p;p.shipClass=family.shipClass;p.role=role;p.structuralSize=ShipClassRoleSystem::Envelope(family.shipClass).structuralSize;p.hullFamilyId=family.familyId;p.budget=ShipClassRoleSystem::RoleBudget(role);p.compatible=ShipClassRoleSystem::SupportsRole(family,role);p.mandatoryCapabilities=ShipFunctionalCoreSystem::Required(true);
    switch(role){case ShipRole::Scout:p.preferredModuleRoles={"SENSOR","PROPULSION","COMMUNICATIONS"};break;case ShipRole::ElectronicWarfare:p.preferredModuleRoles={"SENSOR","EW","POWER","UTILITY"};break;case ShipRole::Logistics:p.preferredModuleRoles={"REPAIR","SUPPLY","CARGO","UTILITY"};break;case ShipRole::Mining:p.preferredModuleRoles={"MINING","CARGO","REFINERY","UTILITY"};break;case ShipRole::Salvage:p.preferredModuleRoles={"SALVAGE","CARGO","UTILITY"};break;case ShipRole::Carrier:p.preferredModuleRoles={"HANGAR","DRONE","CARGO","DEFENSE"};break;case ShipRole::Boarding:p.preferredModuleRoles={"BOARDING","MARINE","BREACH","UTILITY"};break;case ShipRole::Siege:p.preferredModuleRoles={"HEAVY_WEAPON","POWER","THERMAL","ARMOR"};break;default:p.preferredModuleRoles={"WEAPON","ARMOR","SENSOR","PROPULSION"};break;}return p;
}

void ShipPcgRuntimeClosureSystem::ApplyLineage(ProceduralShipVisualRecipe&recipe,const HullFamilyRuntimeProfile&family,ShipRole role,const std::string&exemplarId){
    recipe.factionId=family.factionId;recipe.shipClassId=ShipClassRoleSystem::ClassName(family.shipClass);recipe.hullFamilyId=family.familyId;recipe.roleVariantId=ShipClassRoleSystem::RoleName(role);recipe.exemplarId=exemplarId;recipe.lineageAuthority="FACTION_CLASS_HULL_ROLE_V1";recipe.manufacturerFamily=family.factionId.empty()?recipe.manufacturerFamily:family.factionId;
}

} // namespace subspace
