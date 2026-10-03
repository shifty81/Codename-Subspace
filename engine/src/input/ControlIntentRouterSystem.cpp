#include "input/ControlIntentRouterSystem.h"

#include <algorithm>

namespace subspace {
namespace {
float ClampAxis(float value) { return std::clamp(value, -1.0f, 1.0f); }
float Value(const InputState& input, InputAction action) { return input.GetValue(action); }
}

float ControlIntentRouterSystem::Axis(const InputState& input, InputAction positive, InputAction negative,
                                      InputAction legacyPositive, InputAction legacyNegative)
{
    const float semantic = Value(input, positive) - Value(input, negative);
    if (semantic > 0.0001f || semantic < -0.0001f) return ClampAxis(semantic);
    return ClampAxis(Value(input, legacyPositive) - Value(input, legacyNegative));
}

bool ControlIntentRouterSystem::DownEither(const InputState& input, InputAction semantic, InputAction legacy)
{
    return input.IsDown(semantic) || input.IsDown(legacy);
}

ControlIntent ControlIntentRouterSystem::Build(const InputState& input, ControlDomain domain)
{
    ControlIntent out;
    out.domain = domain;

    switch (domain) {
        case ControlDomain::Pilot:
            out.forward = Axis(input, InputAction::PilotForward, InputAction::PilotReverse,
                               InputAction::ThrustForward, InputAction::ThrustReverse);
            out.right = Axis(input, InputAction::PilotStrafeRight, InputAction::PilotStrafeLeft,
                             InputAction::StrafeRight, InputAction::StrafeLeft);
            out.up = Axis(input, InputAction::PilotThrustUp, InputAction::PilotThrustDown,
                          InputAction::FlightThrustUp, InputAction::FlightThrustDown);
            out.boost = DownEither(input, InputAction::PilotBoost, InputAction::Boost);
            out.brake = DownEither(input, InputAction::PilotBrake, InputAction::EmergencyBrake);
            out.firePrimary = DownEither(input, InputAction::PilotFirePrimary, InputAction::FirePrimary);
            out.fireSecondary = DownEither(input, InputAction::PilotFireSecondary, InputAction::FireMiningMissile);
            out.headLook = input.IsDown(InputAction::PilotHeadLook);
            out.interact = input.WasPressed(InputAction::CharacterInteract);
            break;

        case ControlDomain::FirstPerson:
        case ControlDomain::DockedService:
            out.forward = Axis(input, InputAction::CharacterMoveForward, InputAction::CharacterMoveBackward,
                               InputAction::ThrustForward, InputAction::ThrustReverse);
            out.right = Axis(input, InputAction::CharacterMoveRight, InputAction::CharacterMoveLeft,
                             InputAction::StrafeRight, InputAction::StrafeLeft);
            out.sprint = input.IsDown(InputAction::CharacterSprint);
            out.crouch = input.IsDown(InputAction::CharacterCrouch);
            out.jump = input.WasPressed(InputAction::CharacterJump);
            out.interact = input.WasPressed(InputAction::CharacterInteract) || input.WasPressed(InputAction::MenuAccept);
            out.primaryUse = input.IsDown(InputAction::CharacterPrimaryUse);
            out.secondaryUse = input.IsDown(InputAction::CharacterSecondaryUse);
            out.headLook = input.IsDown(InputAction::CharacterHeadLook);
            break;

        case ControlDomain::FleetStrategy:
            out.cameraForward = Axis(input, InputAction::FleetCameraForward, InputAction::FleetCameraBackward,
                                     InputAction::ThrustForward, InputAction::ThrustReverse);
            out.cameraRight = Axis(input, InputAction::FleetCameraRight, InputAction::FleetCameraLeft,
                                   InputAction::StrafeRight, InputAction::StrafeLeft);
            out.cameraUp = Axis(input, InputAction::FleetCameraUp, InputAction::FleetCameraDown,
                                InputAction::FlightThrustUp, InputAction::FlightThrustDown);
            out.queueModifier = input.IsDown(InputAction::FleetQueueModifier);
            out.additiveSelection = input.IsDown(InputAction::FleetAddSelection);
            out.commandConfirm = input.WasPressed(InputAction::FleetCommandConfirm) || input.WasPressed(InputAction::MenuAccept);
            out.commandCancel = input.WasPressed(InputAction::FleetCommandCancel) || input.WasPressed(InputAction::MenuBack);
            // Intentionally do not populate pilot fire/thrust fields here.
            break;

        case ControlDomain::Authoring:
        case ControlDomain::Transit:
        case ControlDomain::Disabled:
            break;
    }
    return out;
}

} // namespace subspace
