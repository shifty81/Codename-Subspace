#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/SystemBase.h"
#include "input/InputState.h"

namespace subspace {

/// Native replacement for the legacy C# PlayerControlSystem.
///
/// Subspace world/physics authority is fully three-dimensional. The current
/// keyboard flight mapping remains predominantly planar for feel/backward
/// compatibility, but the controller must not erase valid Z force/velocity or
/// pitch/roll torque produced by other systems. TacticalPlanar is an explicit
/// opt-in constraint mode for command/tutorial contexts, never world truth.
class PlayerControlSystem : public SystemBase {
public:
    enum class FlightAuthorityMode { Full3D, TacticalPlanar };
    struct Tuning {
        float thrustMultiplier = 1.0f;
        float rotationMultiplier = 1.0f;
        float forwardThrustRatio = 1.0f;
        float reverseThrustRatio = 0.58f;
        float lateralThrustRatio = 0.42f;
        float dampeningStrength = 0.42f;
        float boostMultiplier = 1.65f;
        float idleLinearDampeningLimit = 0.56f; // maxThrust multiplier
        float idleRotationalDampeningLimit = 0.58f; // maxTorque multiplier
        float emergencyBrakeMultiplier = 1.35f;
        float cruiseSpeed = 18.0f;
        float boostSpeed = 30.0f;
        float softCapStrength = 0.72f;
        float translationalResponse = 3.4f;
        float rotationalResponse = 4.2f;
    };

    PlayerControlSystem(EntityManager& entityManager, InputState& inputState);

    void Update(float deltaTime) override;

    void SetControlledShip(EntityId entityId) { _controlledShipId = entityId; }
    EntityId GetControlledShip() const { return _controlledShipId; }
    void ClearControlledShip() { _controlledShipId = InvalidEntityId; }

    bool IsInertialDampeningEnabled() const { return _inertialDampeningEnabled; }
    void SetInertialDampeningEnabled(bool enabled) { _inertialDampeningEnabled = enabled; }
    bool IsBoostActive() const { return _boostActive; }
    FlightAuthorityMode GetFlightAuthorityMode() const { return _flightAuthorityMode; }
    void SetFlightAuthorityMode(FlightAuthorityMode mode) { _flightAuthorityMode = mode; }

    const Tuning& GetTuning() const { return _tuning; }
    void SetTuning(const Tuning& tuning) { _tuning = tuning; }

private:
    EntityManager& _entityManager;
    InputState& _inputState;
    EntityId _controlledShipId = InvalidEntityId;
    Tuning _tuning{};
    bool _inertialDampeningEnabled = true;
    bool _boostActive = false;
    FlightAuthorityMode _flightAuthorityMode = FlightAuthorityMode::Full3D;
    float _forwardResponse = 0.0f;
    float _reverseResponse = 0.0f;
    float _leftResponse = 0.0f;
    float _rightResponse = 0.0f;
    float _turnResponse = 0.0f;
};

} // namespace subspace
