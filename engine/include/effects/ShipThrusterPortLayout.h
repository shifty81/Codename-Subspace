#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ThrusterPortRole {
    MainAft,
    RetroForward,
    RcsLeft,
    RcsRight,
    RcsNoseLeft,
    RcsNoseRight,
    RcsTailLeft,
    RcsTailRight
};

struct ThrusterPortDefinition {
    std::string portId;
    ThrusterPortRole role = ThrusterPortRole::MainAft;
    float localX = 0.0f;
    float localY = 0.0f;
    float exhaustAngleDegrees = 0.0f;
    float strength = 1.0f;
};

struct ThrusterPortFireState {
    bool forward = false;
    bool reverse = false;
    bool strafeLeft = false;
    bool strafeRight = false;
    bool rotateLeft = false;
    bool rotateRight = false;
    bool mainDriveEnabled = true;
    bool rcsEnabled = true;
};

std::vector<ThrusterPortDefinition> CreateDefaultThrusterPortLayout(float shipHalfWidth, float shipHalfLength);
std::vector<ThrusterPortDefinition> ResolveFiringThrusterPorts(const std::vector<ThrusterPortDefinition>& ports, const ThrusterPortFireState& state);
const char* ThrusterPortRoleName(ThrusterPortRole role);

} // namespace subspace
