#include "effects/ShipThrusterPortLayout.h"

namespace subspace {

const char* ThrusterPortRoleName(ThrusterPortRole role) {
    switch (role) {
    case ThrusterPortRole::MainAft: return "MainAft";
    case ThrusterPortRole::RetroForward: return "RetroForward";
    case ThrusterPortRole::RcsLeft: return "RcsLeft";
    case ThrusterPortRole::RcsRight: return "RcsRight";
    case ThrusterPortRole::RcsNoseLeft: return "RcsNoseLeft";
    case ThrusterPortRole::RcsNoseRight: return "RcsNoseRight";
    case ThrusterPortRole::RcsTailLeft: return "RcsTailLeft";
    case ThrusterPortRole::RcsTailRight: return "RcsTailRight";
    }
    return "Unknown";
}

std::vector<ThrusterPortDefinition> CreateDefaultThrusterPortLayout(float shipHalfWidth, float shipHalfLength) {
    const float w = shipHalfWidth > 0.1f ? shipHalfWidth : 1.0f;
    const float l = shipHalfLength > 0.1f ? shipHalfLength : 1.5f;
    return {
        {"main_aft", ThrusterPortRole::MainAft, 0.0f, l, 180.0f, 1.0f},
        {"retro_forward", ThrusterPortRole::RetroForward, 0.0f, -l, 0.0f, 0.45f},
        {"rcs_left", ThrusterPortRole::RcsLeft, -w, 0.0f, 90.0f, 0.55f},
        {"rcs_right", ThrusterPortRole::RcsRight, w, 0.0f, -90.0f, 0.55f},
        {"rcs_nose_left", ThrusterPortRole::RcsNoseLeft, -w, -l, 90.0f, 0.40f},
        {"rcs_nose_right", ThrusterPortRole::RcsNoseRight, w, -l, -90.0f, 0.40f},
        {"rcs_tail_left", ThrusterPortRole::RcsTailLeft, -w, l, 90.0f, 0.40f},
        {"rcs_tail_right", ThrusterPortRole::RcsTailRight, w, l, -90.0f, 0.40f},
    };
}

std::vector<ThrusterPortDefinition> ResolveFiringThrusterPorts(const std::vector<ThrusterPortDefinition>& ports, const ThrusterPortFireState& state) {
    std::vector<ThrusterPortDefinition> firing;
    for (const auto& port : ports) {
        const bool main = state.mainDriveEnabled && ((port.role == ThrusterPortRole::MainAft && state.forward) || (port.role == ThrusterPortRole::RetroForward && state.reverse));
        const bool rcsStrafe = state.rcsEnabled && ((port.role == ThrusterPortRole::RcsLeft && state.strafeRight) || (port.role == ThrusterPortRole::RcsRight && state.strafeLeft));
        const bool rcsRotateLeft = state.rcsEnabled && state.rotateLeft && (port.role == ThrusterPortRole::RcsNoseRight || port.role == ThrusterPortRole::RcsTailLeft);
        const bool rcsRotateRight = state.rcsEnabled && state.rotateRight && (port.role == ThrusterPortRole::RcsNoseLeft || port.role == ThrusterPortRole::RcsTailRight);
        if (main || rcsStrafe || rcsRotateLeft || rcsRotateRight) firing.push_back(port);
    }
    return firing;
}

} // namespace subspace
