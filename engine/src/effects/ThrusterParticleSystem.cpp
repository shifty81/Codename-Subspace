#include "effects/ThrusterParticleSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

static float Random01(std::uint32_t& seed) {
    seed = seed * 1664525u + 1013904223u;
    return static_cast<float>((seed >> 8) & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

static void Spawn(ThrusterParticleEmitterState& state,
                  float x, float y, float vx, float vy,
                  float radius, float life, std::uint32_t color) {
    if (static_cast<int>(state.particles.size()) >= state.maxParticles) {
        state.particles.erase(state.particles.begin());
    }
    ThrusterParticle p;
    p.x = x;
    p.y = y;
    p.vx = vx;
    p.vy = vy;
    p.radius = radius;
    p.life = life;
    p.maxLife = life;
    p.color = color;
    state.particles.push_back(p);
}

void EmitThrusterParticles(ThrusterParticleEmitterState& state,
                           const ThrusterParticleSpawnContext& context,
                           const ShipFlightControlOutput& control,
                           float dtSeconds) {
    const float c = std::cos(context.shipAngleRadians);
    const float s = std::sin(context.shipAngleRadians);
    const float rearX = context.shipX - c * 18.0f;
    const float rearY = context.shipY - s * 18.0f;
    const int mainBursts = control.mainBurning ? std::max(1, static_cast<int>(32.0f * dtSeconds)) : 0;
    const int retroBursts = control.retroBurning ? std::max(1, static_cast<int>(16.0f * dtSeconds)) : 0;
    const int rcsBursts = (control.leftRcsBurning || control.rightRcsBurning || control.strafeRcsBurning)
                          ? std::max(1, static_cast<int>(22.0f * dtSeconds)) : 0;

    for (int i = 0; i < mainBursts; ++i) {
        const float jitter = (Random01(state.seed) - 0.5f) * 9.0f;
        Spawn(state, rearX - s * jitter, rearY + c * jitter,
              -c * (120.0f + Random01(state.seed) * 90.0f),
              -s * (120.0f + Random01(state.seed) * 90.0f),
              2.0f + Random01(state.seed) * 2.5f, 0.35f + Random01(state.seed) * 0.25f, 0x66CCFFu);
    }
    for (int i = 0; i < retroBursts; ++i) {
        Spawn(state, context.shipX + c * 18.0f, context.shipY + s * 18.0f,
              c * 90.0f, s * 90.0f, 1.8f, 0.25f, 0xFFBB66u);
    }
    for (int i = 0; i < rcsBursts; ++i) {
        const float side = control.leftRcsBurning ? -1.0f : 1.0f;
        const float sx = context.shipX + s * 13.0f * side;
        const float sy = context.shipY - c * 13.0f * side;
        Spawn(state, sx, sy, s * side * 72.0f, -c * side * 72.0f, 1.4f, 0.22f, 0xAADDFFu);
    }
}

void TickThrusterParticles(ThrusterParticleEmitterState& state, float dtSeconds) {
    for (auto& p : state.particles) {
        p.x += p.vx * dtSeconds;
        p.y += p.vy * dtSeconds;
        p.life -= dtSeconds;
        p.radius = std::max(0.1f, p.radius * (1.0f - dtSeconds * 1.4f));
    }
    state.particles.erase(std::remove_if(state.particles.begin(), state.particles.end(),
        [](const ThrusterParticle& p) { return p.life <= 0.0f; }), state.particles.end());
}

int CountAliveThrusterParticles(const ThrusterParticleEmitterState& state) {
    return static_cast<int>(state.particles.size());
}

} // namespace subspace
