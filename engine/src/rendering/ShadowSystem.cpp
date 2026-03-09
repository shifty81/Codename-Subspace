#include "rendering/ShadowSystem.h"
#include "core/ecs/EntityManager.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// ShadowMapConfig
// ---------------------------------------------------------------------------

int ShadowMapConfig::ResolutionForQuality(ShadowQuality quality) {
    switch (quality) {
    case ShadowQuality::Off:    return 0;
    case ShadowQuality::Low:    return 512;
    case ShadowQuality::Medium: return 1024;
    case ShadowQuality::High:   return 2048;
    case ShadowQuality::Ultra:  return 4096;
    }
    return 1024;
}

void ShadowMapConfig::ApplyQuality(ShadowQuality quality) {
    resolution = ResolutionForQuality(quality);

    switch (quality) {
    case ShadowQuality::Off:
        shadowType = ShadowType::None;
        cascadeCount = 0;
        break;
    case ShadowQuality::Low:
        shadowType = ShadowType::Hard;
        cascadeCount = 1;
        bias = 0.008f;
        normalBias = 0.04f;
        break;
    case ShadowQuality::Medium:
        shadowType = ShadowType::PCF;
        cascadeCount = 2;
        bias = 0.005f;
        normalBias = 0.02f;
        break;
    case ShadowQuality::High:
        shadowType = ShadowType::PCF;
        cascadeCount = 3;
        bias = 0.003f;
        normalBias = 0.015f;
        break;
    case ShadowQuality::Ultra:
        shadowType = ShadowType::VSM;
        cascadeCount = 4;
        bias = 0.002f;
        normalBias = 0.01f;
        break;
    }
}

// ---------------------------------------------------------------------------
// ShadowComponent serialization
// ---------------------------------------------------------------------------

static std::string LightTypeToString(LightType t) {
    switch (t) {
    case LightType::Directional: return "Directional";
    case LightType::Point:       return "Point";
    case LightType::Spot:        return "Spot";
    }
    return "Directional";
}

static LightType LightTypeFromString(const std::string& s) {
    if (s == "Point") return LightType::Point;
    if (s == "Spot")  return LightType::Spot;
    return LightType::Directional;
}

static std::string ShadowTypeToString(ShadowType t) {
    switch (t) {
    case ShadowType::None: return "None";
    case ShadowType::Hard: return "Hard";
    case ShadowType::Soft: return "Soft";
    case ShadowType::PCF:  return "PCF";
    case ShadowType::VSM:  return "VSM";
    }
    return "PCF";
}

static ShadowType ShadowTypeFromString(const std::string& s) {
    if (s == "None") return ShadowType::None;
    if (s == "Hard") return ShadowType::Hard;
    if (s == "Soft") return ShadowType::Soft;
    if (s == "VSM")  return ShadowType::VSM;
    return ShadowType::PCF;
}

ComponentData ShadowComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "ShadowComponent";

    // Light
    cd.data["light_type"]      = LightTypeToString(light.type);
    cd.data["light_posX"]      = std::to_string(light.position.x);
    cd.data["light_posY"]      = std::to_string(light.position.y);
    cd.data["light_posZ"]      = std::to_string(light.position.z);
    cd.data["light_dirX"]      = std::to_string(light.direction.x);
    cd.data["light_dirY"]      = std::to_string(light.direction.y);
    cd.data["light_dirZ"]      = std::to_string(light.direction.z);
    cd.data["light_intensity"] = std::to_string(light.intensity);
    cd.data["light_colorR"]    = std::to_string(light.colorR);
    cd.data["light_colorG"]    = std::to_string(light.colorG);
    cd.data["light_colorB"]    = std::to_string(light.colorB);
    cd.data["light_castsShadows"] = light.castsShadows ? "true" : "false";
    cd.data["light_innerCone"] = std::to_string(light.innerConeAngleDeg);
    cd.data["light_outerCone"] = std::to_string(light.outerConeAngleDeg);
    cd.data["light_radius"]    = std::to_string(light.radius);

    // Shadow config
    cd.data["shadow_resolution"]   = std::to_string(shadowConfig.resolution);
    cd.data["shadow_nearPlane"]    = std::to_string(shadowConfig.nearPlane);
    cd.data["shadow_farPlane"]     = std::to_string(shadowConfig.farPlane);
    cd.data["shadow_bias"]         = std::to_string(shadowConfig.bias);
    cd.data["shadow_normalBias"]   = std::to_string(shadowConfig.normalBias);
    cd.data["shadow_cascadeCount"] = std::to_string(shadowConfig.cascadeCount);
    cd.data["shadow_cascadeLambda"]= std::to_string(shadowConfig.cascadeSplitLambda);
    cd.data["shadow_type"]         = ShadowTypeToString(shadowConfig.shadowType);

    cd.data["isDirty"] = isDirty ? "true" : "false";

    return cd;
}

void ShadowComponent::Deserialize(const ComponentData& data) {
    auto getStr = [&](const std::string& key) -> std::string {
        auto it = data.data.find(key);
        return it != data.data.end() ? it->second : "";
    };
    auto getFloat = [&](const std::string& key, float def = 0.0f) -> float {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    };
    auto getInt = [&](const std::string& key, int def = 0) -> int {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };

    // Light
    light.type          = LightTypeFromString(getStr("light_type"));
    light.position.x    = getFloat("light_posX", 0.0f);
    light.position.y    = getFloat("light_posY", 50.0f);
    light.position.z    = getFloat("light_posZ", 0.0f);
    light.direction.x   = getFloat("light_dirX", 0.0f);
    light.direction.y   = getFloat("light_dirY", -1.0f);
    light.direction.z   = getFloat("light_dirZ", 0.0f);
    light.intensity     = getFloat("light_intensity", 1.0f);
    light.colorR        = getFloat("light_colorR", 1.0f);
    light.colorG        = getFloat("light_colorG", 1.0f);
    light.colorB        = getFloat("light_colorB", 1.0f);
    light.castsShadows  = getStr("light_castsShadows") != "false";
    light.innerConeAngleDeg = getFloat("light_innerCone", 30.0f);
    light.outerConeAngleDeg = getFloat("light_outerCone", 45.0f);
    light.radius        = getFloat("light_radius", 50.0f);

    // Shadow config
    shadowConfig.resolution   = getInt("shadow_resolution", 1024);
    shadowConfig.nearPlane    = getFloat("shadow_nearPlane", 0.1f);
    shadowConfig.farPlane     = getFloat("shadow_farPlane", 100.0f);
    shadowConfig.bias         = getFloat("shadow_bias", 0.005f);
    shadowConfig.normalBias   = getFloat("shadow_normalBias", 0.02f);
    shadowConfig.cascadeCount = getInt("shadow_cascadeCount", 3);
    shadowConfig.cascadeSplitLambda = getFloat("shadow_cascadeLambda", 0.75f);
    shadowConfig.shadowType   = ShadowTypeFromString(getStr("shadow_type"));

    isDirty = getStr("isDirty") != "false";
}

// ---------------------------------------------------------------------------
// ShadowSystem
// ---------------------------------------------------------------------------

ShadowSystem::ShadowSystem() : SystemBase("ShadowSystem") {}

void ShadowSystem::Initialize() {
    _activeShadowLightCount = 0;
    _shadowCasterCount = 0;
    _shadowMapCount = 0;
}

void ShadowSystem::Update(float /*deltaTime*/) {
    if (!_isEnabled) return;

    _activeShadowLightCount = 0;
    _shadowCasterCount = 0;
    _shadowMapCount = 0;

    if (!_entityManager) return;

    if (_quality == ShadowQuality::Off) return;

    // Count and update shadow-casting lights
    auto shadowComps = _entityManager->GetAllComponents<ShadowComponent>();
    for (auto* comp : shadowComps) {
        if (!comp->light.castsShadows) continue;
        ++_activeShadowLightCount;

        // Assign shadow map ID if needed
        if (comp->shadowMapId == 0) {
            comp->shadowMapId = _nextShadowMapId++;
            comp->isDirty = true;
        }

        // Apply current quality to shadow config
        comp->shadowConfig.ApplyQuality(_quality);

        // Count shadow maps (cascaded for directional)
        if (comp->light.type == LightType::Directional) {
            _shadowMapCount += comp->shadowConfig.cascadeCount;
        } else {
            _shadowMapCount += 1;
        }

        // Clear dirty flag after processing
        comp->isDirty = false;
    }

    // Count shadow casters
    auto casterComps = _entityManager->GetAllComponents<ShadowCasterComponent>();
    for (auto* comp : casterComps) {
        if (comp->castsShadows) {
            ++_shadowCasterCount;
        }
    }
}

void ShadowSystem::Shutdown() {
    _activeShadowLightCount = 0;
    _shadowCasterCount = 0;
    _shadowMapCount = 0;
}

void ShadowSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

ShadowQuality ShadowSystem::GetShadowQuality() const {
    return _quality;
}

void ShadowSystem::SetShadowQuality(ShadowQuality quality) {
    if (_quality != quality) {
        _quality = quality;
        InvalidateAllShadowMaps();
    }
}

int ShadowSystem::GetActiveShadowLightCount() const {
    return _activeShadowLightCount;
}

int ShadowSystem::GetShadowCasterCount() const {
    return _shadowCasterCount;
}

int ShadowSystem::GetShadowMapCount() const {
    return _shadowMapCount;
}

void ShadowSystem::InvalidateAllShadowMaps() {
    if (!_entityManager) return;

    auto shadowComps = _entityManager->GetAllComponents<ShadowComponent>();
    for (auto* comp : shadowComps) {
        comp->isDirty = true;
    }
}

} // namespace subspace
