#pragma once

#include "content/UniversalKitbashAuthority.h"
#include "ships/ShipClassSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct ShipClassEnvelope {
    ShipClass shipClass = ShipClass::Frigate;
    UniversalSizeClass structuralSize = UniversalSizeClass::XS;
    float minimumLengthMeters = 40.0f;
    float maximumLengthMeters = 90.0f;
    float nominalLengthMeters = 65.0f;
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
    static const char* ClassName(ShipClass shipClass);
    static const char* RoleName(ShipRole role);
    static ShipRoleBudget RoleBudget(ShipRole role);

    // Every faction receives four physical hull families per normal combat
    // class. Roles are configurations of those platforms, not one-model-per-role.
    static std::vector<FactionHullFamilyDefinition> BuildDefaultHullFamilies(const std::string& factionId,
                                                                              ShipClass shipClass);
    static bool SupportsRole(const FactionHullFamilyDefinition& family, ShipRole role);
};

} // namespace subspace
