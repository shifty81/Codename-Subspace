#include "combat/StatusEffectSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// StatusEffect
// ---------------------------------------------------------------------------

bool StatusEffect::IsActive() const {
    return remainingTime > 0.0f;
}

float StatusEffect::GetRemainingPercent() const {
    if (duration <= 0.0f) return 0.0f;
    float percent = (remainingTime / duration) * 100.0f;
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    return percent;
}

std::string StatusEffect::GetEffectName(StatusEffectType type) {
    switch (type) {
        case StatusEffectType::EMPDisruption:  return "EMP Disruption";
        case StatusEffectType::FireDOT:        return "Fire";
        case StatusEffectType::RadiationDOT:   return "Radiation";
        case StatusEffectType::ShieldDrain:    return "Shield Drain";
        case StatusEffectType::EngineJam:      return "Engine Jam";
        case StatusEffectType::SensorScramble: return "Sensor Scramble";
    }
    return "Unknown";
}

float StatusEffect::GetDefaultDuration(StatusEffectType type) {
    switch (type) {
        case StatusEffectType::EMPDisruption:  return 3.0f;
        case StatusEffectType::FireDOT:        return 8.0f;
        case StatusEffectType::RadiationDOT:   return 10.0f;
        case StatusEffectType::ShieldDrain:    return 6.0f;
        case StatusEffectType::EngineJam:      return 5.0f;
        case StatusEffectType::SensorScramble: return 7.0f;
    }
    return 5.0f;
}

float StatusEffect::GetDefaultMagnitude(StatusEffectType type) {
    switch (type) {
        case StatusEffectType::EMPDisruption:  return 0.0f;
        case StatusEffectType::FireDOT:        return 15.0f;
        case StatusEffectType::RadiationDOT:   return 10.0f;
        case StatusEffectType::ShieldDrain:    return 20.0f;
        case StatusEffectType::EngineJam:      return 50.0f;
        case StatusEffectType::SensorScramble: return 40.0f;
    }
    return 10.0f;
}

// ---------------------------------------------------------------------------
// StatusEffectComponent
// ---------------------------------------------------------------------------

bool StatusEffectComponent::ApplyEffect(const StatusEffect& effect) {
    if (isImmune) return false;
    if (activeEffects.size() >= kMaxEffects) return false;

    StatusEffect applied = effect;
    applied.magnitude *= resistanceMultiplier;
    activeEffects.push_back(applied);
    return true;
}

void StatusEffectComponent::RemoveEffectsByType(StatusEffectType type) {
    activeEffects.erase(
        std::remove_if(activeEffects.begin(), activeEffects.end(),
                        [type](const StatusEffect& e) { return e.type == type; }),
        activeEffects.end());
}

void StatusEffectComponent::ClearExpired() {
    activeEffects.erase(
        std::remove_if(activeEffects.begin(), activeEffects.end(),
                        [](const StatusEffect& e) { return e.remainingTime <= 0.0f; }),
        activeEffects.end());
}

bool StatusEffectComponent::HasEffect(StatusEffectType type) const {
    for (const auto& e : activeEffects) {
        if (e.type == type) return true;
    }
    return false;
}

float StatusEffectComponent::GetEffectMagnitude(StatusEffectType type) const {
    float maxMag = 0.0f;
    for (const auto& e : activeEffects) {
        if (e.type == type && e.magnitude > maxMag) {
            maxMag = e.magnitude;
        }
    }
    return maxMag;
}

size_t StatusEffectComponent::GetActiveCount() const {
    return activeEffects.size();
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

static std::string SerializeEffectType(StatusEffectType t) {
    return std::to_string(static_cast<int>(t));
}

static StatusEffectType EffectTypeFromInt(int v) {
    switch (v) {
        case 0: return StatusEffectType::EMPDisruption;
        case 1: return StatusEffectType::FireDOT;
        case 2: return StatusEffectType::RadiationDOT;
        case 3: return StatusEffectType::ShieldDrain;
        case 4: return StatusEffectType::EngineJam;
        case 5: return StatusEffectType::SensorScramble;
    }
    return StatusEffectType::EMPDisruption;
}

ComponentData StatusEffectComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "StatusEffectComponent";
    cd.data["isImmune"] = isImmune ? "true" : "false";
    cd.data["resistanceMultiplier"] = std::to_string(resistanceMultiplier);
    cd.data["effectCount"] = std::to_string(activeEffects.size());

    for (size_t i = 0; i < activeEffects.size(); ++i) {
        const auto& e = activeEffects[i];
        std::string p = "effect_" + std::to_string(i) + "_";
        cd.data[p + "type"]          = SerializeEffectType(e.type);
        cd.data[p + "duration"]      = std::to_string(e.duration);
        cd.data[p + "remainingTime"] = std::to_string(e.remainingTime);
        cd.data[p + "tickInterval"]  = std::to_string(e.tickInterval);
        cd.data[p + "tickTimer"]     = std::to_string(e.tickTimer);
        cd.data[p + "magnitude"]     = std::to_string(e.magnitude);
    }
    return cd;
}

void StatusEffectComponent::Deserialize(const ComponentData& data) {
    activeEffects.clear();

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

    isImmune = getStr("isImmune") == "true";
    resistanceMultiplier = getFloat("resistanceMultiplier", 1.0f);

    int count = getInt("effectCount", 0);
    for (int i = 0; i < count; ++i) {
        std::string p = "effect_" + std::to_string(i) + "_";
        StatusEffect e;
        e.type          = EffectTypeFromInt(getInt(p + "type", 0));
        e.duration      = getFloat(p + "duration", 5.0f);
        e.remainingTime = getFloat(p + "remainingTime", 5.0f);
        e.tickInterval  = getFloat(p + "tickInterval", 1.0f);
        e.tickTimer     = getFloat(p + "tickTimer", 0.0f);
        e.magnitude     = getFloat(p + "magnitude", 10.0f);
        activeEffects.push_back(e);
    }
}

// ---------------------------------------------------------------------------
// StatusEffectSystem
// ---------------------------------------------------------------------------

StatusEffectSystem::StatusEffectSystem() : SystemBase("StatusEffectSystem") {}

StatusEffectSystem::StatusEffectSystem(EntityManager& entityManager)
    : SystemBase("StatusEffectSystem")
    , _entityManager(&entityManager)
{
}

void StatusEffectSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto components = _entityManager->GetAllComponents<StatusEffectComponent>();
    for (auto* comp : components) {
        for (auto& effect : comp->activeEffects) {
            effect.remainingTime -= deltaTime;
            effect.tickTimer += deltaTime;

            if (effect.tickTimer >= effect.tickInterval) {
                effect.tickTimer -= effect.tickInterval;
            }
        }
        comp->ClearExpired();
    }
}

} // namespace subspace
