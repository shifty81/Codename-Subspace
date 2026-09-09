#pragma once

#include "navigation/SystemNavigationSystem.h"

#include <string>

namespace subspace {

enum class VectorTravelStage { Idle, Aligning, Charging, Cruise, Decelerating, Complete, Failed };

struct VectorTravelSession {
    VectorTravelStage stage = VectorTravelStage::Idle;
    WarpPlan plan;
    double elapsedSeconds = 0.0;
    double stageSeconds = 0.0;
    double progress = 0.0;
    double stageProgress = 0.0;
    double visualPhase = 0.0;
    double plannedSeconds = 0.0;
    double remainingSeconds = 0.0;
    std::string status;
};

class VectorTravelSystem {
public:
    bool Begin(VectorTravelSession& session, const WarpPlan& plan) const;
    void Update(VectorTravelSession& session, double deltaSeconds) const;
    void Cancel(VectorTravelSession& session) const;
    bool InTransit(const VectorTravelSession& session) const;
};

} // namespace subspace
