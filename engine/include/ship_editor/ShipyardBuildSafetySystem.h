#pragma once

#include "input/InputState.h"

namespace subspace {

class ShipyardBuildSafetySystem {
public:
    /// Hard flight-input interlock used while structural build mode owns the
    /// viewport. This is intentionally explicit rather than relying on overlay
    /// focus so held thrust/fire keys can never leak into physics or VFX.
    static void SuppressFlightAndWeapons(InputState& input);
};

} // namespace subspace
