#pragma once

#include "core/Math.h"
#include "interior/ShipEmbodimentSystem.h"

namespace subspace {

enum class FirstPersonViewKind { Cockpit, InteriorOnFoot };

struct FirstPersonViewPose {
    FirstPersonViewKind kind=FirstPersonViewKind::InteriorOnFoot;
    Vector3 position{};
    Vector3 forward{0.0f,1.0f,0.0f};
    Vector3 right{1.0f,0.0f,0.0f};
    Vector3 up{0.0f,0.0f,1.0f};
    float verticalFovDegrees=76.0f;
    float nearPlaneMeters=0.03f;
};

/// Pure camera-pose authority shared by the runtime and Shipyard Play mode.
/// It intentionally contains no world ownership: the caller composes a ship-
/// local pose with the persistent ship transform.
class FirstPersonViewSystem {
public:
    static FirstPersonViewPose BuildOnFootLocal(const InteriorAvatarState& avatar);
    static FirstPersonViewPose BuildCockpitLocal(const Vector3& eyeLocal,float yawRadians,float pitchRadians,float rollRadians=0.0f);
};

} // namespace subspace
