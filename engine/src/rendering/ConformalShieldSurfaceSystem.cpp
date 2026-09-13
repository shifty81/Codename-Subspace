#include "rendering/ConformalShieldSurfaceSystem.h"

#include <algorithm>

namespace subspace {
ShieldSurfacePolicy ConformalShieldSurfaceSystem::DefaultPolicy(){return {};}
ShieldSurfaceBudgetDecision ConformalShieldSurfaceSystem::Select(const ShieldSurfaceBudgetContext& c,const ShieldSurfacePolicy& p){
    ShieldSurfaceBudgetDecision d;const std::size_t count=std::max<std::size_t>(1,c.visibleShieldCount);
    if(c.localPlayer||c.editorPreview||(c.distanceMeters<3500.0f&&count<=24)){d.lod=ShieldSurfaceLod::NearConformal;d.rippleBudget=p.maxNearRipples;d.animateCalmWater=true;}
    else if(c.distanceMeters<15000.0f&&count<=80){d.lod=ShieldSurfaceLod::MidDecimated;d.rippleBudget=p.maxMidRipples;d.animateCalmWater=true;}
    else if(c.distanceMeters<60000.0f&&count<=180){d.lod=ShieldSurfaceLod::FarSilhouette;d.rippleBudget=p.maxFarRipples;d.animateCalmWater=false;}
    else {d.lod=ShieldSurfaceLod::StrategicOnly;d.rippleBudget=0;d.animateCalmWater=false;d.receivePointImpacts=false;d.triangleStride=std::max<std::size_t>(1,c.sourceTriangleCount);return d;}
    const std::size_t budget=d.lod==ShieldSurfaceLod::NearConformal?p.nearTriangleBudget:(d.lod==ShieldSurfaceLod::MidDecimated?p.midTriangleBudget:p.farTriangleBudget);d.triangleStride=std::max<std::size_t>(1,(c.sourceTriangleCount+std::max<std::size_t>(1,budget)-1)/std::max<std::size_t>(1,budget));return d;
}
const char* ConformalShieldSurfaceSystem::LodName(ShieldSurfaceLod l){switch(l){case ShieldSurfaceLod::NearConformal:return"NEAR_CONFORMAL";case ShieldSurfaceLod::MidDecimated:return"MID_DECIMATED";case ShieldSurfaceLod::FarSilhouette:return"FAR_SILHOUETTE";case ShieldSurfaceLod::StrategicOnly:return"STRATEGIC_ONLY";}return"NEAR_CONFORMAL";}
} // namespace subspace
