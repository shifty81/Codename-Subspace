#include "ship_editor/ShipyardBuildSafetySystem.h"
#include "input/InputState.h"

namespace subspace {

void ShipyardBuildSafetySystem::SuppressFlightAndWeapons(InputState& input)
{
    // Camera/editor input is intentionally consumed before this safety pass.
    // Clear every gameplay action that can apply force, torque, boost, braking,
    // or weapon fire before PlayerControlSystem and combat systems tick. This
    // keeps the authored ship/station inert while the Shipyard camera moves.
    constexpr InputAction suppressed[] = {
        InputAction::ThrustForward,
        InputAction::ThrustReverse,
        InputAction::StrafeLeft,
        InputAction::StrafeRight,
        InputAction::TurnLeft,
        InputAction::TurnRight,
        InputAction::FlightThrustUp,
        InputAction::FlightThrustDown,
        InputAction::FlightPitchUp,
        InputAction::FlightPitchDown,
        InputAction::FlightRollLeft,
        InputAction::FlightRollRight,
        InputAction::Boost,
        InputAction::EmergencyBrake,
        InputAction::FirePrimary,
        InputAction::FireMiningMissile,
        // Temporary native-backend compatibility bindings used by the 6DOF
        // controller until every backend publishes semantic flight axes.
        InputAction::EditorNudgeLeft,
        InputAction::EditorNudgeRight,
        InputAction::EditorNudgeForward,
        InputAction::EditorNudgeAft,
        InputAction::EditorNudgeUp,
        InputAction::EditorNudgeDown,
    };

    for (const auto action : suppressed) {
        input.SetAction(action, false);
        input.SetActionValue(action, 0.0f);
    }
}

} // namespace subspace
