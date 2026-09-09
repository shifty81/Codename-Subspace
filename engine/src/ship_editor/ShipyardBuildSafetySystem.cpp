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
        InputAction::Boost,
        InputAction::EmergencyBrake,
        InputAction::FirePrimary,
        InputAction::FireMiningMissile,
    };

    for (const auto action : suppressed) {
        input.SetAction(action, false);
        input.SetActionValue(action, 0.0f);
    }
}

} // namespace subspace
