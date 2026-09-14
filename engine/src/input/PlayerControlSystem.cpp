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

Vector3 RotateEulerXYZ(Vector3 v, const Vector3& r)
{
    // Physics rotation uses radians. Apply local X (pitch), local Y (roll),
    // then world/local-composed Z (yaw), matching the existing +Y nose / +Z up
    // convention while finally allowing all authored axes to participate.
    const float cx=std::cos(r.x), sx=std::sin(r.x);
    const float cy=std::cos(r.y), sy=std::sin(r.y);
    const float cz=std::cos(r.z), sz=std::sin(r.z);

    Vector3 x{v.x, v.y*cx-v.z*sx, v.y*sx+v.z*cx};
    Vector3 y{x.x*cy+x.z*sy, x.y, -x.x*sy+x.z*cy};
    return {y.x*cz-y.y*sz, y.x*sz+y.y*cz, y.z};
}

float PreferSemanticAxis(const InputState& input, InputAction semantic, InputAction legacyFallback)
{
    const float semanticValue=input.GetValue(semantic);
    return std::fabs(semanticValue)>0.0001f ? semanticValue : input.GetValue(legacyFallback);
}

Vector3 ClampVectorMagnitude(Vector3 value,float maxMagnitude)
{
    const float len=value.length();
    if(len>maxMagnitude&&len>0.0001f)return value*(maxMagnitude/len);
    return value;
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

    const bool full3D=_flightAuthorityMode==FlightAuthorityMode::Full3D;
    Vector3 forward{};
    Vector3 right{};
    Vector3 up{};
    if(full3D){
        forward=RotateEulerXYZ({0.0f,1.0f,0.0f},physics->rotation).normalized();
        right=RotateEulerXYZ({1.0f,0.0f,0.0f},physics->rotation).normalized();
        up=RotateEulerXYZ({0.0f,0.0f,1.0f},physics->rotation).normalized();
    }else{
        const float yaw=physics->rotation.z;
        const float sinYaw=std::sin(yaw),cosYaw=std::cos(yaw);
        forward={-sinYaw,cosYaw,0.0f};
        right={cosYaw,sinYaw,0.0f};
        up={0.0f,0.0f,1.0f};
    }

    _forwardResponse = SmoothResponse(_forwardResponse, _inputState.GetValue(InputAction::ThrustForward), _tuning.translationalResponse, deltaTime);
    _reverseResponse = SmoothResponse(_reverseResponse, _inputState.GetValue(InputAction::ThrustReverse), _tuning.translationalResponse, deltaTime);
    _leftResponse = SmoothResponse(_leftResponse, _inputState.GetValue(InputAction::StrafeLeft), _tuning.translationalResponse, deltaTime);
    _rightResponse = SmoothResponse(_rightResponse, _inputState.GetValue(InputAction::StrafeRight), _tuning.translationalResponse, deltaTime);
    const float rawUp=full3D?PreferSemanticAxis(_inputState,InputAction::FlightThrustUp,InputAction::EditorNudgeUp):0.0f;
    const float rawDown=full3D?PreferSemanticAxis(_inputState,InputAction::FlightThrustDown,InputAction::EditorNudgeDown):0.0f;
    _upResponse=SmoothResponse(_upResponse,rawUp,_tuning.translationalResponse,deltaTime);
    _downResponse=SmoothResponse(_downResponse,rawDown,_tuning.translationalResponse,deltaTime);

    Vector3 thrust{};
    thrust = thrust + forward * (_forwardResponse * _tuning.forwardThrustRatio);
    thrust = thrust - forward * (_reverseResponse * _tuning.reverseThrustRatio);
    thrust = thrust - right * (_leftResponse * _tuning.lateralThrustRatio);
    thrust = thrust + right * (_rightResponse * _tuning.lateralThrustRatio);
    if(full3D){
        thrust=thrust+up*(_upResponse*_tuning.verticalThrustRatio);
        thrust=thrust-up*(_downResponse*_tuning.verticalThrustRatio);
    }

    const bool hasTranslationInput = thrust.length() > 0.0001f;
    if (hasTranslationInput) {
        const float boostFactor = _boostActive ? _tuning.boostMultiplier : 1.0f;
        const float baseThrust = physics->maxThrust * _tuning.thrustMultiplier * boostFactor;
        physics->AddForce(thrust * baseThrust);
    } else if (_inertialDampeningEnabled) {
        Vector3 velocity=full3D?physics->velocity:Vector3{physics->velocity.x,physics->velocity.y,0.0f};
        if (velocity.length() > 0.05f) {
            Vector3 damping = velocity * (-physics->mass * _tuning.dampeningStrength);
            const float maxDamping = physics->maxThrust * _tuning.idleLinearDampeningLimit;
            physics->AddForce(ClampVectorMagnitude(damping,maxDamping));
        }
    }

    const Vector3 governedVelocity=full3D?physics->velocity:Vector3{physics->velocity.x,physics->velocity.y,0.0f};
    const float speed=governedVelocity.length();
    const float preferredSpeed = _boostActive ? _tuning.boostSpeed : _tuning.cruiseSpeed;
    if (speed > preferredSpeed && speed > 0.001f) {
        const float excessRatio = std::clamp((speed - preferredSpeed) / std::max(1.0f, preferredSpeed), 0.0f, 1.5f);
        physics->AddForce(governedVelocity.normalized() *
            (-physics->maxThrust * _tuning.softCapStrength * excessRatio));
    }

    const float rawYaw=ClampMagnitude(_inputState.GetValue(InputAction::TurnLeft)-_inputState.GetValue(InputAction::TurnRight),1.0f);
    const float rawPitch=full3D?ClampMagnitude(PreferSemanticAxis(_inputState,InputAction::FlightPitchUp,InputAction::EditorNudgeForward)-PreferSemanticAxis(_inputState,InputAction::FlightPitchDown,InputAction::EditorNudgeAft),1.0f):0.0f;
    const float rawRoll=full3D?ClampMagnitude(PreferSemanticAxis(_inputState,InputAction::FlightRollRight,InputAction::EditorNudgeRight)-PreferSemanticAxis(_inputState,InputAction::FlightRollLeft,InputAction::EditorNudgeLeft),1.0f):0.0f;
    _yawResponse=SmoothResponse(_yawResponse,rawYaw,_tuning.rotationalResponse,deltaTime);
    _pitchResponse=SmoothResponse(_pitchResponse,rawPitch,_tuning.rotationalResponse,deltaTime);
    _rollResponse=SmoothResponse(_rollResponse,rawRoll,_tuning.rotationalResponse,deltaTime);

    Vector3 requestedTorque{
        _pitchResponse*physics->maxTorque*_tuning.rotationMultiplier*_tuning.pitchTorqueRatio,
        _rollResponse*physics->maxTorque*_tuning.rotationMultiplier*_tuning.rollTorqueRatio,
        _yawResponse*physics->maxTorque*_tuning.rotationMultiplier*_tuning.yawTorqueRatio
    };
    if(requestedTorque.length()>0.0001f)physics->AddTorque(requestedTorque);
    else if(_inertialDampeningEnabled){
        Vector3 angular=full3D?physics->angularVelocity:Vector3{0.0f,0.0f,physics->angularVelocity.z};
        if(angular.length()>0.001f){
            Vector3 damping=angular*(-physics->momentOfInertia*_tuning.dampeningStrength);
            damping=ClampVectorMagnitude(damping,physics->maxTorque*_tuning.idleRotationalDampeningLimit);
            physics->AddTorque(damping);
        }
    }

    if (_inputState.IsDown(InputAction::EmergencyBrake)) {
        const Vector3 brakeVelocity = full3D ? physics->velocity : Vector3{physics->velocity.x, physics->velocity.y, 0.0f};
        if (brakeVelocity.length() > 0.01f) {
            physics->AddForce(brakeVelocity.normalized() * (-physics->maxThrust * _tuning.emergencyBrakeMultiplier));
        }
        Vector3 angular=full3D?physics->angularVelocity:Vector3{0.0f,0.0f,physics->angularVelocity.z};
        if(angular.length()>0.001f){
            physics->AddTorque(angular.normalized()*(-physics->maxTorque*_tuning.emergencyBrakeMultiplier));
        }
    }

    if (!full3D) {
        physics->appliedForce.z = 0.0f;
        physics->appliedTorque.x = 0.0f;
        physics->appliedTorque.y = 0.0f;
    }
}

} // namespace subspace
