#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/persistence/SaveGameManager.h"
#include "core/Math.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// Shadow mapping types
// ---------------------------------------------------------------------------

enum class ShadowQuality { Off, Low, Medium, High, Ultra };

enum class ShadowType { None, Hard, Soft, PCF, VSM };

enum class LightType { Directional, Point, Spot };

// ---------------------------------------------------------------------------
// Shadow map configuration
// ---------------------------------------------------------------------------

struct ShadowMapConfig {
    int   resolution    = 1024;      // shadow map texture size (power of 2)
    float nearPlane     = 0.1f;      // shadow frustum near
    float farPlane      = 100.0f;    // shadow frustum far
    float bias          = 0.005f;    // depth bias to reduce shadow acne
    float normalBias    = 0.02f;     // normal-direction bias
    int   cascadeCount  = 3;         // number of CSM cascades (directional)
    float cascadeSplitLambda = 0.75f; // cascade split ratio (PSSM lambda)
    ShadowType shadowType = ShadowType::PCF;

    /// Compute the shadow map resolution for a given quality preset.
    static int ResolutionForQuality(ShadowQuality quality);

    /// Apply a quality preset to this config.
    void ApplyQuality(ShadowQuality quality);
};

// ---------------------------------------------------------------------------
// Light source with shadow capability
// ---------------------------------------------------------------------------

struct LightSource {
    LightType type        = LightType::Directional;
    Vector3   position    = {0.0f, 50.0f, 0.0f};
    Vector3   direction   = {0.0f, -1.0f, 0.0f};
    float     intensity   = 1.0f;
    float     colorR = 1.0f, colorG = 1.0f, colorB = 1.0f;
    bool      castsShadows = true;

    // Spot light parameters
    float innerConeAngleDeg = 30.0f;
    float outerConeAngleDeg = 45.0f;

    // Point light parameters
    float radius = 50.0f;
};

// ---------------------------------------------------------------------------
// Shadow component — attach to a light-bearing entity
// ---------------------------------------------------------------------------

struct ShadowComponent : public IComponent {
    LightSource light;
    ShadowMapConfig shadowConfig;

    /// Whether this light's shadow map needs rebuilding.
    bool isDirty = true;

    /// Unique shadow map ID (assigned by system).
    uint64_t shadowMapId = 0;

    ComponentData Serialize() const;
    void Deserialize(const ComponentData& data);
};

// ---------------------------------------------------------------------------
// Shadow caster component — attach to entities that cast shadows
// ---------------------------------------------------------------------------

struct ShadowCasterComponent : public IComponent {
    bool castsShadows = true;
    bool receivesShadows = true;

    /// Bounding sphere radius for shadow culling.
    float boundingRadius = 1.0f;
};

// ---------------------------------------------------------------------------
// Shadow system — manages shadow map generation and updates
// ---------------------------------------------------------------------------

class EntityManager;

class ShadowSystem : public SystemBase {
public:
    ShadowSystem();

    void Initialize() override;
    void Update(float deltaTime) override;
    void Shutdown() override;

    /// Set the entity manager used to query components.
    void SetEntityManager(EntityManager* em);

    /// Get/set the global shadow quality.
    ShadowQuality GetShadowQuality() const;
    void SetShadowQuality(ShadowQuality quality);

    /// Get the number of active shadow-casting lights.
    int GetActiveShadowLightCount() const;

    /// Get the number of shadow caster entities.
    int GetShadowCasterCount() const;

    /// Get the total number of shadow maps generated.
    int GetShadowMapCount() const;

    /// Mark all shadow maps as needing rebuild.
    void InvalidateAllShadowMaps();

private:
    EntityManager* _entityManager = nullptr;
    ShadowQuality _quality = ShadowQuality::Medium;
    int _activeShadowLightCount = 0;
    int _shadowCasterCount = 0;
    int _shadowMapCount = 0;
    uint64_t _nextShadowMapId = 1;
};

} // namespace subspace
