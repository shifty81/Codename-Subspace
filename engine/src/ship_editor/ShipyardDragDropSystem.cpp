#include "ship_editor/ShipyardDragDropSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
ShipyardDragPreview ShipyardDragDropSystem::Begin(const ShipyardModuleRecord& child,
                                                   const std::vector<ShipyardModuleRecord>& catalog,
                                                   const ProceduralShipVisualRecipe& recipe){
    // Preserve the historical status vocabulary for callers that do not opt
    // into explicit XS-XL morph placement.  The size-aware overload is what
    // the live Shipyard now uses, while older regression/API consumers retain
    // the stable SNAP READY / FREE BUILD strings.
    auto preview=Begin(child,catalog,recipe,UniversalKitbashAuthority::FromShipyardSize(child.size));
    if(preview.snapped) preview.status=preview.valid?"SNAP READY":"COLLISION";
    else if(preview.freePlacement) preview.status="FREE BUILD / DRAG INTO VIEWPORT";
    return preview;
}

ShipyardDragPreview ShipyardDragDropSystem::Begin(const ShipyardModuleRecord& child,
                                                   const std::vector<ShipyardModuleRecord>& catalog,
                                                   const ProceduralShipVisualRecipe& recipe,
                                                   UniversalSizeClass targetSize){
    ShipyardDragPreview p;p.active=true;p.moduleId=child.source.moduleId;p.ghost.moduleId=child.source.moduleId;
    const auto profile=UniversalKitbashAuthority::BuildProfile(child,KitbashMaterialCertification::NormalizedFallback);
    p.requestedSize=targetSize;p.resolvedSize=UniversalKitbashAuthority::ClampSizeToMorph(profile,targetSize);
    p.sizeAdjusted=p.requestedSize!=p.resolvedSize;
    p.resolvedUniformScale=UniversalKitbashAuthority::SafeUniformScale(profile,p.resolvedSize);
    p.ghost.scaleX=p.ghost.scaleY=p.ghost.scaleZ=p.resolvedUniformScale;
    p.ghost.material=child.moduleClass==ShipyardModuleClass::Command?SpaceMaterialKind::Canopy:
        child.moduleClass==ShipyardModuleClass::Propulsion?SpaceMaterialKind::EngineHousing:
        child.moduleClass==ShipyardModuleClass::Hull?SpaceMaterialKind::IndustrialHull:SpaceMaterialKind::StructuralMetal;
    p.snapRadius=std::max(0.85f,std::max({child.source.halfWidth,child.source.halfLength,child.source.halfHeight})*1.75f);

    auto find=[&](const std::string& id)->const ShipyardModuleRecord*{for(const auto& r:catalog)if(r.source.moduleId==id)return &r;return nullptr;};
    for(std::size_t pi=0;pi<recipe.modules.size();++pi){
        const auto* parent=find(recipe.modules[pi].moduleId);if(!parent)continue;
        for(const auto& ps:parent->sockets)for(const auto& cs:child.sockets){
            if(!ShipyardModuleSystem::CanMate(ps.type,cs.type)&&!ShipyardModuleSystem::CanMate(cs.type,ps.type))continue;
            const bool mainDrive=child.semantic==ShipyardModuleSemantic::MainEngine||child.semantic==ShipyardModuleSemantic::EngineNozzle;
            const bool housing=child.semantic==ShipyardModuleSemantic::EngineHousing;
            if(housing&&!ShipyardModuleSystem::IsRearDriveSocketName(ps.name))continue;
            if(mainDrive&&parent->semantic!=ShipyardModuleSemantic::EngineHousing&&!ShipyardModuleSystem::IsRearDriveSocketName(ps.name))continue;
            if(mainDrive&&parent->semantic==ShipyardModuleSemantic::EngineHousing&&ps.name!="engine_cavity")continue;
            ShipyardSnapCandidate c;c.parentModuleIndex=pi;c.parentSocket=ps.name;c.childSocket=cs.name;
            c.placement=ShipyardModuleSystem::BuildAttachmentPlacement(recipe.modules[pi],ps,child,cs,p.resolvedUniformScale);
            const bool lateral=ps.name.find("port")!=std::string::npos||ps.name.find("starboard")!=std::string::npos;
            const bool structural=ps.type.find("STRUCT")!=std::string::npos||ps.type.find("HULL")!=std::string::npos||ps.type.find("struct")!=std::string::npos||ps.type.find("hull")!=std::string::npos;
            const bool functional=ps.type.find("FUNC")!=std::string::npos||ps.type.find("UTILITY")!=std::string::npos||ps.type.find("func")!=std::string::npos||ps.type.find("utility")!=std::string::npos;
            c.score=(lateral?2.0f:1.0f)+(structural?1.0f:0.0f)+(functional?.25f:0.0f);
            p.candidates.push_back(c);
        }
    }
    std::sort(p.candidates.begin(),p.candidates.end(),[](const auto&a,const auto&b){return a.score>b.score;});
    SelectBest(p);return p;
}
bool ShipyardDragDropSystem::SelectBest(ShipyardDragPreview& p){
    if(p.candidates.empty()){
        // A custom blueprint author is still allowed to carry the real part in
        // the cursor and free-place it. Production certification remains closed
        // until a real attachment graph is created.
        p.valid=true;p.selectedCandidate=-1;p.snapped=false;p.freePlacement=true;
        p.status=std::string("FREE BUILD / ")+UniversalKitbashAuthority::SizeName(p.resolvedSize)+(p.sizeAdjusted?" (SIZE CLAMPED)":"")+" / DRAG INTO VIEWPORT";return true;
    }
    p.selectedCandidate=0;p.ghost=p.candidates.front().placement;p.valid=!p.candidates.front().collisionRisk;p.snapped=true;p.freePlacement=false;
    p.status=p.valid?(std::string("SNAP READY / ")+UniversalKitbashAuthority::SizeName(p.resolvedSize)+(p.sizeAdjusted?" (SIZE CLAMPED)":"")):"COLLISION";return p.valid;
}
} // namespace subspace
