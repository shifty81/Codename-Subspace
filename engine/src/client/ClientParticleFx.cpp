#include "client/ClientParticleFx.h"

#include <algorithm>
#include <cmath>

namespace subspace::client {

void ClientParticleFx::SetSeed(std::uint32_t seed)
{
    _seed = seed == 0 ? 0xC0FFEEu : seed;
}

float ClientParticleFx::Rand01()
{
    _seed = _seed * 1664525u + 1013904223u;
    return static_cast<float>(_seed & 0x00ffffffu) / static_cast<float>(0x01000000u);
}

float ClientParticleFx::RandSigned()
{
    return Rand01() * 2.0f - 1.0f;
}

void ClientParticleFx::Update(float dt)
{
    for (auto& particle : _particles) {
        particle.age += dt;
        particle.x += particle.vx * dt;
        particle.y += particle.vy * dt;
        particle.vx *= 0.985f;
        particle.vy *= 0.985f;
    }
    _particles.erase(std::remove_if(_particles.begin(), _particles.end(), [](const Particle2D& particle) {
        return particle.age >= particle.lifetime;
    }), _particles.end());
}

void ClientParticleFx::Burst(const ParticleBurstRequest& request)
{
    const int clampedCount = std::max(0, std::min(request.count, 64));
    for (int i = 0; i < clampedCount; ++i) {
        Particle2D particle;
        const float angle = request.directionRadians + RandSigned() * request.spreadRadians;
        const float speed = std::max(0.0f, request.baseSpeed + RandSigned() * request.speedVariance);
        particle.x = request.x;
        particle.y = request.y;
        particle.vx = request.inheritVx + std::cos(angle) * speed;
        particle.vy = request.inheritVy + std::sin(angle) * speed;
        particle.lifetime = std::max(0.05f, request.lifetime * (0.75f + Rand01() * 0.5f));
        particle.size = std::max(1.0f, request.size * (0.7f + Rand01() * 0.8f));
        particle.startColor = request.startColor;
        particle.endColor = request.endColor;
        _particles.push_back(particle);
    }
    if (_particles.size() > 700) {
        _particles.erase(_particles.begin(), _particles.begin() + static_cast<std::ptrdiff_t>(_particles.size() - 700));
    }
}

void ClientParticleFx::Clear()
{
    _particles.clear();
}

const std::vector<Particle2D>& ClientParticleFx::Particles() const
{
    return _particles;
}

int ClientParticleFx::AliveCount() const
{
    return static_cast<int>(_particles.size());
}

std::uint32_t LerpRgb(std::uint32_t a, std::uint32_t b, float t)
{
    t = std::max(0.0f, std::min(1.0f, t));
    const auto ar = static_cast<int>((a >> 16) & 0xffu);
    const auto ag = static_cast<int>((a >> 8) & 0xffu);
    const auto ab = static_cast<int>(a & 0xffu);
    const auto br = static_cast<int>((b >> 16) & 0xffu);
    const auto bg = static_cast<int>((b >> 8) & 0xffu);
    const auto bb = static_cast<int>(b & 0xffu);
    const auto rr = static_cast<std::uint32_t>(ar + static_cast<int>((br - ar) * t));
    const auto rg = static_cast<std::uint32_t>(ag + static_cast<int>((bg - ag) * t));
    const auto rb = static_cast<std::uint32_t>(ab + static_cast<int>((bb - ab) * t));
    return (rr << 16) | (rg << 8) | rb;
}

} // namespace subspace::client
