#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/persistence/SaveGameManager.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// Particle — a single short-lived visual element
// ---------------------------------------------------------------------------

struct Particle {
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float velX = 0.0f, velY = 0.0f, velZ = 0.0f;

    // Color (RGBA, each [0,1])
    float colorR = 1.0f, colorG = 1.0f, colorB = 1.0f, colorA = 1.0f;
    float endColorR = 0.0f, endColorG = 0.0f, endColorB = 0.0f, endColorA = 0.0f;

    float size = 1.0f;
    float lifetime = 1.0f;   // total seconds
    float age = 0.0f;        // seconds elapsed

    /// True while the particle has time left.
    bool IsAlive() const;

    /// 0 → just born, 1 → about to die.
    float GetNormalizedAge() const;

    /// Current interpolated color (lerp start→end by normalized age).
    void GetCurrentColor(float& r, float& g, float& b, float& a) const;
};

// ---------------------------------------------------------------------------
// Emitter configuration
// ---------------------------------------------------------------------------

enum class EmitterShape { Point, Sphere, Cone, Box };

struct ParticleEmitterConfig {
    EmitterShape shape = EmitterShape::Point;

    float emitRate = 10.0f;          // particles per second
    int   maxParticles = 200;

    float minLifetime = 0.5f;
    float maxLifetime = 2.0f;
    float minSpeed = 1.0f;
    float maxSpeed = 5.0f;
    float minSize = 0.1f;
    float maxSize = 0.5f;

    float gravityY = 0.0f;          // constant downward acceleration

    // Start colour
    float startR = 1.0f, startG = 1.0f, startB = 1.0f, startA = 1.0f;
    // End colour (particles fade towards this)
    float endR = 1.0f, endG = 1.0f, endB = 1.0f, endA = 0.0f;

    // Cone shape: half-angle in degrees
    float coneAngleDeg = 30.0f;

    // Sphere shape: emit radius
    float sphereRadius = 1.0f;

    // Box shape: half-extents
    float boxHalfW = 1.0f, boxHalfH = 1.0f, boxHalfD = 1.0f;
};

// ---------------------------------------------------------------------------
// Particle emitter
// ---------------------------------------------------------------------------

class ParticleEmitter {
public:
    ParticleEmitter();
    explicit ParticleEmitter(const std::string& id,
                             const ParticleEmitterConfig& cfg = {});

    const std::string& GetId() const;
    bool IsActive() const;
    void SetActive(bool active);

    void SetPosition(float x, float y, float z);
    void GetPosition(float& x, float& y, float& z) const;

    const ParticleEmitterConfig& GetConfig() const;
    void SetConfig(const ParticleEmitterConfig& cfg);

    /// Advance emitter by dt seconds — spawns, updates, and prunes particles.
    void Update(float dt);

    /// Burst-emit a specific number of particles immediately.
    void Emit(int count);

    /// Remove all particles and reset accumulator.
    void Reset();

    int GetAliveCount() const;
    const std::vector<Particle>& GetParticles() const;

    /// Deterministic seed (0 = use default sequence).
    void SetSeed(uint32_t seed);

private:
    float RandFloat();           // [0, 1)
    float RandRange(float lo, float hi);
    Particle SpawnOne();

    std::string _id;
    ParticleEmitterConfig _config;
    float _posX = 0.0f, _posY = 0.0f, _posZ = 0.0f;

    std::vector<Particle> _particles;
    float _emitAccumulator = 0.0f;
    bool  _active = true;

    uint32_t _seed = 12345;
};

// ---------------------------------------------------------------------------
// Particle component — attach to an entity for visual effects
// ---------------------------------------------------------------------------

struct ParticleComponent : public IComponent {
    std::vector<ParticleEmitter> emitters;

    void AddEmitter(const ParticleEmitter& emitter);
    bool RemoveEmitter(const std::string& id);
    ParticleEmitter* GetEmitter(const std::string& id);
    int GetTotalParticleCount() const;
    int GetEmitterCount() const;

    void StopAll();
    void ResumeAll();
};

// ---------------------------------------------------------------------------
// Particle system — updates all ParticleComponents each frame
// ---------------------------------------------------------------------------

class ParticleSystem : public SystemBase {
public:
    ParticleSystem();

    void Initialize() override;
    void Update(float deltaTime) override;
    void Shutdown() override;

    // ---- preset emitter configs ----
    static ParticleEmitterConfig CreateExplosionPreset();
    static ParticleEmitterConfig CreateEngineThrustPreset();
    static ParticleEmitterConfig CreateShieldHitPreset();
    static ParticleEmitterConfig CreateMiningPreset();
    static ParticleEmitterConfig CreateHyperdrivePreset();
};

} // namespace subspace
