#include "developer/ShipyardDevWorldSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

const char* ShipyardDevWorldSystem::BackdropName(DevWorldBackdrop b){switch(b){case DevWorldBackdrop::Stars:return "STARS";case DevWorldBackdrop::Checkerboard:return "CHECKERBOARD";case DevWorldBackdrop::NeutralStudio:return "NEUTRAL STUDIO";case DevWorldBackdrop::Hangar:return "HANGAR";case DevWorldBackdrop::PlanetSurface:return "PLANET SURFACE";case DevWorldBackdrop::Nebula:return "NEBULA";case DevWorldBackdrop::Black:return "BLACK";case DevWorldBackdrop::White:return "WHITE";}return "CHECKERBOARD";}
const char* ShipyardDevWorldSystem::ZoneName(DevWorldZoneKind k){switch(k){case DevWorldZoneKind::PlayerScale:return "PLAYER SCALE";case DevWorldZoneKind::AnimationViewer:return "ANIMATION VIEWER";case DevWorldZoneKind::KitbashCatalog:return "KITBASH CATALOG";case DevWorldZoneKind::CertifiedShips:return "CERTIFIED SHIPS";case DevWorldZoneKind::InteriorKit:return "INTERIOR KIT";case DevWorldZoneKind::PcgProvingGround:return "PCG PROVING GROUND";case DevWorldZoneKind::Terraforming:return "TERRAFORMING";case DevWorldZoneKind::LightingMaterials:return "LIGHTING / MATERIALS";}return "DEV";}

ShipyardDevWorldState ShipyardDevWorldSystem::CreateDefault(const WorldScaleProfile& scale){
    ShipyardDevWorldState s;s.enabled=true;s.backdrop=DevWorldBackdrop::Checkerboard;s.checkerCellMeters=scale.shipFoundationMeters;s.playerSpawn={0,0,0};
    const float spacing=96.0f;
    const DevWorldZoneKind kinds[]={DevWorldZoneKind::PlayerScale,DevWorldZoneKind::AnimationViewer,DevWorldZoneKind::KitbashCatalog,DevWorldZoneKind::CertifiedShips,DevWorldZoneKind::InteriorKit,DevWorldZoneKind::PcgProvingGround,DevWorldZoneKind::Terraforming,DevWorldZoneKind::LightingMaterials};
    for(std::size_t i=0;i<sizeof(kinds)/sizeof(kinds[0]);++i){DevWorldZone z;z.kind=kinds[i];z.id="dev.zone."+std::to_string(i);z.displayName=ZoneName(kinds[i]);z.origin={float(i%4)*spacing-1.5f*spacing,float(i/4)*spacing,0};z.sizeMeters={80,80,24};s.zones.push_back(z);}return s;
}

void ShipyardDevWorldSystem::CycleBackdrop(ShipyardDevWorldState& s,int direction){constexpr int count=8;int v=static_cast<int>(s.backdrop);v=(v+(direction>=0?1:-1)+count)%count;s.backdrop=static_cast<DevWorldBackdrop>(v);}

std::vector<DevWorldAssetPedestal> ShipyardDevWorldSystem::LayoutKitbashCatalog(const std::vector<ShipyardModuleRecord>& catalog,const Vector3& origin,const WorldScaleProfile& scale,bool certifiedOnly){
    std::vector<DevWorldAssetPedestal> out;const float spacing=std::max(3.0f,scale.referencePlayerHeightMeters*2.2f);const int columns=12;int n=0;
    for(const auto&r:catalog){if(certifiedOnly&&!r.generatorEligible)continue;DevWorldAssetPedestal p;p.assetId=r.source.moduleId;p.label=ShipyardPartTaxonomySystem::DisplayName(r);p.position={origin.x+float(n%columns)*spacing,origin.y+float(n/columns)*spacing,origin.z};p.certified=r.generatorEligible&&!r.source.moduleId.empty();p.reviewRequired=!r.generatorEligible||r.partRole==ShipyardPartRole::ReviewRequired;p.uniformScale=1.0f;out.push_back(p);++n;}return out;
}

std::vector<DevWorldShipPad> ShipyardDevWorldSystem::LayoutCertifiedShips(const std::vector<ProceduralShipVisualRecipe>& recipes,const Vector3& origin,const WorldScaleProfile& scale){
    std::vector<DevWorldShipPad> out;const float spacing=std::max(48.0f,scale.shipFoundationMeters*16.0f);int n=0;for(const auto&r:recipes){if(r.modules.empty())continue;DevWorldShipPad p;p.recipeId=r.recipeId;p.displayName=r.recipeId;p.position={origin.x+float(n%5)*spacing,origin.y+float(n/5)*spacing,origin.z};p.headingDegrees=0;p.certified=true;out.push_back(p);++n;}return out;
}

} // namespace subspace
