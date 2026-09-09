#pragma once

#include "core/Math.h"
#include "station/StationDockingGeometrySystem.h"
#include <cstdint>
#include <string>
#include <cstddef>

namespace subspace {

enum class DockingExperienceStage { Undocked, Requested, Approach, Capture, Docked, Undocking };
struct DockingExperienceState {
    DockingExperienceStage stage = DockingExperienceStage::Undocked;
    std::uint64_t stationId = 0;
    Vector3 stationWorld{};
    Vector3 approachWorld{};
    StationDockGeometry geometry{};
    std::size_t corridorWaypoint = 0;
    Vector3 assignedForward{0.0f,1.0f,0.0f};
    float assignedSpeedLimit = 18.0f;
    float alignment = 0.0f;
    float distanceToGuidance = 0.0f;
    float lateralError = 0.0f;
    bool insideCorridor = false;
    std::string guidanceCue;
    double progress = 0.0;
    bool autoDock = true;
    std::string berthId;
    double corridorRadius = 4.0;
    double captureStrength = 0.0;
    bool trafficClearance = false;
    bool hangarReady = false;
    std::string status;
};

class DockingExperienceSystem {
public:
    bool Request(DockingExperienceState& state, std::uint64_t stationId,
                 const Vector3& stationWorld, const Vector3& shipWorld, bool autoDock) const;
    Vector3 Update(DockingExperienceState& state, const Vector3& shipWorld, double seconds) const;
    bool RequestUndock(DockingExperienceState& state) const;
    bool IsDocked(const DockingExperienceState& state) const { return state.stage==DockingExperienceStage::Docked; }
};

} // namespace subspace
