#pragma once

#include "core/Math.h"
#include "integration/PlayerFacingIntegrationSystem.h"
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class FleetCaptainTemperament { Cautious, Balanced, Aggressive, Industrial, Supportive };
enum class FleetCaptainState { Forming, Following, CatchingUp, Engaging, Mining, Salvaging, Supporting, Docking, VectorSync, Holding };

struct FleetCaptainProfile {
    std::uint64_t captainId = 0;
    std::string name;
    float navigation = 0.65f;
    float tactics = 0.55f;
    float industry = 0.55f;
    float discipline = 0.70f;
    FleetCaptainTemperament temperament = FleetCaptainTemperament::Balanced;
};

struct FleetAiShipState {
    std::uint64_t shipId = 0;
    FleetShipRole role = FleetShipRole::Combat;
    FleetCaptainProfile captain{};
    Vector3 position{};
    Vector3 velocity{};
    float headingRadians = 0.0f;
    Vector3 formationTarget{};
    Vector3 actionTarget{};
    StrategicOrderKind mirroredOrder = StrategicOrderKind::Follow;
    FleetCaptainState state = FleetCaptainState::Forming;
    std::string behavior = "FORMING";
    bool initialized = false;
    bool operational = true;
};

struct FleetCaptainRuntime {
    std::vector<FleetAiShipState> ships;
};

class FleetCaptainAiSystem {
public:
    FleetCaptainProfile MakeCaptain(std::uint64_t shipId, FleetShipRole role) const;
    void EnsureWing(FleetCaptainRuntime& runtime, const FleetRuntimeModel& desired,
                    const Vector3& leaderPosition, float leaderHeadingRadians) const;
    void Step(FleetCaptainRuntime& runtime, const FleetRuntimeModel& desired,
              const Vector3& leaderPosition, const Vector3& leaderVelocity,
              float leaderHeadingRadians, const Vector3& selectedTarget,
              bool targetValid, bool vectorActive, bool docked, float deltaTime) const;
    static const char* StateName(FleetCaptainState state);
};

} // namespace subspace
