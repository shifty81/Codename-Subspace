#pragma once

#include "station/StationDockingGeometrySystem.h"

#include <vector>

namespace subspace {

struct StationNavLight {
    Vector3 position{};
    DockNavLightKind kind = DockNavLightKind::Guidance;
    float phaseOffset = 0.0f;
    float intensity = 1.0f;
};

class StationNavigationLightSystem {
public:
    std::vector<StationNavLight> Build(const StationDockGeometry& dock,
                                       bool clearanceGranted,
                                       bool berthOccupied) const;
    static float Pulse(const StationNavLight& light,double seconds);
};

} // namespace subspace
