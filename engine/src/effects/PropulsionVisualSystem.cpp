#include "effects/PropulsionVisualSystem.h"
#include <algorithm>
namespace subspace {
PropulsionVisualProfile PropulsionVisualSystem::Evaluate(ShipyardModuleSemantic semantic,float activity,PropulsionVisualLod lod,bool boost,bool vectorDrive){
    PropulsionVisualProfile p;activity=std::clamp(activity,0.0f,1.0f);if(activity<=.001f)return p;
    const bool rcs=semantic==ShipyardModuleSemantic::RcsThruster;
    p.nozzleGlow=.28f+.72f*activity;p.coreLength=(rcs?.22f:.48f)+activity*(rcs?.35f:1.10f);p.bodyLength=(rcs?.32f:.82f)+activity*(rcs?.48f:2.4f);p.haloWidth=(rcs?.7f:1.0f)+activity*.42f;p.flicker=.04f+.10f*activity;
    if(boost){p.coreLength*=1.35f;p.bodyLength*=1.55f;p.nozzleGlow=1.0f;}
    if(vectorDrive){p.bodyLength*=2.2f;p.haloWidth*=1.35f;p.ribbon=true;p.distortion=true;}
    if(lod==PropulsionVisualLod::Near){p.sparkBudget=rcs?4:12;p.ribbon=!rcs||p.ribbon;p.distortion=p.distortion||boost;}
    else if(lod==PropulsionVisualLod::Mid){p.sparkBudget=rcs?0:4;p.distortion=false;}
    else {p.sparkBudget=0;p.ribbon=false;p.distortion=false;p.bodyLength*=.62f;p.coreLength*=.72f;}
    return p;
}
} // namespace subspace
