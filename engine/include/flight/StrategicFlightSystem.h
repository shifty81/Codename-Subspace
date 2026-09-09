#pragma once

#include "core/Math.h"
#include <cstdint>
#include <string>

namespace subspace {

enum class FlightControlMode { Manual, Strategic };
enum class StrategicOrderKind { None, Approach, Orbit, KeepRange, Align, Follow, Hold, Engage, Mine, Salvage, Dock, VectorTo };

struct StrategicFlightOrder {
    StrategicOrderKind kind=StrategicOrderKind::None;
    std::uint64_t targetId=0;
    Vector3 targetPosition{};
    float desiredRange=0.0f;
    bool valid=false;
};

struct StrategicFlightIntent {
    Vector3 desiredDirection{};
    float desiredThrottle=0.0f;
    bool fire=false;
    bool useVector=false;
    bool requestDock=false;
};

class StrategicFlightSystem {
public:
    void ToggleMode();
    void SetMode(FlightControlMode mode){mode_=mode;}
    FlightControlMode Mode() const{return mode_;}
    void Issue(const StrategicFlightOrder& order){order_=order;}
    const StrategicFlightOrder& CurrentOrder() const{return order_;}
    StrategicFlightIntent Evaluate(const Vector3& shipPosition,const Vector3& targetPosition,float distance) const;
private:
    FlightControlMode mode_=FlightControlMode::Manual;
    StrategicFlightOrder order_{};
};

} // namespace subspace
