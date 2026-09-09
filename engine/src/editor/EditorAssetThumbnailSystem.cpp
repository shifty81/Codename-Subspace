#include "editor/EditorAssetThumbnailSystem.h"

#include <cmath>
#include <sstream>

namespace subspace {
namespace {
EditorThumbnailViewPreset ViewFor(const ShipyardModuleRecord& r){
    if(r.moduleClass==ShipyardModuleClass::Propulsion)return EditorThumbnailViewPreset::Axial;
    if(r.semantic==ShipyardModuleSemantic::Sensor||r.semantic==ShipyardModuleSemantic::CommandBridge)return EditorThumbnailViewPreset::Vertical;
    return EditorThumbnailViewPreset::ThreeQuarter;
}
}

std::string EditorAssetThumbnailSystem::AxisLabel(const Vector3& a){
    const float ax=std::fabs(a.x),ay=std::fabs(a.y),az=std::fabs(a.z);
    if(ax>=ay&&ax>=az)return a.x>=0.0f?"+X":"-X";
    if(ay>=ax&&ay>=az)return a.y>=0.0f?"+Y":"-Y";
    return a.z>=0.0f?"+Z":"-Z";
}

const char* EditorAssetThumbnailSystem::ReviewName(EditorThumbnailReviewState s){
    switch(s){
        case EditorThumbnailReviewState::Certified:return "PCG";
        case EditorThumbnailReviewState::ManualOnly:return "MANUAL";
        case EditorThumbnailReviewState::Review:return "REVIEW";
    }
    return "REVIEW";
}

EditorAssetThumbnailRecord EditorAssetThumbnailSystem::Build(const ShipyardModuleRecord& r,int materialRevision,int previewRecipeVersion){
    EditorAssetThumbnailRecord out;
    const auto profile=UniversalKitbashAuthority::BuildProfile(r,KitbashMaterialCertification::NormalizedFallback);
    out.assetId=r.source.moduleId;
    out.previewKey=r.source.moduleId+"::canonical-preview";
    out.viewPreset=ViewFor(r);
    out.sizeBadge=UniversalKitbashAuthority::SizeName(profile.referenceSize);
    out.mirrorSupported=profile.morph.mirrorX;
    out.morphSupported=profile.morph.policy!=KitbashScalingPolicy::FixedReference;
    if(UniversalKitbashAuthority::IsPcgEligible(profile))out.reviewState=EditorThumbnailReviewState::Certified;
    else if(r.generatorEligible)out.reviewState=EditorThumbnailReviewState::Review;
    else out.reviewState=EditorThumbnailReviewState::ManualOnly;
    out.certificationBadge=ReviewName(out.reviewState);
    const auto axes=PropulsionRoleSystem::Infer(r);
    out.propulsion=axes.role!=PropulsionRole::None;
    if(out.propulsion){
        out.localThrustAxis=axes.localThrustAxis;out.localExhaustAxis=axes.localExhaustAxis;
        out.thrustLabel=AxisLabel(axes.localThrustAxis);out.exhaustLabel=AxisLabel(axes.localExhaustAxis);
    }
    std::ostringstream key;key<<out.assetId<<"|S="<<out.sizeBadge<<"|M="<<materialRevision<<"|P="<<previewRecipeVersion
                              <<"|R="<<out.certificationBadge<<"|V="<<static_cast<int>(out.viewPreset);
    out.cacheKey=key.str();
    return out;
}

} // namespace subspace
