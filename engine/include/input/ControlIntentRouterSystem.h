#pragma once

#include "input/InputState.h"

namespace subspace {

/// One explicit owner for interpreting physical/gameplay actions.  The native
/// window may keep emitting the historical flight actions while old input
/// profiles migrate; this router ensures those values mean different things in
/// cockpit, first-person, fleet-strategy, and authoring contexts.
enum class ControlDomain {
    Pilot,
    FirstPerson,
    FleetStrategy,
    Authoring,
    DockedService,
    Transit,
    Disabled
};

struct ControlIntent {
    ControlDomain domain = ControlDomain::Disabled;

    // Local movement axes.  Forward/right/up are normalized to [-1,1].
    float forward = 0.0f;
    float right = 0.0f;
    float up = 0.0f;

    // First-person state.
    bool sprint = false;
    bool crouch = false;
    bool jump = false;
    bool interact = false;

    // Direct-pilot state.  These remain false outside Pilot.
    bool boost = false;
    bool brake = false;
    bool firePrimary = false;
    bool fireSecondary = false;

    // Fleet strategy state.  W/S/A/D are camera pan here, never ship thrust.
    float cameraForward = 0.0f;
    float cameraRight = 0.0f;
    float cameraUp = 0.0f;
    bool queueModifier = false;
    bool additiveSelection = false;
    bool commandConfirm = false;
    bool commandCancel = false;

    bool AllowsShipThrust() const { return domain == ControlDomain::Pilot; }
    bool AllowsAvatarMotion() const { return domain == ControlDomain::FirstPerson || domain == ControlDomain::DockedService; }
    bool AllowsFleetOrders() const { return domain == ControlDomain::FleetStrategy; }
};

class ControlIntentRouterSystem {
public:
    static ControlIntent Build(const InputState& input, ControlDomain domain);
    static bool SuppressesFlight(ControlDomain domain) { return domain != ControlDomain::Pilot; }

private:
    static float Axis(const InputState& input, InputAction positive, InputAction negative,
                      InputAction legacyPositive, InputAction legacyNegative);
    static bool DownEither(const InputState& input, InputAction semantic, InputAction legacy);
};

} // namespace subspace
