#pragma once

#include <cstdint>
#include <vector>

namespace subspace::client {

struct Particle2D {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float age = 0.0f;
    float lifetime = 0.45f;
    float size = 2.0f;
    std::uint32_t startColor = 0x80d8ffu;
    std::uint32_t endColor = 0x162033u;
};

struct ParticleBurstRequest {
    float x = 0.0f;
    float y = 0.0f;
    float directionRadians = 0.0f;
    float spreadRadians = 0.4f;
    float baseSpeed = 80.0f;
    float speedVariance = 80.0f;
    float lifetime = 0.45f;
    float size = 2.0f;
    int count = 1;
    float inheritVx = 0.0f;
    float inheritVy = 0.0f;
    std::uint32_t startColor = 0x80d8ffu;
    std::uint32_t endColor = 0x162033u;
};

class ClientParticleFx {
public:
    void SetSeed(std::uint32_t seed);
    void Update(float dt);
    void Burst(const ParticleBurstRequest& request);
    void Clear();
    const std::vector<Particle2D>& Particles() const;
    int AliveCount() const;

private:
    float Rand01();
    float RandSigned();

    std::uint32_t _seed = 0xC0FFEEu;
    std::vector<Particle2D> _particles;
};

std::uint32_t LerpRgb(std::uint32_t a, std::uint32_t b, float t);

} // namespace subspace::client
