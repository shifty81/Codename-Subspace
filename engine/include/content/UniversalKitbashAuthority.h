#pragma once

#include "assets/CanonicalAsset.h"
#include "content/ShipyardModuleSystem.h"

#include <string>
#include <vector>

namespace subspace {

// Pass595-614: one normalized construction vocabulary for every kitbash-based
// editor/runtime domain. Domain-specific systems extend this contract instead
// of inventing parallel size/scaling/material authorities.
enum class ConstructionDomain {
    Ship,
    Station,
    Planetary,
    Weapon,
    Vehicle,
    Interior
};

enum class UniversalSizeClass {
    XS,
    S,
    M,
    L,
    XL
};

enum class KitbashScalingPolicy {
    StructuralFree,
    ParametricFunctional,
    DiscreteFamily,
    FixedReference
};

enum class KitbashMaterialCertification {
    Complete,
    NormalizedFallback,
    ReviewRequired,
    BrokenDependency
};

enum class PropulsionRole {
    None,
    MainDrive,
    RetroBrake,
    LateralRcs,
    VerticalRcs,
    PitchControl,
    YawControl,
    RollControl,
    VtolLift,
    VtolControl,
    LandingThruster,
    OmnidirectionalRcs
};

enum class WeaponAssemblyRole {
    None,
    Hardpoint,
    TurretRing,
    TurretBase,
    Traverse,
    Housing,
    Barrel,
    BarrelCluster,
    MissileLauncher,
    PointDefense,
    TargetingSensor,
    CompleteTurret
};

struct KitbashDomainRole {
    ConstructionDomain domain = ConstructionDomain::Ship;
    std::string role;
    bool generatorEligible = true;
};

struct KitbashMorphProfile {
    KitbashScalingPolicy policy = KitbashScalingPolicy::FixedReference;
    UniversalSizeClass referenceSize = UniversalSizeClass::M;
    UniversalSizeClass minimumSize = UniversalSizeClass::M;
    UniversalSizeClass maximumSize = UniversalSizeClass::M;
    float minLengthScale = 1.0f;
    float maxLengthScale = 1.0f;
    float minWidthScale = 1.0f;
    float maxWidthScale = 1.0f;
    float minHeightScale = 1.0f;
    float maxHeightScale = 1.0f;
    bool preserveMountRegions = true;
    bool preserveFunctionalRegions = true;
    bool mirrorX = true;
};

struct UniversalKitbashProfile {
    std::string assetId;
    UniversalSizeClass referenceSize = UniversalSizeClass::M;
    KitbashMorphProfile morph{};
    KitbashMaterialCertification materialCertification = KitbashMaterialCertification::ReviewRequired;
    PropulsionRole propulsionRole = PropulsionRole::None;
    WeaponAssemblyRole weaponRole = WeaponAssemblyRole::None;
    std::vector<KitbashDomainRole> domainRoles;
    std::vector<assets::SurfaceSemantic> appearanceSurfaces;
    bool hasKnownOrientation = false;
    bool socketCertified = false;
    bool pcgCertified = false;
};

class UniversalKitbashAuthority {
public:
    static UniversalSizeClass FromShipyardSize(ShipyardModuleSize size);
    static ShipyardModuleSize ToShipyardSize(UniversalSizeClass size);
    static const char* SizeName(UniversalSizeClass size);
    static float NominalScale(UniversalSizeClass size);
    static bool SizeWithin(UniversalSizeClass value, UniversalSizeClass minimum, UniversalSizeClass maximum);
    static UniversalSizeClass ClampSizeToMorph(const UniversalKitbashProfile& profile, UniversalSizeClass requested);
    static float SafeUniformScale(const UniversalKitbashProfile& profile, UniversalSizeClass target);

    static PropulsionRole InferPropulsionRole(const ShipyardModuleRecord& record);
    static WeaponAssemblyRole InferWeaponRole(const ShipyardModuleRecord& record);
    static KitbashMorphProfile InferMorphProfile(const ShipyardModuleRecord& record);
    static std::vector<KitbashDomainRole> InferDomainRoles(const ShipyardModuleRecord& record);
    static UniversalKitbashProfile BuildProfile(const ShipyardModuleRecord& record,
                                                KitbashMaterialCertification materialCertification = KitbashMaterialCertification::NormalizedFallback);

    static bool IsPcgEligible(const UniversalKitbashProfile& profile);
};

} // namespace subspace
