#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/SystemBase.h"
#include "input/InputState.h"

namespace subspace {

/// Native first-person ship-flight authority.
///
/// Full3D uses the ship's complete Euler orientation to derive local forward,
/// right and up axes and permits translation/rotation on all six degrees of
/// freedom. TacticalPlanar is retained only as an explicit opt-in compatibility
/// constraint; it is never the physical truth of the world.
class PlayerControlSystem : public SystemBase {
public:
    enum class FlightAuthorityMode { Full3D, TacticalPlanar };
    struct Tuning {
        float thrustMultiplier = 1.0f;
        float rotationMultiplier = 1.0f;
        float forwardThrustRatio = 1.0f;
        float reverseThrustRatio = 0.58f;
        float lateralThrustRatio = 0.42f;
        float verticalThrustRatio = 0.38f;
        float pitchTorqueRatio = 0.82f;
        float rollTorqueRatio = 0.76f;
        float yawTorqueRatio = 1.0f;
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
    float _upResponse = 0.0f;
    float _downResponse = 0.0f;
    float _yawResponse = 0.0f;
    float _pitchResponse = 0.0f;
    float _rollResponse = 0.0f;
};

} // namespace subspace
