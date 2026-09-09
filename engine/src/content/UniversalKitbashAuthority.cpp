#include "content/UniversalKitbashAuthority.h"

#include <algorithm>
#include <cctype>

namespace subspace {
namespace {
int Rank(UniversalSizeClass s){return static_cast<int>(s);}
std::string Lower(std::string s){for(char& c:s)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return s;}
bool Has(const std::string& s,const char* token){return s.find(token)!=std::string::npos;}
}

UniversalSizeClass UniversalKitbashAuthority::FromShipyardSize(ShipyardModuleSize size){
    switch(size){
    case ShipyardModuleSize::XS:return UniversalSizeClass::XS;
    case ShipyardModuleSize::S:return UniversalSizeClass::S;
    case ShipyardModuleSize::M:return UniversalSizeClass::M;
    case ShipyardModuleSize::L:return UniversalSizeClass::L;
    case ShipyardModuleSize::XL:return UniversalSizeClass::XL;
    }
    return UniversalSizeClass::M;
}
ShipyardModuleSize UniversalKitbashAuthority::ToShipyardSize(UniversalSizeClass size){
    switch(size){
    case UniversalSizeClass::XS:return ShipyardModuleSize::XS;
    case UniversalSizeClass::S:return ShipyardModuleSize::S;
    case UniversalSizeClass::M:return ShipyardModuleSize::M;
    case UniversalSizeClass::L:return ShipyardModuleSize::L;
    case UniversalSizeClass::XL:return ShipyardModuleSize::XL;
    }
    return ShipyardModuleSize::M;
}
const char* UniversalKitbashAuthority::SizeName(UniversalSizeClass size){
    switch(size){case UniversalSizeClass::XS:return "XS";case UniversalSizeClass::S:return "S";case UniversalSizeClass::M:return "M";case UniversalSizeClass::L:return "L";case UniversalSizeClass::XL:return "XL";}return "M";
}
float UniversalKitbashAuthority::NominalScale(UniversalSizeClass size){
    switch(size){case UniversalSizeClass::XS:return .48f;case UniversalSizeClass::S:return .72f;case UniversalSizeClass::M:return 1.0f;case UniversalSizeClass::L:return 1.45f;case UniversalSizeClass::XL:return 2.05f;}return 1.0f;
}
bool UniversalKitbashAuthority::SizeWithin(UniversalSizeClass value,UniversalSizeClass minimum,UniversalSizeClass maximum){return Rank(value)>=Rank(minimum)&&Rank(value)<=Rank(maximum);}
UniversalSizeClass UniversalKitbashAuthority::ClampSizeToMorph(const UniversalKitbashProfile& profile,UniversalSizeClass requested){
    const int lo=Rank(profile.morph.minimumSize), hi=Rank(profile.morph.maximumSize);
    const int v=std::clamp(Rank(requested),lo,hi);
    if(profile.morph.policy==KitbashScalingPolicy::FixedReference)return profile.referenceSize;
    return static_cast<UniversalSizeClass>(v);
}
float UniversalKitbashAuthority::SafeUniformScale(const UniversalKitbashProfile& profile,UniversalSizeClass target){
    target=ClampSizeToMorph(profile,target);
    const float ratio=NominalScale(target)/std::max(.001f,NominalScale(profile.referenceSize));
    const float lo=std::max({profile.morph.minLengthScale,profile.morph.minWidthScale,profile.morph.minHeightScale});
    const float hi=std::min({profile.morph.maxLengthScale,profile.morph.maxWidthScale,profile.morph.maxHeightScale});
    if(profile.morph.policy==KitbashScalingPolicy::FixedReference)return 1.0f;
    return std::clamp(ratio,lo,std::max(lo,hi));
}

PropulsionRole UniversalKitbashAuthority::InferPropulsionRole(const ShipyardModuleRecord& record){
    const std::string n=Lower(record.source.moduleId+" "+record.placementRole+" "+record.preferredMountFace);
    if(record.semantic==ShipyardModuleSemantic::MainEngine||record.semantic==ShipyardModuleSemantic::EngineNozzle)return PropulsionRole::MainDrive;
    if(record.semantic!=ShipyardModuleSemantic::RcsThruster)return PropulsionRole::None;
    if(Has(n,"vtol")||Has(n,"lift"))return PropulsionRole::VtolLift;
    if(Has(n,"landing"))return PropulsionRole::LandingThruster;
    if(Has(n,"retro")||Has(n,"brake"))return PropulsionRole::RetroBrake;
    if(Has(n,"vertical")||Has(n,"dorsal")||Has(n,"ventral"))return PropulsionRole::VerticalRcs;
    if(Has(n,"pitch"))return PropulsionRole::PitchControl;
    if(Has(n,"yaw"))return PropulsionRole::YawControl;
    if(Has(n,"roll"))return PropulsionRole::RollControl;
    if(Has(n,"omni"))return PropulsionRole::OmnidirectionalRcs;
    return PropulsionRole::LateralRcs;
}

WeaponAssemblyRole UniversalKitbashAuthority::InferWeaponRole(const ShipyardModuleRecord& record){
    const std::string n=Lower(record.source.moduleId);
    if(record.semantic==ShipyardModuleSemantic::TurretHardpoint){
        if(Has(n,"ring"))return WeaponAssemblyRole::TurretRing;
        if(Has(n,"turret")||Has(n,"base"))return WeaponAssemblyRole::TurretBase;
        return WeaponAssemblyRole::Hardpoint;
    }
    if(record.semantic!=ShipyardModuleSemantic::WeaponMount)return WeaponAssemblyRole::None;
    const bool barrel=Has(n,"barrel")||Has(n,"gun")||Has(n,"cannon");
    const bool base=Has(n,"base")||Has(n,"turret")||Has(n,"mount");
    if(Has(n,"missile")||Has(n,"launcher"))return WeaponAssemblyRole::MissileLauncher;
    if(Has(n,"pointdef")||Has(n,"point_def")||Has(n,"pd_"))return WeaponAssemblyRole::PointDefense;
    if(Has(n,"sensor")||Has(n,"optic")||Has(n,"radar"))return WeaponAssemblyRole::TargetingSensor;
    if(barrel&&base)return WeaponAssemblyRole::CompleteTurret;
    if(barrel)return Has(n,"cluster")||Has(n,"twin")||Has(n,"quad")?WeaponAssemblyRole::BarrelCluster:WeaponAssemblyRole::Barrel;
    if(Has(n,"housing"))return WeaponAssemblyRole::Housing;
    if(Has(n,"traverse"))return WeaponAssemblyRole::Traverse;
    return WeaponAssemblyRole::Hardpoint;
}

KitbashMorphProfile UniversalKitbashAuthority::InferMorphProfile(const ShipyardModuleRecord& record){
    KitbashMorphProfile p;p.referenceSize=FromShipyardSize(record.size);p.minimumSize=p.referenceSize;p.maximumSize=p.referenceSize;
    switch(record.semantic){
    case ShipyardModuleSemantic::StructuralFrame:
    case ShipyardModuleSemantic::HullMid:
    case ShipyardModuleSemantic::Adapter:
        p.policy=KitbashScalingPolicy::StructuralFree;p.minimumSize=UniversalSizeClass::XS;p.maximumSize=UniversalSizeClass::XL;
        p.minLengthScale=.35f;p.maxLengthScale=3.5f;p.minWidthScale=.60f;p.maxWidthScale=2.1f;p.minHeightScale=.60f;p.maxHeightScale=2.1f;break;
    case ShipyardModuleSemantic::MainEngine:
    case ShipyardModuleSemantic::EngineNozzle:
    case ShipyardModuleSemantic::RcsThruster:
    case ShipyardModuleSemantic::Wing:
    case ShipyardModuleSemantic::EngineHousing:
        p.policy=KitbashScalingPolicy::ParametricFunctional;p.minimumSize=UniversalSizeClass::XS;p.maximumSize=UniversalSizeClass::XL;
        p.minLengthScale=.55f;p.maxLengthScale=2.25f;p.minWidthScale=.55f;p.maxWidthScale=2.0f;p.minHeightScale=.55f;p.maxHeightScale=2.0f;break;
    case ShipyardModuleSemantic::CommandCockpit:
    case ShipyardModuleSemantic::CommandBridge:
    case ShipyardModuleSemantic::TurretHardpoint:
    case ShipyardModuleSemantic::WeaponMount:
        p.policy=KitbashScalingPolicy::DiscreteFamily;
        p.minimumSize=UniversalSizeClass::XS;p.maximumSize=UniversalSizeClass::XL;
        p.minLengthScale=.72f;p.maxLengthScale=1.45f;p.minWidthScale=.72f;p.maxWidthScale=1.45f;p.minHeightScale=.72f;p.maxHeightScale=1.45f;break;
    default:
        if(record.surfaceOnly){
            p.policy=KitbashScalingPolicy::StructuralFree;p.minimumSize=UniversalSizeClass::XS;p.maximumSize=UniversalSizeClass::XL;
            p.minLengthScale=.50f;p.maxLengthScale=2.5f;p.minWidthScale=.50f;p.maxWidthScale=2.5f;p.minHeightScale=.50f;p.maxHeightScale=2.5f;
        }else{
            // Unknown/general functional kitbash is still allowed to participate in
            // neighboring XS-XL families, but only as a conservative discrete
            // family variant. Truly fixed assets can be explicitly overridden
            // through authoring metadata later.
            p.policy=KitbashScalingPolicy::DiscreteFamily;
            p.minimumSize=UniversalSizeClass::XS;p.maximumSize=UniversalSizeClass::XL;
            p.minLengthScale=.72f;p.maxLengthScale=1.45f;p.minWidthScale=.72f;p.maxWidthScale=1.45f;p.minHeightScale=.72f;p.maxHeightScale=1.45f;
        }
        break;
    }
    p.mirrorX=record.semantic!=ShipyardModuleSemantic::CommandCockpit||record.mirrorPreferred;
    return p;
}

std::vector<KitbashDomainRole> UniversalKitbashAuthority::InferDomainRoles(const ShipyardModuleRecord& record){
    std::vector<KitbashDomainRole> roles;
    auto add=[&](ConstructionDomain d,const char* role,bool eligible=true){roles.push_back({d,role,eligible});};
    switch(record.semantic){
    case ShipyardModuleSemantic::StructuralFrame:add(ConstructionDomain::Ship,"STRUCTURAL_CONNECTOR");add(ConstructionDomain::Station,"STRUCTURAL_SPINE");add(ConstructionDomain::Planetary,"GANTRY_SUPPORT");break;
    case ShipyardModuleSemantic::Sensor:add(ConstructionDomain::Ship,"SENSOR");add(ConstructionDomain::Station,"SENSOR_ARRAY");add(ConstructionDomain::Planetary,"COMMUNICATIONS_ARRAY");add(ConstructionDomain::Weapon,"TARGETING_SENSOR");break;
    case ShipyardModuleSemantic::TurretHardpoint:case ShipyardModuleSemantic::WeaponMount:add(ConstructionDomain::Ship,"WEAPON");add(ConstructionDomain::Station,"DEFENSE");add(ConstructionDomain::Planetary,"DEFENSE");add(ConstructionDomain::Weapon,"TURRET_COMPONENT");break;
    case ShipyardModuleSemantic::MainEngine:case ShipyardModuleSemantic::EngineNozzle:case ShipyardModuleSemantic::RcsThruster:case ShipyardModuleSemantic::EngineHousing:add(ConstructionDomain::Ship,"PROPULSION");add(ConstructionDomain::Vehicle,"PROPULSION",false);break;
    case ShipyardModuleSemantic::Adapter:add(ConstructionDomain::Ship,"ADAPTER");add(ConstructionDomain::Station,"ADAPTER");add(ConstructionDomain::Planetary,"STRUCTURAL_ADAPTER");break;
    case ShipyardModuleSemantic::SurfaceDetail:add(ConstructionDomain::Ship,"DETAIL");add(ConstructionDomain::Station,"DETAIL");add(ConstructionDomain::Planetary,"DETAIL");break;
    default:add(ConstructionDomain::Ship,record.placementRole.empty()?"MODULE":record.placementRole.c_str());break;
    }
    return roles;
}

UniversalKitbashProfile UniversalKitbashAuthority::BuildProfile(const ShipyardModuleRecord& record,KitbashMaterialCertification materialCertification){
    UniversalKitbashProfile p;p.assetId=record.source.moduleId;p.referenceSize=FromShipyardSize(record.size);p.morph=InferMorphProfile(record);p.materialCertification=materialCertification;p.propulsionRole=InferPropulsionRole(record);p.weaponRole=InferWeaponRole(record);p.domainRoles=InferDomainRoles(record);p.hasKnownOrientation=record.mountFaceConfidence>=.5f||!record.preferredMountFace.empty()||record.semantic==ShipyardModuleSemantic::HullMid;p.socketCertified=!record.sockets.empty();p.pcgCertified=record.generatorEligible&&p.hasKnownOrientation&&p.socketCertified&&(materialCertification==KitbashMaterialCertification::Complete||materialCertification==KitbashMaterialCertification::NormalizedFallback);
    return p;
}
bool UniversalKitbashAuthority::IsPcgEligible(const UniversalKitbashProfile& profile){return profile.pcgCertified&&profile.materialCertification!=KitbashMaterialCertification::BrokenDependency&&profile.materialCertification!=KitbashMaterialCertification::ReviewRequired;}

} // namespace subspace
