#pragma once

#include "core/Math.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class StationBerthSize { Small, Standard, Heavy, Capital };

enum class DockNavLightKind { Guidance, Hold, Aperture, Closed, ServiceBoundary };

struct StationDockWaypoint {
    Vector3 position{};
    float radius = 3.0f;
    float speedLimit = 18.0f;
};

struct StationDockGeometry {
    std::uint64_t stationId = 0;
    std::string berthId;
    StationBerthSize berthSize = StationBerthSize::Standard;
    Vector3 stationWorld{};
    Vector3 approachDirection{0.0f,-1.0f,0.0f};
    Vector3 apertureWorld{};
    Vector3 captureWorld{};
    Vector3 undockWorld{};
    float dockingEnvelopeRadius = 18.0f;
    float captureRadius = 1.6f;
    float captureSpeedLimit = 5.0f;
    std::vector<StationDockWaypoint> corridor;
};

struct DockCaptureEvaluation {
    bool insideEnvelope = false;
    bool insideCapture = false;
    bool speedAcceptable = false;
    float distanceToCapture = 0.0f;
    float alignment = 0.0f;
    bool capturable = false;
};

class StationDockingGeometrySystem {
public:
    /// Build a deterministic berth facing the approaching ship. The station does
    /// not teleport the ship to its center: aperture/capture/undock positions are
    /// all exterior physical locations that can be rendered and manually flown.
    StationDockGeometry Build(std::uint64_t stationId,
                              const Vector3& stationWorld,
                              const Vector3& approachingShipWorld,
                              StationBerthSize requestedSize = StationBerthSize::Standard) const;

    DockCaptureEvaluation Evaluate(const StationDockGeometry& dock,
                                   const Vector3& shipWorld,
                                   const Vector3& shipVelocity,
                                   const Vector3& shipForward) const;

    static const char* BerthSizeName(StationBerthSize size);
};

} // namespace subspace
