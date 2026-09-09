#pragma once

#include "content/UniversalKitbashAuthority.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <string>

namespace subspace {

struct PropulsionAxisProfile {
    PropulsionRole role = PropulsionRole::None;
    Vector3 localMountAxis{0.0f,1.0f,0.0f};
    Vector3 localExhaustAxis{0.0f,-1.0f,0.0f};
    Vector3 localThrustAxis{0.0f,1.0f,0.0f};
    float gimbalDegrees = 0.0f;
    bool inferred = true;
    float confidence = 0.0f;
};

struct PropulsionPlacementReport {
    bool valid = false;
    PropulsionRole resolvedRole = PropulsionRole::None;
    Vector3 shipExhaustAxis{};
    Vector3 shipThrustAxis{};
    std::string message;
};

class PropulsionRoleSystem {
public:
    static PropulsionAxisProfile Infer(const ShipyardModuleRecord& record);
    static Vector3 TransformDirection(const VisualModulePlacement& placement, Vector3 localDirection);
    static PropulsionRole ResolveRoleFromThrust(const Vector3& thrustAxis, const Vector3& shipLocalPosition = {});
    static PropulsionPlacementReport Validate(const ShipyardModuleRecord& record,
                                              const VisualModulePlacement& placement,
                                              PropulsionRole requestedRole = PropulsionRole::None);
    static bool ReorientForRole(const ShipyardModuleRecord& record,
                                VisualModulePlacement& placement,
                                PropulsionRole requestedRole);
};

} // namespace subspace
