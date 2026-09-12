#pragma once

#include "content/UniversalKitbashAuthority.h"
#include "ships/ShipClassSystem.h"

#include <array>
#include <string>
#include <vector>

namespace subspace {

struct ShipClassEnvelope {
    ShipClass shipClass = ShipClass::Frigate;

    // Compatibility field retained for older callers. This is a preferred
    // structural component tier only; it is never the identity of the ship.
    UniversalSizeClass structuralSize = UniversalSizeClass::S;

    float minimumLengthMeters = 40.0f;
    float maximumLengthMeters = 90.0f;
    float nominalLengthMeters = 65.0f;
};

struct ShipClassComponentProfile {
    ShipClass shipClass = ShipClass::Frigate;
    UniversalSizeClass preferredStructuralSize = UniversalSizeClass::S;
    UniversalSizeClass minimumStructuralSize = UniversalSizeClass::XS;
    UniversalSizeClass maximumStructuralSize = UniversalSizeClass::M;
    UniversalSizeClass minimumAuxiliarySize = UniversalSizeClass::XS;
    UniversalSizeClass maximumAuxiliarySize = UniversalSizeClass::M;

    // Indexed by UniversalSizeClass XS..XL. These are selection weights, not
    // legality booleans; zero means the class should not normally generate that
    // tier for the requested lane.
    std::array<float,5> structuralWeights{{0.25f,0.55f,0.20f,0.0f,0.0f}};
    std::array<float,5> auxiliaryWeights{{0.45f,0.40f,0.15f,0.0f,0.0f}};
};

struct ShipRoleBudget {
    ShipRole role = ShipRole::GeneralCombat;
    float weapons = 1.0f;
    float armor = 1.0f;
    float sensors = 1.0f;
    float utility = 1.0f;
    float cargo = 1.0f;
    float hangar = 0.0f;
    float propulsion = 1.0f;
    float logistics = 0.0f;
    float industry = 0.0f;
};

struct ShipRoleSpatialProfile {
    ShipRole role = ShipRole::GeneralCombat;
    float cargoVolumeBias = 1.0f;
    float machineryVolumeBias = 1.0f;
    float habitationVolumeBias = 1.0f;
    float hangarVolumeBias = 0.0f;
    float commandVolumeBias = 1.0f;
    float maintenanceAccessBias = 1.0f;
    float exteriorAccessBias = 1.0f;
    float sensorExposureBias = 1.0f;
    float armorShellBias = 1.0f;
    float propulsionReserveBias = 1.0f;
    std::vector<std::string> requiredInteriorFunctions;
    std::vector<std::string> preferredExteriorRoles;
};

struct FactionHullFamilyDefinition {
    std::string factionId;
    ShipClass shipClass = ShipClass::Frigate;
    int designIndex = 0; // 0..3 by normalized faction/class policy.
    std::string familyId;
    std::string chassisStyle;
    std::vector<ShipRole> allowedRoles;
    std::vector<ShipRole> preferredRoles;
    float speedBias = 1.0f;
    float armorBias = 1.0f;
    float utilityBias = 1.0f;
    float internalVolumeBias = 1.0f;
};

class ShipClassRoleSystem {
public:
    static ShipClassEnvelope Envelope(ShipClass shipClass);
    static ShipClassComponentProfile ComponentProfile(ShipClass shipClass);
    static const char* ClassName(ShipClass shipClass);
    static const char* RoleName(ShipRole role);
    static ShipRoleBudget RoleBudget(ShipRole role);
    static ShipRoleSpatialProfile RoleSpatialProfile(ShipRole role);

    static float SizeWeight(const ShipClassComponentProfile& profile,
                            UniversalSizeClass size,
                            bool auxiliary);
    static bool SupportsModuleSize(const ShipClassComponentProfile& profile,
                                   UniversalSizeClass size,
                                   bool auxiliary);

    // Every faction receives four physical hull families per normal combat
    // class. Roles are configurations of those platforms, not one-model-per-role.
    static std::vector<FactionHullFamilyDefinition> BuildDefaultHullFamilies(const std::string& factionId,
                                                                              ShipClass shipClass);
    static bool SupportsRole(const FactionHullFamilyDefinition& family, ShipRole role);
};

} // namespace subspace
