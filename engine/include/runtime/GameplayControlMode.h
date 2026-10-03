#pragma once

namespace subspace {

/// Authoritative runtime gameplay control ownership. Camera/HUD/overlays are
/// selected from this state but remain independent presentation layers.
enum class GameplayControlMode {
    OnFoot,
    Pilot,
    FleetCommand
};

} // namespace subspace
