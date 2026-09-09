#include "ship_editor/ShipyardOrientationConstraintSystem.h"
#include "content/ShipyardAuthoredOrientation.generated.h"
#include "content/ShipyardNameClassification.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {

float Fold180(float v) {
    v=std::fmod(v,360.0f);
    if(v>180.0f)v-=360.0f;
    if(v<-180.0f)v+=360.0f;
    return v;
}

Vector3 RotateLocal(const Vector3& v,const VisualModulePlacement& p) {
    constexpr float kDegToRad=3.14159265358979323846f/180.0f;
    const float yaw=p.yawDegrees*kDegToRad;
    const float pitch=p.pitchDegrees*kDegToRad;
    const float roll=p.rollDegrees*kDegToRad;
    const float cr=std::cos(roll),sr=std::sin(roll);
    const float x1=v.x*cr+v.z*sr,y1=v.y,z1=-v.x*sr+v.z*cr;
    const float cp=std::cos(pitch),sp=std::sin(pitch);
    const float x2=x1,y2=y1*cp-z1*sp,z2=y1*sp+z1*cp;
    const float cy=std::cos(yaw),sy=std::sin(yaw);
    return {x2*cy-y2*sy,x2*sy+y2*cy,z2};
}

Vector3 ProjectedHalfExtents(const ShipyardModuleRecord& m,const VisualModulePlacement& p) {
    const float hx=std::max(.001f,m.source.halfWidth*std::fabs(p.scaleX));
    const float hy=std::max(.001f,m.source.halfLength*std::fabs(p.scaleY));
    const float hz=std::max(.001f,m.source.halfHeight*std::fabs(p.scaleZ));
    const auto x=RotateLocal({1,0,0},p);
    const auto y=RotateLocal({0,1,0},p);
    const auto z=RotateLocal({0,0,1},p);
    return {
        std::fabs(x.x)*hx+std::fabs(y.x)*hy+std::fabs(z.x)*hz,
        std::fabs(x.y)*hx+std::fabs(y.y)*hy+std::fabs(z.y)*hz,
        std::fabs(x.z)*hx+std::fabs(y.z)*hy+std::fabs(z.z)*hz
    };
}

} // namespace

ShipyardOrientationRule ShipyardOrientationConstraintSystem::RuleFor(const ShipyardModuleRecord& m) {
    ShipyardOrientationRule r;
    if(m.semantic==ShipyardModuleSemantic::Wing) {
        // A normal WING is a lateral lifting/armor plane regardless of legacy
        // Greyoxide leaf names containing "fin". Only taxonomy-verified Fin
        // roles may stand vertically.
        const auto inferredRole=ShipyardPartTaxonomySystem::RoleFor(m.semantic,m.source.moduleId);
        const bool explicitFin=m.partRole==ShipyardPartRole::Fin ||
                               (m.partRole==ShipyardPartRole::Unknown && inferredRole==ShipyardPartRole::Fin);
        r.allowVertical=explicitFin;
        r.matchShipForward=!explicitFin;
        r.matchShipDorsal=!explicitFin;
        r.maximumRollDegrees=explicitFin?95.0f:32.0f;
        r.mirrorRecommended=true;
    }
    if(m.moduleClass==ShipyardModuleClass::Command ||
       m.semantic==ShipyardModuleSemantic::MainEngine ||
       m.semantic==ShipyardModuleSemantic::EngineHousing) {
        r.matchShipForward=true;
        r.maximumRollDegrees=35.0f;
    }
    return r;
}

ShipyardOrientationValidation ShipyardOrientationConstraintSystem::Validate(const ShipyardModuleRecord& m,const VisualModulePlacement& p) {
    ShipyardOrientationValidation v;
    const auto r=RuleFor(m);
    if(m.semantic==ShipyardModuleSemantic::Wing && !r.allowVertical) {
        // Euler-roll alone cannot identify an upright Greyoxide wing because
        // source parts use different local root axes. Validate the transformed
        // geometry envelope instead. A certified lateral wing must read much
        // flatter vertically than its dominant in-plane extent and must have a
        // meaningful lateral footprint.
        const auto e=ProjectedHalfExtents(m,p);
        const float plane=std::max(e.x,e.y);
        const bool tooTall=e.z>plane*.76f;
        const bool noLateralRead=e.x<e.z*.58f;
        if(tooTall || noLateralRead) {
            v.valid=false;
            v.warning="Wing is vertically oriented; root face must mate laterally and the wing plane must remain horizontal";
            return v;
        }

        // User-certified source-basis metadata also closes the remaining
        // 180-degree ambiguity that a flat envelope cannot detect. This catches
        // a wing that is attached and horizontal but has its small/leading end
        // facing aft or its broad presentation face upside-down.
        if(const auto* authored=FindShipyardAuthoredOrientation(ShipyardNameClassifier::CanonicalLeafName(m.source.moduleId))){
            const float mirror=p.mirrorX?-1.0f:1.0f;
            const auto forward=RotateLocal({authored->forwardX*mirror,authored->forwardY,authored->forwardZ},p).normalized();
            const auto up=RotateLocal({authored->upX*mirror,authored->upY,authored->upZ},p).normalized();
            if(forward.y<.35f){
                v.valid=false;
                v.warning="Wing leading edge faces aft; rotate the certified forward edge toward ship forward";
                return v;
            }
            if(up.z<.35f){
                v.valid=false;
                v.warning="Wing broad presentation face is flipped; rotate the certified dorsal face upward";
                return v;
            }
        }
    }
    return v;
}

void ShipyardOrientationConstraintSystem::Normalize(const ShipyardModuleRecord& m,VisualModulePlacement& p) {
    const auto r=RuleFor(m);
    // Socket/root authority is responsible for the initial quarter-turn of
    // oddly-authored source wings. Preserve such a turn when it already makes
    // the transformed envelope valid. For a transform that actually produces
    // an invalid vertical normal wing, clamp only the offending manual roll so
    // the editor remains compatible with its original bounded-correction path.
    if(m.semantic==ShipyardModuleSemantic::Wing && !r.allowVertical) {
        const auto before=Validate(m,p);
        if(!before.valid && std::fabs(Fold180(p.rollDegrees))>r.maximumRollDegrees) {
            const float folded=Fold180(p.rollDegrees);
            p.rollDegrees=std::clamp(folded,-r.maximumRollDegrees,r.maximumRollDegrees);
        }
    }
    if(r.matchShipForward) {
        const float yaw=Fold180(p.yawDegrees);
        if(std::fabs(yaw)<7.5f)p.yawDegrees=0.0f;
    }
}

} // namespace subspace
