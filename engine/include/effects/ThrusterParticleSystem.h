#pragma once

#include "flight/ShipFlightControl.h"

#include <cstdint>
#include <vector>

namespace subspace {

struct ThrusterParticle {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float radius = 1.0f;
    float life = 0.0f;
    float maxLife = 1.0f;
    std::uint32_t color = 0xFFFFFFu;
};

struct ThrusterParticleEmitterState {
    std::vector<ThrusterParticle> particles;
    std::uint32_t seed = 1;
    int maxParticles = 180;
};

struct ThrusterParticleSpawnContext {
    float shipX = 0.0f;
    float shipY = 0.0f;
    float shipAngleRadians = 0.0f;
    float shipSpeed = 0.0f;
};

void EmitThrusterParticles(ThrusterParticleEmitterState& state,
                           const ThrusterParticleSpawnContext& context,
                           const ShipFlightControlOutput& control,
                           float dtSeconds);
void TickThrusterParticles(ThrusterParticleEmitterState& state, float dtSeconds);
int CountAliveThrusterParticles(const ThrusterParticleEmitterState& state);

} // namespace subspace
