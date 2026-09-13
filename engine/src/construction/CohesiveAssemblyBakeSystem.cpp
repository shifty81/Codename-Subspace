#include "construction/CohesiveAssemblyBakeSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>& c,const std::string& id){for(const auto& r:c)if(r.source.moduleId==id)return &r;return nullptr;}
const ShipyardAssemblySocket* Socket(const ShipyardModuleRecord& r,const std::string& id){for(const auto& s:r.sockets)if(s.name==id)return &s;return nullptr;}
bool InteriorCandidate(const ShipyardModuleRecord& r){switch(r.semantic){case ShipyardModuleSemantic::HullBow:case ShipyardModuleSemantic::HullMid:case ShipyardModuleSemantic::HullAft:case ShipyardModuleSemantic::CommandCockpit:case ShipyardModuleSemantic::CommandBridge:case ShipyardModuleSemantic::Adapter:return true;default:return r.partRole==ShipyardPartRole::Cargo||r.partRole==ShipyardPartRole::Hangar;}}
std::string SafeId(std::string id){for(char& c:id)if(!(c>='a'&&c<='z')&&!(c>='A'&&c<='Z')&&!(c>='0'&&c<='9')&&c!='_'&&c!='-')c='_';return id.empty()?"assembly":id;}
}

bool CohesiveAssemblyBakeSystem::ValidatePenetration(const VisualAssemblyAttachment& a,const ShipyardModuleRecord& parent,const ShipyardModuleRecord& child,const CohesiveAssemblyBakePolicy& p,float* allowed){
    const auto* ps=Socket(parent,a.parentSocket);const auto* cs=Socket(child,a.childSocket);if(!ps||!cs){if(allowed)*allowed=0.0f;return false;}
    const float socketAllowance=std::max(ps->insertionDepth,cs->insertionDepth);const float limit=std::min(p.maximumAttachmentPenetrationMeters,std::max(.02f,socketAllowance));if(allowed)*allowed=limit;return std::fabs(a.measuredGap)<=limit+.01f;
}

CohesiveAssemblyBakePlan CohesiveAssemblyBakeSystem::BuildPlan(const ProceduralShipVisualRecipe& recipe,const std::vector<ShipyardModuleRecord>& catalog,const WorldScaleProfile& scale){
    CohesiveAssemblyBakePlan out;out.sourceRecipeId=recipe.recipeId;out.outputAssetId="baked."+SafeId(recipe.recipeId);out.outputObjName=SafeId(recipe.recipeId)+"_cohesive.obj";out.policy=AuthoringStandardsSystem::DefaultBakePolicy(scale);
    if(recipe.modules.empty())out.errors.push_back("assembly has no modules to bake");
    for(std::size_t i=0;i<recipe.modules.size();++i){const auto* r=Find(catalog,recipe.modules[i].moduleId);if(!r){out.errors.push_back("bake references unknown module: "+recipe.modules[i].moduleId);continue;}out.elements.push_back({i,r->source.moduleId,true,InteriorCandidate(*r),true});}
    for(const auto& a:recipe.attachments){if(a.parentModuleIndex>=recipe.modules.size()||a.childModuleIndex>=recipe.modules.size()){out.errors.push_back("attachment index outside recipe during bake");continue;}const auto* p=Find(catalog,recipe.modules[a.parentModuleIndex].moduleId);const auto* c=Find(catalog,recipe.modules[a.childModuleIndex].moduleId);if(!p||!c)continue;float allowed=0;if(!ValidatePenetration(a,*p,*c,out.policy,&allowed))out.errors.push_back("attachment penetration/gap exceeds certified insertion envelope");out.portalCuts.push_back({a.parentModuleIndex,a.childModuleIndex,a.parentSocket,a.childSocket,allowed});}
    const float largestInteriorHeight=[&](){float h=0;for(const auto& e:out.elements)if(e.interiorVolumeCandidate){const auto* r=Find(catalog,e.moduleId);if(r)h=std::max(h,r->source.halfHeight*2.0f);}return h;}();
    out.characterScaleCompatible=largestInteriorHeight>=out.policy.minimumWalkableHeadroomMeters;
    if(!out.characterScaleCompatible)out.warnings.push_back("no candidate module currently provides canonical player headroom; bake requires interior carve expansion or non-walkable classification");
    out.readyForBooleanBake=out.errors.empty()&&!out.elements.empty();return out;
}

} // namespace subspace
