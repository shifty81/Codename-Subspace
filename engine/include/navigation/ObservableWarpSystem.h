#pragma once

#include "core/Math.h"
#include "navigation/VectorTravelSystem.h"

#include <cstdint>
#include <vector>

namespace subspace {

enum class WarpEvidenceKind { ChargeField, DepartureStreak, TransitWake, CollapseRing, ArrivalFlare };

struct ObservableWarpEvent {
    std::uint64_t shipId = 0;
    WarpEvidenceKind kind = WarpEvidenceKind::ChargeField;
    Vector3 origin{};
    Vector3 direction{0.0f,1.0f,0.0f};
    float intensity = 0.0f;
    float ageSeconds = 0.0f;
    float lifetimeSeconds = 1.0f;
};

class ObservableWarpSystem {
public:
    static void EmitStageTransition(std::vector<ObservableWarpEvent>& events,std::uint64_t shipId,
                                    VectorTravelStage previous,VectorTravelStage next,
                                    const Vector3& origin,const Vector3& direction);
    static void Tick(std::vector<ObservableWarpEvent>& events,float deltaSeconds);
};

} // namespace subspace
