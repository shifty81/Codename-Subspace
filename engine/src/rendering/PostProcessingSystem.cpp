#include "rendering/PostProcessingSystem.h"
#include "core/ecs/EntityManager.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// PostProcessingConfig
// ---------------------------------------------------------------------------

int PostProcessingConfig::GetEnabledEffectCount() const {
    int count = 0;
    if (bloom.enabled) ++count;
    if (hdr.enabled) ++count;
    if (toneMapping.enabled) ++count;
    if (vignette.enabled) ++count;
    if (chromaticAberration.enabled) ++count;
    if (fxaa.enabled) ++count;
    return count;
}

bool PostProcessingConfig::IsEffectEnabled(PostProcessEffect effect) const {
    switch (effect) {
    case PostProcessEffect::Bloom:                return bloom.enabled;
    case PostProcessEffect::HDR:                  return hdr.enabled;
    case PostProcessEffect::ToneMapping:          return toneMapping.enabled;
    case PostProcessEffect::Vignette:             return vignette.enabled;
    case PostProcessEffect::ChromaticAberration:  return chromaticAberration.enabled;
    case PostProcessEffect::FXAA:                 return fxaa.enabled;
    }
    return false;
}

void PostProcessingConfig::SetEffectEnabled(PostProcessEffect effect, bool enabled) {
    switch (effect) {
    case PostProcessEffect::Bloom:                bloom.enabled = enabled; break;
    case PostProcessEffect::HDR:                  hdr.enabled = enabled; break;
    case PostProcessEffect::ToneMapping:          toneMapping.enabled = enabled; break;
    case PostProcessEffect::Vignette:             vignette.enabled = enabled; break;
    case PostProcessEffect::ChromaticAberration:  chromaticAberration.enabled = enabled; break;
    case PostProcessEffect::FXAA:                 fxaa.enabled = enabled; break;
    }
}

void PostProcessingConfig::ApplyPreset(QualityPreset preset) {
    switch (preset) {
    case QualityPreset::Low:
        bloom.enabled = false;
        hdr.enabled = true;
        hdr.exposure = 1.0f;
        hdr.gamma = 2.2f;
        toneMapping.enabled = true;
        toneMapping.op = ToneMappingSettings::Operator::Reinhard;
        vignette.enabled = false;
        chromaticAberration.enabled = false;
        fxaa.enabled = false;
        break;

    case QualityPreset::Medium:
        bloom.enabled = true;
        bloom.threshold = 1.0f;
        bloom.intensity = 0.5f;
        bloom.blurPasses = 2;
        hdr.enabled = true;
        hdr.exposure = 1.0f;
        hdr.gamma = 2.2f;
        toneMapping.enabled = true;
        toneMapping.op = ToneMappingSettings::Operator::ACES;
        vignette.enabled = true;
        vignette.intensity = 0.2f;
        chromaticAberration.enabled = false;
        fxaa.enabled = true;
        fxaa.edgeThreshold = 0.166f;
        break;

    case QualityPreset::High:
        bloom.enabled = true;
        bloom.threshold = 0.8f;
        bloom.intensity = 1.0f;
        bloom.blurPasses = 4;
        hdr.enabled = true;
        hdr.exposure = 1.0f;
        hdr.gamma = 2.2f;
        hdr.autoExposure = true;
        toneMapping.enabled = true;
        toneMapping.op = ToneMappingSettings::Operator::ACES;
        vignette.enabled = true;
        vignette.intensity = 0.3f;
        chromaticAberration.enabled = true;
        chromaticAberration.intensity = 0.003f;
        fxaa.enabled = true;
        fxaa.edgeThreshold = 0.125f;
        break;

    case QualityPreset::Ultra:
        bloom.enabled = true;
        bloom.threshold = 0.6f;
        bloom.intensity = 1.2f;
        bloom.blurPasses = 6;
        bloom.blurRadius = 1.5f;
        hdr.enabled = true;
        hdr.exposure = 1.0f;
        hdr.gamma = 2.2f;
        hdr.autoExposure = true;
        hdr.adaptSpeed = 0.5f;
        toneMapping.enabled = true;
        toneMapping.op = ToneMappingSettings::Operator::ACES;
        vignette.enabled = true;
        vignette.intensity = 0.3f;
        vignette.softness = 0.4f;
        chromaticAberration.enabled = true;
        chromaticAberration.intensity = 0.005f;
        fxaa.enabled = true;
        fxaa.edgeThreshold = 0.0625f;
        fxaa.edgeThresholdMin = 0.0312f;
        break;
    }
}

// ---------------------------------------------------------------------------
// PostProcessingComponent serialization
// ---------------------------------------------------------------------------

ComponentData PostProcessingComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "PostProcessingComponent";
    cd.data["priority"] = std::to_string(priority);

    // Bloom
    cd.data["bloom_enabled"]     = config.bloom.enabled ? "true" : "false";
    cd.data["bloom_threshold"]   = std::to_string(config.bloom.threshold);
    cd.data["bloom_intensity"]   = std::to_string(config.bloom.intensity);
    cd.data["bloom_blurPasses"]  = std::to_string(config.bloom.blurPasses);
    cd.data["bloom_blurRadius"]  = std::to_string(config.bloom.blurRadius);

    // HDR
    cd.data["hdr_enabled"]       = config.hdr.enabled ? "true" : "false";
    cd.data["hdr_exposure"]      = std::to_string(config.hdr.exposure);
    cd.data["hdr_gamma"]         = std::to_string(config.hdr.gamma);
    cd.data["hdr_autoExposure"]  = config.hdr.autoExposure ? "true" : "false";
    cd.data["hdr_adaptSpeed"]    = std::to_string(config.hdr.adaptSpeed);

    // Tone mapping
    cd.data["tm_enabled"]        = config.toneMapping.enabled ? "true" : "false";
    cd.data["tm_operator"]       = std::to_string(static_cast<int>(config.toneMapping.op));
    cd.data["tm_whitePoint"]     = std::to_string(config.toneMapping.whitePoint);

    // Vignette
    cd.data["vig_enabled"]       = config.vignette.enabled ? "true" : "false";
    cd.data["vig_intensity"]     = std::to_string(config.vignette.intensity);
    cd.data["vig_radius"]        = std::to_string(config.vignette.radius);
    cd.data["vig_softness"]      = std::to_string(config.vignette.softness);

    // Chromatic aberration
    cd.data["ca_enabled"]        = config.chromaticAberration.enabled ? "true" : "false";
    cd.data["ca_intensity"]      = std::to_string(config.chromaticAberration.intensity);

    // FXAA
    cd.data["fxaa_enabled"]      = config.fxaa.enabled ? "true" : "false";
    cd.data["fxaa_edgeThreshold"]    = std::to_string(config.fxaa.edgeThreshold);
    cd.data["fxaa_edgeThresholdMin"] = std::to_string(config.fxaa.edgeThresholdMin);

    return cd;
}

void PostProcessingComponent::Deserialize(const ComponentData& data) {
    auto getStr = [&](const std::string& key) -> std::string {
        auto it = data.data.find(key);
        return it != data.data.end() ? it->second : "";
    };
    auto getInt = [&](const std::string& key, int def = 0) -> int {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };
    auto getFloat = [&](const std::string& key, float def = 0.0f) -> float {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    };

    priority = getInt("priority", 0);

    // Bloom
    config.bloom.enabled    = getStr("bloom_enabled") == "true";
    config.bloom.threshold  = getFloat("bloom_threshold", 0.8f);
    config.bloom.intensity  = getFloat("bloom_intensity", 1.0f);
    config.bloom.blurPasses = getInt("bloom_blurPasses", 4);
    config.bloom.blurRadius = getFloat("bloom_blurRadius", 1.0f);

    // HDR
    config.hdr.enabled      = getStr("hdr_enabled") == "true";
    config.hdr.exposure     = getFloat("hdr_exposure", 1.0f);
    config.hdr.gamma        = getFloat("hdr_gamma", 2.2f);
    config.hdr.autoExposure = getStr("hdr_autoExposure") == "true";
    config.hdr.adaptSpeed   = getFloat("hdr_adaptSpeed", 1.0f);

    // Tone mapping
    config.toneMapping.enabled    = getStr("tm_enabled") == "true";
    config.toneMapping.op         = static_cast<ToneMappingSettings::Operator>(getInt("tm_operator", 1));
    config.toneMapping.whitePoint = getFloat("tm_whitePoint", 11.2f);

    // Vignette
    config.vignette.enabled   = getStr("vig_enabled") == "true";
    config.vignette.intensity = getFloat("vig_intensity", 0.3f);
    config.vignette.radius    = getFloat("vig_radius", 0.8f);
    config.vignette.softness  = getFloat("vig_softness", 0.5f);

    // Chromatic aberration
    config.chromaticAberration.enabled   = getStr("ca_enabled") == "true";
    config.chromaticAberration.intensity = getFloat("ca_intensity", 0.005f);

    // FXAA
    config.fxaa.enabled          = getStr("fxaa_enabled") == "true";
    config.fxaa.edgeThreshold    = getFloat("fxaa_edgeThreshold", 0.125f);
    config.fxaa.edgeThresholdMin = getFloat("fxaa_edgeThresholdMin", 0.0625f);
}

// ---------------------------------------------------------------------------
// PostProcessingSystem
// ---------------------------------------------------------------------------

PostProcessingSystem::PostProcessingSystem() : SystemBase("PostProcessingSystem") {}

void PostProcessingSystem::Initialize() {
    _activeComponentCount = 0;
}

void PostProcessingSystem::Update(float /*deltaTime*/) {
    if (!_isEnabled) return;

    _activeComponentCount = 0;

    if (_entityManager) {
        auto components = _entityManager->GetAllComponents<PostProcessingComponent>();
        _activeComponentCount = static_cast<int>(components.size());
    }
}

void PostProcessingSystem::Shutdown() {
    _activeComponentCount = 0;
}

void PostProcessingSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

const PostProcessingConfig& PostProcessingSystem::GetGlobalConfig() const {
    return _globalConfig;
}

void PostProcessingSystem::SetGlobalConfig(const PostProcessingConfig& cfg) {
    _globalConfig = cfg;
}

void PostProcessingSystem::ApplyGlobalPreset(PostProcessingConfig::QualityPreset preset) {
    _globalConfig.ApplyPreset(preset);
}

int PostProcessingSystem::GetActiveComponentCount() const {
    return _activeComponentCount;
}

} // namespace subspace
