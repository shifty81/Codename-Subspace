#include "input/PlayerControlSystem.h"

#include "core/Math.h"
#include "core/physics/PhysicsComponent.h"

#include <algorithm>
#include <cmath>

namespace subspace {

namespace {

float ClampMagnitude(float value, float maxAbs)
{
    return std::clamp(value, -maxAbs, maxAbs);
}

float SmoothResponse(float current, float target, float response, float dt)
{
    if (dt <= 0.0f) return target;
    const float alpha = 1.0f - std::exp(-std::max(0.0f, response) * dt);
    return current + (target - current) * alpha;
}

} // namespace

PlayerControlSystem::PlayerControlSystem(EntityManager& entityManager, InputState& inputState)
    : SystemBase("PlayerControlSystem")
    , _entityManager(entityManager)
    , _inputState(inputState)
{
}

void PlayerControlSystem::Update(float deltaTime)
{
    if (_inputState.WasPressed(InputAction::ToggleDampening)) {
        _inertialDampeningEnabled = !_inertialDampeningEnabled;
    }

    _boostActive = _inputState.IsDown(InputAction::Boost);

    if (_controlledShipId == InvalidEntityId) return;

    auto* physics = _entityManager.GetComponent<PhysicsComponent>(_controlledShipId);
    if (!physics || physics->isStatic) return;

    // +Y is the ship nose in the current visual/socket convention.
    const float yaw = physics->rotation.z;
    const float sinYaw = std::sin(yaw);
    const float cosYaw = std::cos(yaw);
    const Vector3 forward{-sinYaw, cosYaw, 0.0f};
    const Vector3 right{cosYaw, sinYaw, 0.0f};

    _forwardResponse = SmoothResponse(_forwardResponse, _inputState.GetValue(InputAction::ThrustForward), _tuning.translationalResponse, deltaTime);
    _reverseResponse = SmoothResponse(_reverseResponse, _inputState.GetValue(InputAction::ThrustReverse), _tuning.translationalResponse, deltaTime);
    _leftResponse = SmoothResponse(_leftResponse, _inputState.GetValue(InputAction::StrafeLeft), _tuning.translationalResponse, deltaTime);
    _rightResponse = SmoothResponse(_rightResponse, _inputState.GetValue(InputAction::StrafeRight), _tuning.translationalResponse, deltaTime);

    Vector3 thrust{};
    thrust = thrust + forward * (_forwardResponse * _tuning.forwardThrustRatio);
    thrust = thrust - forward * (_reverseResponse * _tuning.reverseThrustRatio);
    thrust = thrust - right * (_leftResponse * _tuning.lateralThrustRatio);
    thrust = thrust + right * (_rightResponse * _tuning.lateralThrustRatio);

    const bool hasTranslationInput = thrust.length() > 0.0001f;
    if (hasTranslationInput) {
        const float boostFactor = _boostActive ? _tuning.boostMultiplier : 1.0f;
        const float baseThrust = physics->maxThrust * _tuning.thrustMultiplier * boostFactor;
        physics->AddForce(thrust * baseThrust);
    } else if (_inertialDampeningEnabled) {
        Vector3 planarVelocity{physics->velocity.x, physics->velocity.y, 0.0f};
        if (planarVelocity.length() > 0.05f) {
            Vector3 damping = planarVelocity * (-physics->mass * _tuning.dampeningStrength);
            const float maxDamping = physics->maxThrust * _tuning.idleLinearDampeningLimit;
            if (damping.length() > maxDamping) {
                damping = damping.normalized() * maxDamping;
            }
            physics->AddForce(damping);
        }
    }

    // Newtonian-ish soft cap: velocity is never hard-snapped. Past the preferred
    // tactical cruise envelope, thrusters apply a progressive counter-force so
    // momentum remains visible while the battlefield does not become warp-speed.
    Vector3 planarVelocity{physics->velocity.x, physics->velocity.y, 0.0f};
    const float planarSpeed = planarVelocity.length();
    const float preferredSpeed = _boostActive ? _tuning.boostSpeed : _tuning.cruiseSpeed;
    if (planarSpeed > preferredSpeed && planarSpeed > 0.001f) {
        const float excessRatio = std::clamp((planarSpeed - preferredSpeed) / std::max(1.0f, preferredSpeed), 0.0f, 1.5f);
        physics->AddForce(planarVelocity.normalized() *
            (-physics->maxThrust * _tuning.softCapStrength * excessRatio));
    }

    // Turn is yaw only. Faux-Z battlefield presentation remains render-only.
    const float rawTurn = ClampMagnitude(_inputState.GetValue(InputAction::TurnLeft) -
                                         _inputState.GetValue(InputAction::TurnRight), 1.0f);
    _turnResponse = SmoothResponse(_turnResponse, rawTurn, _tuning.rotationalResponse, deltaTime);
    const float turnInput = _turnResponse;
    const bool hasTurnInput = std::fabs(turnInput) > 0.0001f;

    if (hasTurnInput) {
        physics->AddTorque({0.0f, 0.0f,
            turnInput * physics->maxTorque * _tuning.rotationMultiplier});
    } else if (_inertialDampeningEnabled && std::fabs(physics->angularVelocity.z) > 0.001f) {
        float dampingTorque = -physics->angularVelocity.z * physics->momentOfInertia * _tuning.dampeningStrength;
        const float maxDamping = physics->maxTorque * _tuning.idleRotationalDampeningLimit;
        dampingTorque = ClampMagnitude(dampingTorque, maxDamping);
        physics->AddTorque({0.0f, 0.0f, dampingTorque});
    }

    if (_inputState.IsDown(InputAction::EmergencyBrake)) {
        Vector3 planarVelocity{physics->velocity.x, physics->velocity.y, 0.0f};
        if (planarVelocity.length() > 0.01f) {
            physics->AddForce(planarVelocity.normalized() *
                (-physics->maxThrust * _tuning.emergencyBrakeMultiplier));
        }
        if (std::fabs(physics->angularVelocity.z) > 0.001f) {
            const float direction = physics->angularVelocity.z > 0.0f ? -1.0f : 1.0f;
            physics->AddTorque({0.0f, 0.0f,
                direction * physics->maxTorque * _tuning.emergencyBrakeMultiplier});
        }
    }

    // Enforce the authoritative 2D contract at the controller boundary.
    physics->appliedForce.z = 0.0f;
    physics->appliedTorque.x = 0.0f;
    physics->appliedTorque.y = 0.0f;
}

} // namespace subspace
