#include "ship_editor/ShipyardKitbashTransformSystem.h"

#include <algorithm>
#include <queue>
#include <unordered_set>

namespace subspace {
namespace { int Rank(UniversalSizeClass s){return static_cast<int>(s);} }

VisualModulePlacement ShipyardKitbashTransformSystem::MirrorPlacementX(const VisualModulePlacement&p){ConstructionSymmetryFrame f;f.axis=ConstructionSymmetryAxis::PortStarboard;return ConstructionSymmetrySystem::ReflectPlacement(p,f);}
ShipyardAssemblySocket ShipyardKitbashTransformSystem::MirrorSocketX(const ShipyardAssemblySocket&s){return ConstructionSymmetrySystem::ReflectSocket(s,ConstructionSymmetryAxis::PortStarboard);}
std::string ShipyardKitbashTransformSystem::MirrorLateralName(const std::string&n){return ConstructionSymmetrySystem::ReflectSocketName(n,ConstructionSymmetryAxis::PortStarboard);}
VisualModulePlacement ShipyardKitbashTransformSystem::MirrorPlacement(const VisualModulePlacement&p,const ConstructionSymmetryFrame&f){return ConstructionSymmetrySystem::ReflectPlacement(p,f);}
ShipyardAssemblySocket ShipyardKitbashTransformSystem::MirrorSocket(const ShipyardAssemblySocket&s,ConstructionSymmetryAxis a){return ConstructionSymmetrySystem::ReflectSocket(s,a);}

bool ShipyardKitbashTransformSystem::MirrorRecipeSubtree(ProceduralShipVisualRecipe&recipe,std::size_t root,const ConstructionSymmetryFrame&frame){
    if(root>=recipe.modules.size())return false;
    std::unordered_set<std::size_t> selected;std::queue<std::size_t> q;q.push(root);selected.insert(root);
    while(!q.empty()){const auto i=q.front();q.pop();for(const auto&a:recipe.attachments)if(a.parentModuleIndex==i&&a.childModuleIndex<recipe.modules.size()&&selected.insert(a.childModuleIndex).second)q.push(a.childModuleIndex);}
    for(const auto i:selected)recipe.modules[i]=MirrorPlacement(recipe.modules[i],frame);
    for(auto&a:recipe.attachments){
        if(selected.count(a.parentModuleIndex))a.parentSocket=ConstructionSymmetrySystem::ReflectSocketName(a.parentSocket,frame.axis);
        if(selected.count(a.childModuleIndex))a.childSocket=ConstructionSymmetrySystem::ReflectSocketName(a.childSocket,frame.axis);
    }
    return true;
}
bool ShipyardKitbashTransformSystem::MirrorRecipeSubtreeX(ProceduralShipVisualRecipe&recipe,std::size_t root){ConstructionSymmetryFrame f;f.axis=ConstructionSymmetryAxis::PortStarboard;return MirrorRecipeSubtree(recipe,root,f);}

DerivedKitbashVariant ShipyardKitbashTransformSystem::DeriveSizeVariant(const UniversalKitbashProfile&profile,const VisualModulePlacement&source,UniversalSizeClass target){
    DerivedKitbashVariant out;out.sourceAssetId=profile.assetId;out.size=target;out.placement=source;out.placement.moduleId=source.moduleId.empty()?profile.assetId:source.moduleId;
    if(!UniversalKitbashAuthority::SizeWithin(target,profile.morph.minimumSize,profile.morph.maximumSize)){out.message="Target size is outside certified morph range";return out;}
    if(profile.morph.policy==KitbashScalingPolicy::FixedReference&&target!=profile.referenceSize){out.message="Fixed-reference module cannot derive another size";return out;}
    const float uniform=UniversalKitbashAuthority::SafeUniformScale(profile,target);
    out.placement.scaleX*=uniform;out.placement.scaleY*=uniform;out.placement.scaleZ*=uniform;
    out.variantId=profile.assetId+"@"+UniversalKitbashAuthority::SizeName(target);
    out.valid=true;out.message="Derived canonical size variant";return out;
}

} // namespace subspace
