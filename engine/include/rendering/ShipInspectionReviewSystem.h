#pragma once

#include "core/Math.h"
#include <string>
#include <vector>

namespace subspace {

enum class ShipInspectionSnap { Free, Front, Rear, Left, Right, Top, Bottom, FrontQuarter, RearQuarter };

struct ShipInspectionCameraState {
    float yawDegrees = 0.0f;
    float pitchDegrees = 22.0f;
    float distance = 18.0f;
    Vector3 pan{};
    ShipInspectionSnap snap = ShipInspectionSnap::Free;
};

struct ShipInspectionOverlay {
    bool sockets = false;
    bool moduleBounds = false;
    bool attachmentLines = false;
    bool thrustVectors = false;
    bool invalidModules = false;
};

class ShipInspectionReviewSystem {
public:
    void Orbit(ShipInspectionCameraState& state, float deltaYaw, float deltaPitch) const;
    void Pan(ShipInspectionCameraState& state, float deltaX, float deltaY) const;
    void Zoom(ShipInspectionCameraState& state, float wheelDelta) const;
    void Snap(ShipInspectionCameraState& state, ShipInspectionSnap snap) const;
    std::vector<std::string> OverlayLegend(const ShipInspectionOverlay& overlay) const;
};

} // namespace subspace
