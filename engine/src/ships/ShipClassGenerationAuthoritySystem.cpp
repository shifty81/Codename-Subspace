#include "ships/ShipClassGenerationAuthoritySystem.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace subspace {
namespace {
int Rank(UniversalSizeClass size){return static_cast<int>(size);}
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>& catalog,const std::string& id){
    for(const auto& record:catalog)if(record.source.moduleId==id)return &record;
    return nullptr;
}

Vector3 RotatePoint(const Vector3& point,const VisualModulePlacement& p){
    constexpr float kDegToRad=3.14159265358979323846f/180.0f;
    const float yaw=p.yawDegrees*kDegToRad,pitch=p.pitchDegrees*kDegToRad,roll=p.rollDegrees*kDegToRad;
    const float cy=std::cos(yaw),sy=std::sin(yaw);
    const float cp=std::cos(pitch),sp=std::sin(pitch);
    const float cr=std::cos(roll),sr=std::sin(roll);
    // Match the established Shipyard transform convention: roll around Y,
    // pitch around X, yaw around Z. Exact world-space bounds must include
    // authored rotations; a planar Y-only estimate can massively overcorrect.
    const Vector3 r1{point.x*cr+point.z*sr,point.y,-point.x*sr+point.z*cr};
    const Vector3 r2{r1.x,r1.y*cp-r1.z*sp,r1.y*sp+r1.z*cp};
    return {r2.x*cy-r2.y*sy,r2.x*sy+r2.y*cy,r2.z};
}

float MaxInstanceScale(const ProceduralShipVisualRecipe& recipe){
    float out=0.0f;
    for(const auto& p:recipe.modules){
        out=std::max(out,std::max({std::fabs(p.scaleX),std::fabs(p.scaleY),std::fabs(p.scaleZ)}));
    }
    return out;
}
}

ShipGenerationBounds ShipClassGenerationAuthoritySystem::MeasureBoundsMeters(
    const ProceduralShipVisualRecipe& recipe,const std::vector<ShipyardModuleRecord>& catalog){
    ShipGenerationBounds out;
    if(recipe.modules.empty())return out;
    Vector3 minimum{std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};
    Vector3 maximum{-minimum.x,-minimum.y,-minimum.z};
    bool any=false;
    for(const auto& placement:recipe.modules){
        const auto* record=Find(catalog,placement.moduleId);if(!record)continue;
        const float hx=record->source.halfWidth*std::max(.001f,std::fabs(placement.scaleX));
        const float hy=record->source.halfLength*std::max(.001f,std::fabs(placement.scaleY));
        const float hz=record->source.halfHeight*std::max(.001f,std::fabs(placement.scaleZ));
        for(int sx:{-1,1})for(int sy:{-1,1})for(int sz:{-1,1}){
            const auto r=RotatePoint({hx*static_cast<float>(sx),hy*static_cast<float>(sy),hz*static_cast<float>(sz)},placement);
            const Vector3 world{placement.x+r.x,placement.y+r.y,placement.z+r.z};
            minimum.x=std::min(minimum.x,world.x);minimum.y=std::min(minimum.y,world.y);minimum.z=std::min(minimum.z,world.z);
            maximum.x=std::max(maximum.x,world.x);maximum.y=std::max(maximum.y,world.y);maximum.z=std::max(maximum.z,world.z);
            any=true;
        }
    }
    if(!any)return out;
    out.valid=true;out.minimum=minimum;out.maximum=maximum;out.size=maximum-minimum;
    return out;
}

float ShipClassGenerationAuthoritySystem::MeasureLengthMeters(const ProceduralShipVisualRecipe& recipe,
                                                               const std::vector<ShipyardModuleRecord>& catalog) {
    const auto bounds=MeasureBoundsMeters(recipe,catalog);
    return bounds.valid?std::max(0.0f,bounds.size.y):0.0f;
}

float ShipClassGenerationAuthoritySystem::TargetLengthMeters(ShipClass shipClass,
                                                              UniversalSizeClass requestedSize,
                                                              UniversalSizeClass* resolvedSize) {
    const auto component=ShipClassRoleSystem::ComponentProfile(shipClass);
    const auto resolved=ShipClassRoleSystem::ClampModuleSize(shipClass,requestedSize,false);
    if(resolvedSize)*resolvedSize=resolved;
    const int lo=Rank(component.minimumStructuralSize),hi=Rank(component.maximumStructuralSize),value=Rank(resolved);
    const float t=hi<=lo?.5f:std::clamp(static_cast<float>(value-lo)/static_cast<float>(hi-lo),0.0f,1.0f);
    const auto envelope=ShipClassRoleSystem::Envelope(shipClass);
    return envelope.minimumLengthMeters+(envelope.maximumLengthMeters-envelope.minimumLengthMeters)*t;
}

ShipClassGenerationReport ShipClassGenerationAuthoritySystem::Resolve(const ProceduralShipVisualRecipe& recipe,
                                                                       const std::vector<ShipyardModuleRecord>& catalog,
                                                                       ShipClass shipClass,
                                                                       UniversalSizeClass requestedSize) {
    ShipClassGenerationReport out;
    out.shipClass=shipClass;out.requestedSize=requestedSize;out.moduleCount=recipe.modules.size();
    out.targetLengthMeters=TargetLengthMeters(shipClass,requestedSize,&out.resolvedSize);
    const auto bounds=MeasureBoundsMeters(recipe,catalog);
    if(bounds.valid){out.measuredWidthMeters=bounds.size.x;out.measuredLengthMeters=bounds.size.y;out.measuredHeightMeters=bounds.size.z;}
    out.maximumInstanceScale=MaxInstanceScale(recipe);
    if(requestedSize!=out.resolvedSize)
        out.warnings.push_back(std::string("requested module size clamped to class envelope: ")+UniversalKitbashAuthority::SizeName(out.resolvedSize));
    if(out.measuredLengthMeters<=.001f)out.errors.push_back("generated hull has no measurable certified module length");
    else out.appliedScale=out.targetLengthMeters/out.measuredLengthMeters;

    const auto envelope=ShipClassRoleSystem::Envelope(shipClass);
    if(out.moduleCount<envelope.minimumModules)out.errors.push_back("generated hull is below selected class topology/module minimum");
    if(out.moduleCount>envelope.maximumModules)out.errors.push_back("generated hull exceeds selected class module budget");

    if(out.maximumInstanceScale>MaximumGeneratedInstanceScale){
        out.errors.push_back("generated recipe contains a module instance scaled beyond certified authoring limits");
        out.topologyRegenerationRequired=true;
    }
    if(out.appliedScale<MinimumFinalCorrectionScale||out.appliedScale>MaximumFinalCorrectionScale){
        out.errors.push_back("class target requires topology/module-family regeneration; whole-ship scale correction is forbidden");
        out.topologyRegenerationRequired=true;
    }

    if(bounds.valid&&out.measuredLengthMeters>.001f){
        const float widthRatio=out.measuredWidthMeters/out.measuredLengthMeters;
        const float heightRatio=out.measuredHeightMeters/out.measuredLengthMeters;
        // Extremely collapsed candidates are never promoted. These floors are
        // deliberately permissive: they reject pathological planes without
        // forcing every faction into the same broad silhouette.
        if(widthRatio<0.025f){out.errors.push_back("generated hull collapsed below minimum 3D width ratio");out.topologyRegenerationRequired=true;}
        if(heightRatio<0.015f){out.errors.push_back("generated hull collapsed below minimum 3D height ratio");out.topologyRegenerationRequired=true;}
    }

    out.valid=out.errors.empty();return out;
}

float ShipClassGenerationAuthoritySystem::CombinedScale(const ProceduralShipVisualRecipe& recipe,
                                                         const std::vector<ShipyardModuleRecord>& catalog,
                                                         ShipClass shipClass,
                                                         UniversalSizeClass requestedSize,
                                                         UniversalSizeClass* resolvedSize) {
    const auto report=Resolve(recipe,catalog,shipClass,requestedSize);
    if(resolvedSize)*resolvedSize=report.resolvedSize;
    return report.valid?report.appliedScale:1.0f;
}

void ShipClassGenerationAuthoritySystem::ApplyScale(ProceduralShipVisualRecipe& recipe,float uniformScale) {
    if(uniformScale<MinimumFinalCorrectionScale||uniformScale>MaximumFinalCorrectionScale)return;
    if(std::fabs(uniformScale-1.0f)<1.0e-5f)return;
    for(auto& p:recipe.modules){p.x*=uniformScale;p.y*=uniformScale;p.z*=uniformScale;p.scaleX*=uniformScale;p.scaleY*=uniformScale;p.scaleZ*=uniformScale;}
    for(auto& a:recipe.anchors){a.position.x*=uniformScale;a.position.y*=uniformScale;a.position.z*=uniformScale;}
    for(auto& h:recipe.hardpoints){h.position.x*=uniformScale;h.position.y*=uniformScale;h.position.z*=uniformScale;}
    recipe.widthScale*=uniformScale;recipe.lengthScale*=uniformScale;
}

ShipClassGenerationReport ShipClassGenerationAuthoritySystem::ApplyAndStamp(ProceduralShipVisualRecipe& recipe,
                                                                             const std::vector<ShipyardModuleRecord>& catalog,
                                                                             ShipClass shipClass,
                                                                             UniversalSizeClass requestedSize) {
    auto report=Resolve(recipe,catalog,shipClass,requestedSize);
    if(report.valid)ApplyScale(recipe,report.appliedScale);
    recipe.shipClassId=ShipClassRoleSystem::ClassName(shipClass);
    recipe.lineageAuthority="CLASS_SIZE_ENVELOPE_V3";
    recipe.runtimeCertificationMessage=report.valid?"CLASS_TOPOLOGY_ENVELOPE_CERTIFIED":"CLASS_TOPOLOGY_REGEN_REQUIRED";
    return report;
}

bool ShipClassGenerationAuthoritySystem::Validate(const ProceduralShipVisualRecipe& recipe,
                                                   const std::vector<ShipyardModuleRecord>& catalog,
                                                   ShipClass shipClass,
                                                   UniversalSizeClass requestedSize,
                                                   std::string* error) {
    const auto report=Resolve(recipe,catalog,shipClass,requestedSize);
    if(!recipe.shipClassId.empty()&&recipe.shipClassId!=ShipClassRoleSystem::ClassName(shipClass)){
        if(error)*error="recipe class lineage does not match selected ship class";
        return false;
    }
    if(!report.valid){if(error)*error=report.errors.empty()?"class generation envelope rejected recipe":report.errors.front();return false;}
    if(recipe.lineageAuthority=="CLASS_SIZE_ENVELOPE_V3"||recipe.lineageAuthority=="FACTION_CLASS_HULL_ROLE_V1"){
        const auto envelope=ShipClassRoleSystem::Envelope(shipClass);
        const float measured=report.measuredLengthMeters;
        if(measured<envelope.minimumLengthMeters*.96f||measured>envelope.maximumLengthMeters*1.04f){
            if(error)*error="generated ship length is outside selected class envelope";
            return false;
        }
    }
    return true;
}

} // namespace subspace
