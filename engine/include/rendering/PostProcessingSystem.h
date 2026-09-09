#pragma once

#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>

namespace subspace {

// ---------------------------------------------------------------------------
// Post-processing effect types
// ---------------------------------------------------------------------------

enum class PostProcessEffect {
    Bloom,
    HDR,
    ToneMapping,
    Vignette,
    ChromaticAberration,
    FXAA
};

// ---------------------------------------------------------------------------
// Individual effect configuration
// ---------------------------------------------------------------------------

struct BloomSettings {
    float threshold   = 0.8f;   // brightness threshold for bloom
    float intensity   = 1.0f;   // bloom strength
    int   blurPasses  = 4;      // number of Gaussian blur passes
    float blurRadius  = 1.0f;   // blur sample radius
    bool  enabled     = false;
};

struct HDRSettings {
    float exposure    = 1.0f;   // exposure multiplier
    float gamma       = 2.2f;   // gamma correction value
    bool  autoExposure = false; // automatic exposure adaptation
    float adaptSpeed  = 1.0f;   // speed of auto-exposure adaptation
    bool  enabled     = false;
};

struct ToneMappingSettings {
    enum class Operator { Reinhard, ACES, Filmic, Uncharted2 };

    Operator op       = Operator::ACES;
    float whitePoint  = 11.2f;  // white point for Uncharted2
    bool  enabled     = false;
};

struct VignetteSettings {
    float intensity   = 0.3f;   // vignette strength [0,1]
    float radius      = 0.8f;   // inner radius before fade
    float softness    = 0.5f;   // fade transition width
    bool  enabled     = false;
};

struct ChromaticAberrationSettings {
    float intensity   = 0.005f; // separation strength
    bool  enabled     = false;
};

struct FXAASettings {
    float edgeThreshold    = 0.125f; // edge detection threshold
    float edgeThresholdMin = 0.0625f;
    bool  enabled          = false;
};

// ---------------------------------------------------------------------------
// Post-processing pipeline configuration
// ---------------------------------------------------------------------------

struct PostProcessingConfig {
    BloomSettings              bloom;
    HDRSettings                hdr;
    ToneMappingSettings        toneMapping;
    VignetteSettings           vignette;
    ChromaticAberrationSettings chromaticAberration;
    FXAASettings               fxaa;

    /// Returns the number of effects currently enabled.
    int GetEnabledEffectCount() const;

    /// Check if a specific effect is enabled.
    bool IsEffectEnabled(PostProcessEffect effect) const;

    /// Enable / disable a specific effect.
    void SetEffectEnabled(PostProcessEffect effect, bool enabled);

    /// Apply a predefined quality preset.
    enum class QualityPreset { Low, Medium, High, Ultra };
    void ApplyPreset(QualityPreset preset);
};

// ---------------------------------------------------------------------------
// Post-processing component — attach to a camera / viewport entity
// ---------------------------------------------------------------------------

struct PostProcessingComponent : public IComponent {
    PostProcessingConfig config;

    /// Per-entity override priority (higher = applied later).
    int priority = 0;

    ComponentData Serialize() const;
    void Deserialize(const ComponentData& data);
};

// ---------------------------------------------------------------------------
// Post-processing system
// ---------------------------------------------------------------------------

class EntityManager;

class PostProcessingSystem : public SystemBase {
public:
    PostProcessingSystem();

    void Initialize() override;
    void Update(float deltaTime) override;
    void Shutdown() override;

    /// Set the entity manager used to query PostProcessingComponents.
    void SetEntityManager(EntityManager* em);

    /// Get/set the global pipeline config (used when no per-entity overrides).
    const PostProcessingConfig& GetGlobalConfig() const;
    void SetGlobalConfig(const PostProcessingConfig& cfg);

    /// Apply a quality preset to the global config.
    void ApplyGlobalPreset(PostProcessingConfig::QualityPreset preset);

    /// Get the number of active post-processing components.
    int GetActiveComponentCount() const;

private:
    EntityManager* _entityManager = nullptr;
    PostProcessingConfig _globalConfig;
    int _activeComponentCount = 0;
};

} // namespace subspace
