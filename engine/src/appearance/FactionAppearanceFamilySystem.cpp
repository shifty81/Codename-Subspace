#include "appearance/FactionAppearanceFamilySystem.h"

#include <algorithm>

namespace subspace {
namespace {
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>& catalog,const std::string& id){for(const auto&r:catalog)if(r.source.moduleId==id)return &r;return nullptr;}
assets::SurfaceSemantic DefaultSurface(const ShipyardModuleRecord&r){
    switch(r.semantic){
    case ShipyardModuleSemantic::CommandCockpit:return assets::SurfaceSemantic::CanopyGlass;
    case ShipyardModuleSemantic::MainEngine:return assets::SurfaceSemantic::EngineHousing;
    case ShipyardModuleSemantic::EngineNozzle:return assets::SurfaceSemantic::Nozzle;
    case ShipyardModuleSemantic::RcsThruster:return assets::SurfaceSemantic::ThrusterEmissive;
    case ShipyardModuleSemantic::WeaponMount:case ShipyardModuleSemantic::TurretHardpoint:return assets::SurfaceSemantic::WeaponMetal;
    case ShipyardModuleSemantic::Sensor:return assets::SurfaceSemantic::Sensor;
    case ShipyardModuleSemantic::StructuralFrame:case ShipyardModuleSemantic::Adapter:return assets::SurfaceSemantic::StructuralMetal;
    case ShipyardModuleSemantic::SurfaceDetail:return assets::SurfaceSemantic::Trim;
    default:return assets::SurfaceSemantic::HullPrimary;
    }
}
ShipSpatialRegion SpatialForZone(ShipAppearanceZone z){
    switch(z){case ShipAppearanceZone::Bow:return ShipSpatialRegion::Bow;case ShipAppearanceZone::Command:return ShipSpatialRegion::Command;case ShipAppearanceZone::ForwardHull:return ShipSpatialRegion::ForwardHull;case ShipAppearanceZone::AftHull:return ShipSpatialRegion::AftHull;case ShipAppearanceZone::PortWing:return ShipSpatialRegion::Port;case ShipAppearanceZone::StarboardWing:return ShipSpatialRegion::Starboard;case ShipAppearanceZone::Dorsal:return ShipSpatialRegion::Dorsal;case ShipAppearanceZone::Ventral:return ShipSpatialRegion::Ventral;case ShipAppearanceZone::MainPropulsion:case ShipAppearanceZone::Maneuvering:case ShipAppearanceZone::VtolLanding:return ShipSpatialRegion::Propulsion;default:return ShipSpatialRegion::MidHull;}
}
}

FactionAppearanceFamily FactionAppearanceFamilySystem::Build(const std::string& faction,const std::string& variant){
    FactionAppearanceFamily f;f.factionId=faction;f.shipPresetId=faction+"_SHIP_"+variant;f.stationPresetId=faction+"_STATION_"+variant;f.planetaryPresetId=faction+"_PLANETARY_"+variant;f.weaponPresetId=faction+"_WEAPON_"+variant;f.shipPreset=KitbashAppearanceSystem::BuildFactionPreset(faction,variant);f.shipPreset.id=f.shipPresetId;return f;
}

ShipAppearanceZone FactionAppearanceFamilySystem::ZoneFor(const ShipyardModuleRecord&r,const VisualModulePlacement&p,const ProceduralShipVisualRecipe&recipe){
    switch(r.semantic){
    case ShipyardModuleSemantic::CommandCockpit:case ShipyardModuleSemantic::CommandBridge:return ShipAppearanceZone::Command;
    case ShipyardModuleSemantic::MainEngine:case ShipyardModuleSemantic::EngineHousing:case ShipyardModuleSemantic::EngineNozzle:return ShipAppearanceZone::MainPropulsion;
    case ShipyardModuleSemantic::RcsThruster:return (p.z<-.5f?ShipAppearanceZone::VtolLanding:ShipAppearanceZone::Maneuvering);
    case ShipyardModuleSemantic::WeaponMount:case ShipyardModuleSemantic::TurretHardpoint:return ShipAppearanceZone::Weapons;
    case ShipyardModuleSemantic::Sensor:return ShipAppearanceZone::Sensors;
    case ShipyardModuleSemantic::Wing:return p.x<0?ShipAppearanceZone::PortWing:ShipAppearanceZone::StarboardWing;
    case ShipyardModuleSemantic::StructuralFrame:case ShipyardModuleSemantic::Adapter:return ShipAppearanceZone::Structural;
    default:break;
    }
    switch(ShipSpatialAssemblySystem::ClassifyRegion(p,recipe)){case ShipSpatialRegion::Bow:return ShipAppearanceZone::Bow;case ShipSpatialRegion::ForwardHull:return ShipAppearanceZone::ForwardHull;case ShipSpatialRegion::AftHull:return ShipAppearanceZone::AftHull;case ShipSpatialRegion::Port:return ShipAppearanceZone::PortWing;case ShipSpatialRegion::Starboard:return ShipAppearanceZone::StarboardWing;case ShipSpatialRegion::Dorsal:return ShipAppearanceZone::Dorsal;case ShipSpatialRegion::Ventral:return ShipAppearanceZone::Ventral;default:return ShipAppearanceZone::MidHull;}
}

std::vector<SurfaceRegionBinding> FactionAppearanceFamilySystem::SegmentShip(const std::vector<ShipyardModuleRecord>&catalog,const ProceduralShipVisualRecipe&recipe,const AppearancePresetDefinition&preset){
    std::vector<SurfaceRegionBinding> out;out.reserve(recipe.modules.size());
    for(std::size_t i=0;i<recipe.modules.size();++i){const auto* r=Find(catalog,recipe.modules[i].moduleId);if(!r)continue;SurfaceRegionBinding b;b.moduleIndex=i;b.moduleId=r->source.moduleId;b.zone=ZoneFor(*r,recipe.modules[i],recipe);b.semantic=DefaultSurface(*r);b.channel=KitbashAppearanceSystem::Resolve(preset,SpatialForZone(b.zone),b.semantic);b.patternEligible=b.semantic!=assets::SurfaceSemantic::CanopyGlass&&b.semantic!=assets::SurfaceSemantic::Nozzle&&b.semantic!=assets::SurfaceSemantic::ThrusterEmissive;b.decalEligible=b.semantic!=assets::SurfaceSemantic::ThrusterEmissive&&b.semantic!=assets::SurfaceSemantic::Nozzle;b.weatheringEligible=b.semantic!=assets::SurfaceSemantic::ThrusterEmissive;out.push_back(b);}return out;
}

AppearancePresetDefinition FactionAppearanceFamilySystem::Inherit(const AppearancePresetDefinition&parent,const AppearancePresetDefinition&child){AppearancePresetDefinition result=parent;result.id=child.id.empty()?parent.id:child.id;result.parentId=parent.id;if(!child.rules.empty())for(const auto&rule:child.rules){auto it=std::find_if(result.rules.begin(),result.rules.end(),[&](const auto&x){return x.region==rule.region&&x.surface==rule.surface;});if(it==result.rules.end())result.rules.push_back(rule);else *it=rule;}result.hullPatternProjection=child.hullPatternProjection;result.localWarningProjection=child.localWarningProjection;return result;}

PatternProjectionMode FactionAppearanceFamilySystem::ProjectionFor(ShipAppearanceZone zone,bool readableText){if(readableText)return PatternProjectionMode::ModuleLocal;if(zone==ShipAppearanceZone::MainPropulsion||zone==ShipAppearanceZone::Maneuvering||zone==ShipAppearanceZone::VtolLanding||zone==ShipAppearanceZone::Weapons)return PatternProjectionMode::ModuleLocal;if(zone==ShipAppearanceZone::Structural)return PatternProjectionMode::Triplanar;return PatternProjectionMode::ShipSpace;}

} // namespace subspace
