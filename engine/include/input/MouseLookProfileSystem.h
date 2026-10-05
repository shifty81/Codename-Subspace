#pragma once

#include <algorithm>

namespace subspace {

struct MouseLookDelta {
    float yawRadians=0.0f;
    float pitchRadians=0.0f;
};

struct PilotMouseSteer {
    float yawAxis=0.0f;   // +right / -left
    float pitchAxis=0.0f; // +down / -up
};

/// Central mouse-direction/sensitivity contract. Win32 raw X is positive when
/// the device moves right, while the current Subspace yaw basis turns right
/// with a negative yaw delta; keeping that conversion here prevents another
/// left/right inversion from being reintroduced independently by FPS/cockpit.
class MouseLookProfileSystem {
public:
    static MouseLookDelta OnFoot(float rawDx,float rawDy){return {-rawDx*0.0026f,-rawDy*0.0026f};}
    static MouseLookDelta PilotHead(float rawDx,float rawDy){return {-rawDx*0.0024f,-rawDy*0.0024f};}
    static PilotMouseSteer PilotSteer(float rawDx,float rawDy){
        return {std::clamp(rawDx*0.025f,-1.0f,1.0f),std::clamp(rawDy*0.025f,-1.0f,1.0f)};
    }
};

} // namespace subspace
