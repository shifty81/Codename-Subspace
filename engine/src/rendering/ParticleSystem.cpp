#include "rendering/ParticleSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// Particle
// ---------------------------------------------------------------------------

bool Particle::IsAlive() const { return age < lifetime; }

float Particle::GetNormalizedAge() const {
    if (lifetime <= 0.0f) return 1.0f;
    return std::min(age / lifetime, 1.0f);
}

void Particle::GetCurrentColor(float& r, float& g, float& b, float& a) const {
    float t = GetNormalizedAge();
    r = colorR + (endColorR - colorR) * t;
    g = colorG + (endColorG - colorG) * t;
    b = colorB + (endColorB - colorB) * t;
    a = colorA + (endColorA - colorA) * t;
}

// ---------------------------------------------------------------------------
// ParticleEmitter
// ---------------------------------------------------------------------------

ParticleEmitter::ParticleEmitter() = default;

ParticleEmitter::ParticleEmitter(const std::string& id,
                                 const ParticleEmitterConfig& cfg)
    : _id(id), _config(cfg) {}

const std::string& ParticleEmitter::GetId() const { return _id; }
bool ParticleEmitter::IsActive() const { return _active; }
void ParticleEmitter::SetActive(bool active) { _active = active; }

void ParticleEmitter::SetPosition(float x, float y, float z) {
    _posX = x; _posY = y; _posZ = z;
}

void ParticleEmitter::GetPosition(float& x, float& y, float& z) const {
    x = _posX; y = _posY; z = _posZ;
}

const ParticleEmitterConfig& ParticleEmitter::GetConfig() const { return _config; }
void ParticleEmitter::SetConfig(const ParticleEmitterConfig& cfg) { _config = cfg; }

void ParticleEmitter::SetSeed(uint32_t seed) { _seed = seed; }

float ParticleEmitter::RandFloat() {
    // Simple LCG
    _seed = _seed * 1664525u + 1013904223u;
    return static_cast<float>(_seed & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

float ParticleEmitter::RandRange(float lo, float hi) {
    return lo + RandFloat() * (hi - lo);
}

Particle ParticleEmitter::SpawnOne() {
    Particle p;
    p.lifetime = RandRange(_config.minLifetime, _config.maxLifetime);
    p.size     = RandRange(_config.minSize, _config.maxSize);
    p.age      = 0.0f;

    p.colorR    = _config.startR; p.colorG    = _config.startG;
    p.colorB    = _config.startB; p.colorA    = _config.startA;
    p.endColorR = _config.endR;   p.endColorG = _config.endG;
    p.endColorB = _config.endB;   p.endColorA = _config.endA;

    float speed = RandRange(_config.minSpeed, _config.maxSpeed);

    switch (_config.shape) {
    case EmitterShape::Point:
        p.posX = _posX; p.posY = _posY; p.posZ = _posZ;
        {
            // Random direction on unit sphere
            float theta = RandRange(0.0f, 6.2831853f);
            float phi   = std::acos(RandRange(-1.0f, 1.0f));
            float sp    = std::sin(phi);
            p.velX = speed * sp * std::cos(theta);
            p.velY = speed * sp * std::sin(theta);
            p.velZ = speed * std::cos(phi);
        }
        break;

    case EmitterShape::Sphere:
        {
            float theta = RandRange(0.0f, 6.2831853f);
            float phi   = std::acos(RandRange(-1.0f, 1.0f));
            float sp    = std::sin(phi);
            float r     = RandRange(0.0f, _config.sphereRadius);
            p.posX = _posX + r * sp * std::cos(theta);
            p.posY = _posY + r * sp * std::sin(theta);
            p.posZ = _posZ + r * std::cos(phi);
            p.velX = speed * sp * std::cos(theta);
            p.velY = speed * sp * std::sin(theta);
            p.velZ = speed * std::cos(phi);
        }
        break;

    case EmitterShape::Cone:
        {
            float halfAngle = _config.coneAngleDeg * 3.14159265f / 180.0f;
            float theta = RandRange(0.0f, 6.2831853f);
            float phi   = RandRange(0.0f, halfAngle);
            float sp    = std::sin(phi);
            p.posX = _posX; p.posY = _posY; p.posZ = _posZ;
            // Cone points along +Y
            p.velX = speed * sp * std::cos(theta);
            p.velZ = speed * sp * std::sin(theta);
            p.velY = speed * std::cos(phi);
        }
        break;

    case EmitterShape::Box:
        p.posX = _posX + RandRange(-_config.boxHalfW, _config.boxHalfW);
        p.posY = _posY + RandRange(-_config.boxHalfH, _config.boxHalfH);
        p.posZ = _posZ + RandRange(-_config.boxHalfD, _config.boxHalfD);
        {
            float theta = RandRange(0.0f, 6.2831853f);
            float phi   = std::acos(RandRange(-1.0f, 1.0f));
            float sp    = std::sin(phi);
            p.velX = speed * sp * std::cos(theta);
            p.velY = speed * sp * std::sin(theta);
            p.velZ = speed * std::cos(phi);
        }
        break;
    }

    return p;
}

void ParticleEmitter::Update(float dt) {
    // Update existing particles
    for (auto& p : _particles) {
        p.age += dt;
        p.velY += _config.gravityY * dt;
        p.posX += p.velX * dt;
        p.posY += p.velY * dt;
        p.posZ += p.velZ * dt;
    }

    // Remove dead particles
    _particles.erase(
        std::remove_if(_particles.begin(), _particles.end(),
                       [](const Particle& p) { return !p.IsAlive(); }),
        _particles.end());

    // Emit new particles
    if (_active) {
        _emitAccumulator += _config.emitRate * dt;
        int toEmit = static_cast<int>(_emitAccumulator);
        _emitAccumulator -= static_cast<float>(toEmit);

        for (int i = 0; i < toEmit; ++i) {
            if (static_cast<int>(_particles.size()) >= _config.maxParticles) break;
            _particles.push_back(SpawnOne());
        }
    }
}

void ParticleEmitter::Emit(int count) {
    for (int i = 0; i < count; ++i) {
        if (static_cast<int>(_particles.size()) >= _config.maxParticles) break;
        _particles.push_back(SpawnOne());
    }
}

void ParticleEmitter::Reset() {
    _particles.clear();
    _emitAccumulator = 0.0f;
}

int ParticleEmitter::GetAliveCount() const {
    return static_cast<int>(_particles.size());
}

const std::vector<Particle>& ParticleEmitter::GetParticles() const {
    return _particles;
}

// ---------------------------------------------------------------------------
// ParticleComponent
// ---------------------------------------------------------------------------

void ParticleComponent::AddEmitter(const ParticleEmitter& emitter) {
    emitters.push_back(emitter);
}

bool ParticleComponent::RemoveEmitter(const std::string& id) {
    auto it = std::find_if(emitters.begin(), emitters.end(),
                           [&](const ParticleEmitter& e) { return e.GetId() == id; });
    if (it == emitters.end()) return false;
    emitters.erase(it);
    return true;
}

ParticleEmitter* ParticleComponent::GetEmitter(const std::string& id) {
    for (auto& e : emitters)
        if (e.GetId() == id) return &e;
    return nullptr;
}

int ParticleComponent::GetTotalParticleCount() const {
    int total = 0;
    for (const auto& e : emitters) total += e.GetAliveCount();
    return total;
}

int ParticleComponent::GetEmitterCount() const {
    return static_cast<int>(emitters.size());
}

void ParticleComponent::StopAll() {
    for (auto& e : emitters) e.SetActive(false);
}

void ParticleComponent::ResumeAll() {
    for (auto& e : emitters) e.SetActive(true);
}

// ---------------------------------------------------------------------------
// ParticleSystem
// ---------------------------------------------------------------------------

ParticleSystem::ParticleSystem() : SystemBase("ParticleSystem") {}

void ParticleSystem::Initialize() {}
void ParticleSystem::Update(float /*deltaTime*/) {}
void ParticleSystem::Shutdown() {}

// ---------------------------------------------------------------------------
// Preset emitter configs
// ---------------------------------------------------------------------------

ParticleEmitterConfig ParticleSystem::CreateExplosionPreset() {
    ParticleEmitterConfig cfg;
    cfg.shape         = EmitterShape::Sphere;
    cfg.sphereRadius  = 0.5f;
    cfg.emitRate      = 0.0f;   // burst only
    cfg.maxParticles  = 100;
    cfg.minLifetime   = 0.3f;
    cfg.maxLifetime   = 1.0f;
    cfg.minSpeed      = 5.0f;
    cfg.maxSpeed      = 15.0f;
    cfg.minSize       = 0.2f;
    cfg.maxSize       = 0.8f;
    cfg.gravityY      = -2.0f;
    cfg.startR = 1.0f; cfg.startG = 0.6f; cfg.startB = 0.1f; cfg.startA = 1.0f;
    cfg.endR   = 0.3f; cfg.endG   = 0.0f; cfg.endB   = 0.0f; cfg.endA   = 0.0f;
    return cfg;
}

ParticleEmitterConfig ParticleSystem::CreateEngineThrustPreset() {
    ParticleEmitterConfig cfg;
    cfg.shape        = EmitterShape::Cone;
    cfg.coneAngleDeg = 15.0f;
    cfg.emitRate     = 50.0f;
    cfg.maxParticles = 200;
    cfg.minLifetime  = 0.2f;
    cfg.maxLifetime  = 0.6f;
    cfg.minSpeed     = 8.0f;
    cfg.maxSpeed     = 14.0f;
    cfg.minSize      = 0.05f;
    cfg.maxSize      = 0.15f;
    cfg.startR = 0.3f; cfg.startG = 0.5f; cfg.startB = 1.0f; cfg.startA = 1.0f;
    cfg.endR   = 0.1f; cfg.endG   = 0.1f; cfg.endB   = 0.3f; cfg.endA   = 0.0f;
    return cfg;
}

ParticleEmitterConfig ParticleSystem::CreateShieldHitPreset() {
    ParticleEmitterConfig cfg;
    cfg.shape         = EmitterShape::Point;
    cfg.emitRate      = 0.0f;
    cfg.maxParticles  = 60;
    cfg.minLifetime   = 0.2f;
    cfg.maxLifetime   = 0.5f;
    cfg.minSpeed      = 2.0f;
    cfg.maxSpeed      = 6.0f;
    cfg.minSize       = 0.1f;
    cfg.maxSize       = 0.3f;
    cfg.startR = 0.2f; cfg.startG = 0.8f; cfg.startB = 1.0f; cfg.startA = 1.0f;
    cfg.endR   = 0.0f; cfg.endG   = 0.2f; cfg.endB   = 0.5f; cfg.endA   = 0.0f;
    return cfg;
}

ParticleEmitterConfig ParticleSystem::CreateMiningPreset() {
    ParticleEmitterConfig cfg;
    cfg.shape         = EmitterShape::Cone;
    cfg.coneAngleDeg  = 25.0f;
    cfg.emitRate      = 30.0f;
    cfg.maxParticles  = 100;
    cfg.minLifetime   = 0.4f;
    cfg.maxLifetime   = 1.0f;
    cfg.minSpeed      = 3.0f;
    cfg.maxSpeed      = 8.0f;
    cfg.minSize       = 0.08f;
    cfg.maxSize       = 0.25f;
    cfg.gravityY      = -1.5f;
    cfg.startR = 0.7f; cfg.startG = 0.5f; cfg.startB = 0.3f; cfg.startA = 1.0f;
    cfg.endR   = 0.3f; cfg.endG   = 0.2f; cfg.endB   = 0.1f; cfg.endA   = 0.0f;
    return cfg;
}

ParticleEmitterConfig ParticleSystem::CreateHyperdrivePreset() {
    ParticleEmitterConfig cfg;
    cfg.shape        = EmitterShape::Cone;
    cfg.coneAngleDeg = 5.0f;
    cfg.emitRate     = 80.0f;
    cfg.maxParticles = 300;
    cfg.minLifetime  = 0.1f;
    cfg.maxLifetime  = 0.4f;
    cfg.minSpeed     = 20.0f;
    cfg.maxSpeed     = 40.0f;
    cfg.minSize      = 0.02f;
    cfg.maxSize      = 0.08f;
    cfg.startR = 0.8f; cfg.startG = 0.8f; cfg.startB = 1.0f; cfg.startA = 1.0f;
    cfg.endR   = 0.4f; cfg.endG   = 0.4f; cfg.endB   = 1.0f; cfg.endA   = 0.0f;
    return cfg;
}

} // namespace subspace
