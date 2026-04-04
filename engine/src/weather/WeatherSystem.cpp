#include "weather/WeatherSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// WeatherModifiers helpers
// ---------------------------------------------------------------------------

std::string WeatherModifiers::GetWeatherTypeName(SpaceWeatherType type) {
    switch (type) {
        case SpaceWeatherType::Clear:               return "Clear";
        case SpaceWeatherType::IonStorm:             return "Ion Storm";
        case SpaceWeatherType::SolarFlare:           return "Solar Flare";
        case SpaceWeatherType::NebulaDrift:          return "Nebula Drift";
        case SpaceWeatherType::MeteorShower:         return "Meteor Shower";
        case SpaceWeatherType::RadiationBurst:       return "Radiation Burst";
        case SpaceWeatherType::GravityAnomaly:       return "Gravity Anomaly";
        case SpaceWeatherType::ElectromagneticPulse: return "Electromagnetic Pulse";
    }
    return "Unknown";
}

std::string WeatherModifiers::GetPhaseName(WeatherPhase phase) {
    switch (phase) {
        case WeatherPhase::Inactive:    return "Inactive";
        case WeatherPhase::Approaching: return "Approaching";
        case WeatherPhase::Active:      return "Active";
        case WeatherPhase::Dissipating: return "Dissipating";
    }
    return "Unknown";
}

std::string WeatherModifiers::GetSeverityName(WeatherSeverity severity) {
    switch (severity) {
        case WeatherSeverity::Mild:     return "Mild";
        case WeatherSeverity::Moderate: return "Moderate";
        case WeatherSeverity::Severe:   return "Severe";
        case WeatherSeverity::Extreme:  return "Extreme";
    }
    return "Unknown";
}

float WeatherModifiers::GetSeverityIntensity(WeatherSeverity severity) {
    switch (severity) {
        case WeatherSeverity::Mild:     return 0.5f;
        case WeatherSeverity::Moderate: return 1.0f;
        case WeatherSeverity::Severe:   return 1.5f;
        case WeatherSeverity::Extreme:  return 2.0f;
    }
    return 1.0f;
}

WeatherModifiers WeatherModifiers::GetDefaults(SpaceWeatherType type,
                                                WeatherSeverity severity) {
    WeatherModifiers m;
    float intensity = GetSeverityIntensity(severity);

    switch (type) {
        case SpaceWeatherType::Clear:
            // No modifiers
            break;
        case SpaceWeatherType::IonStorm:
            m.shieldMultiplier = 1.0f - 0.2f * intensity;
            m.sensorRangeMultiplier = 1.0f - 0.3f * intensity;
            m.accuracyMultiplier = 1.0f - 0.15f * intensity;
            break;
        case SpaceWeatherType::SolarFlare:
            m.damagePerSecond = 5.0f * intensity;
            m.weaponDamageMultiplier = 1.0f + 0.2f * intensity;
            m.shieldMultiplier = 1.0f - 0.1f * intensity;
            break;
        case SpaceWeatherType::NebulaDrift:
            m.sensorRangeMultiplier = 1.0f - 0.4f * intensity;
            m.stealthBonus = 0.3f * intensity;
            m.speedMultiplier = 1.0f - 0.1f * intensity;
            break;
        case SpaceWeatherType::MeteorShower:
            m.damagePerSecond = 3.0f * intensity;
            m.miningYieldMultiplier = 1.0f + 0.25f * intensity;
            m.speedMultiplier = 1.0f - 0.05f * intensity;
            break;
        case SpaceWeatherType::RadiationBurst:
            m.damagePerSecond = 8.0f * intensity;
            m.shieldMultiplier = 1.0f - 0.15f * intensity;
            break;
        case SpaceWeatherType::GravityAnomaly:
            m.speedMultiplier = 1.0f - 0.3f * intensity;
            m.accuracyMultiplier = 1.0f - 0.1f * intensity;
            break;
        case SpaceWeatherType::ElectromagneticPulse:
            m.shieldMultiplier = 1.0f - 0.5f * intensity;
            m.sensorRangeMultiplier = 1.0f - 0.5f * intensity;
            m.weaponDamageMultiplier = 1.0f - 0.3f * intensity;
            m.accuracyMultiplier = 1.0f - 0.4f * intensity;
            break;
    }

    // Clamp multipliers to valid ranges
    m.speedMultiplier       = std::max(0.1f, m.speedMultiplier);
    m.shieldMultiplier      = std::max(0.0f, m.shieldMultiplier);
    m.sensorRangeMultiplier = std::max(0.05f, m.sensorRangeMultiplier);
    m.weaponDamageMultiplier = std::max(0.1f, m.weaponDamageMultiplier);
    m.accuracyMultiplier    = std::max(0.1f, m.accuracyMultiplier);
    m.stealthBonus          = std::min(1.0f, m.stealthBonus);

    return m;
}

// ---------------------------------------------------------------------------
// WeatherEvent
// ---------------------------------------------------------------------------

float WeatherEvent::GetCurrentIntensity() const {
    if (phase == WeatherPhase::Inactive) return 0.0f;

    if (phase == WeatherPhase::Approaching) {
        if (approachDuration <= 0.0f) return 1.0f;
        return std::min(1.0f, elapsedTime / approachDuration);
    }

    if (phase == WeatherPhase::Dissipating) {
        float dissipateStart = totalDuration - dissipationDuration;
        float progress = elapsedTime - dissipateStart;
        if (dissipationDuration <= 0.0f) return 0.0f;
        return std::max(0.0f, 1.0f - progress / dissipationDuration);
    }

    return 1.0f; // Active phase = full intensity
}

bool WeatherEvent::IsFinished() const {
    return elapsedTime >= totalDuration;
}

// ---------------------------------------------------------------------------
// WeatherComponent
// ---------------------------------------------------------------------------

WeatherComponent::WeatherComponent(uint64_t sectorId)
    : _sectorId(sectorId) {}

uint64_t WeatherComponent::GetSectorId() const { return _sectorId; }
void WeatherComponent::SetSectorId(uint64_t sectorId) { _sectorId = sectorId; }

const std::vector<WeatherEvent>& WeatherComponent::GetActiveWeather() const {
    return _events;
}

bool WeatherComponent::AddWeatherEvent(const WeatherEvent& event) {
    if (_events.size() >= kMaxConcurrentEvents) return false;
    _events.push_back(event);
    return true;
}

size_t WeatherComponent::GetActiveEventCount() const {
    return _events.size();
}

void WeatherComponent::ClearFinished() {
    _events.erase(
        std::remove_if(_events.begin(), _events.end(),
            [](const WeatherEvent& e) { return e.IsFinished(); }),
        _events.end());
}

WeatherModifiers WeatherComponent::GetCombinedModifiers() const {
    WeatherModifiers combined;
    for (const auto& e : _events) {
        float intensity = e.GetCurrentIntensity();
        if (intensity <= 0.0f) continue;

        // Blend modifiers multiplicatively for multipliers, additively for DPS/stealth
        combined.speedMultiplier       *= 1.0f + (e.modifiers.speedMultiplier - 1.0f) * intensity;
        combined.shieldMultiplier      *= 1.0f + (e.modifiers.shieldMultiplier - 1.0f) * intensity;
        combined.sensorRangeMultiplier *= 1.0f + (e.modifiers.sensorRangeMultiplier - 1.0f) * intensity;
        combined.weaponDamageMultiplier*= 1.0f + (e.modifiers.weaponDamageMultiplier - 1.0f) * intensity;
        combined.accuracyMultiplier    *= 1.0f + (e.modifiers.accuracyMultiplier - 1.0f) * intensity;
        combined.miningYieldMultiplier *= 1.0f + (e.modifiers.miningYieldMultiplier - 1.0f) * intensity;
        combined.damagePerSecond       += e.modifiers.damagePerSecond * intensity;
        combined.stealthBonus          += e.modifiers.stealthBonus * intensity;
    }
    combined.stealthBonus = std::min(1.0f, combined.stealthBonus);
    return combined;
}

bool WeatherComponent::HasWeatherType(SpaceWeatherType type) const {
    for (const auto& e : _events) {
        if (e.type == type && !e.IsFinished()) return true;
    }
    return false;
}

const WeatherEvent* WeatherComponent::GetMostSevereEvent() const {
    const WeatherEvent* worst = nullptr;
    for (const auto& e : _events) {
        if (e.IsFinished()) continue;
        if (!worst || static_cast<int>(e.severity) > static_cast<int>(worst->severity)) {
            worst = &e;
        }
    }
    return worst;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData WeatherComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "WeatherComponent";
    cd.data["sectorId"] = std::to_string(_sectorId);
    cd.data["eventCount"] = std::to_string(_events.size());

    for (size_t i = 0; i < _events.size(); ++i) {
        std::string p = "evt_" + std::to_string(i) + "_";
        const auto& e = _events[i];
        cd.data[p + "type"]          = std::to_string(static_cast<int>(e.type));
        cd.data[p + "phase"]         = std::to_string(static_cast<int>(e.phase));
        cd.data[p + "severity"]      = std::to_string(static_cast<int>(e.severity));
        cd.data[p + "totalDuration"] = std::to_string(e.totalDuration);
        cd.data[p + "elapsedTime"]   = std::to_string(e.elapsedTime);
        cd.data[p + "approachDur"]   = std::to_string(e.approachDuration);
        cd.data[p + "dissipDur"]     = std::to_string(e.dissipationDuration);
        cd.data[p + "sectorId"]      = std::to_string(e.sectorId);
    }
    return cd;
}

void WeatherComponent::Deserialize(const ComponentData& data) {
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
    auto getUint64 = [&](const std::string& key, uint64_t def = 0) -> uint64_t {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoull(it->second); } catch (...) { return def; }
    };

    _sectorId = getUint64("sectorId", 0);
    int count = getInt("eventCount", 0);
    _events.clear();
    _events.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        std::string p = "evt_" + std::to_string(i) + "_";
        WeatherEvent e;
        int typeVal = getInt(p + "type", 0);
        constexpr int kMaxType = static_cast<int>(SpaceWeatherType::ElectromagneticPulse);
        if (typeVal >= 0 && typeVal <= kMaxType)
            e.type = static_cast<SpaceWeatherType>(typeVal);

        int phaseVal = getInt(p + "phase", 0);
        constexpr int kMaxPhase = static_cast<int>(WeatherPhase::Dissipating);
        if (phaseVal >= 0 && phaseVal <= kMaxPhase)
            e.phase = static_cast<WeatherPhase>(phaseVal);

        int sevVal = getInt(p + "severity", 0);
        constexpr int kMaxSev = static_cast<int>(WeatherSeverity::Extreme);
        if (sevVal >= 0 && sevVal <= kMaxSev)
            e.severity = static_cast<WeatherSeverity>(sevVal);

        e.totalDuration      = getFloat(p + "totalDuration", 60.0f);
        e.elapsedTime        = getFloat(p + "elapsedTime", 0.0f);
        e.approachDuration   = getFloat(p + "approachDur", 10.0f);
        e.dissipationDuration= getFloat(p + "dissipDur", 10.0f);
        e.sectorId           = getUint64(p + "sectorId", 0);
        e.modifiers          = WeatherModifiers::GetDefaults(e.type, e.severity);
        _events.push_back(e);
    }
}

// ---------------------------------------------------------------------------
// WeatherSystem
// ---------------------------------------------------------------------------

WeatherSystem::WeatherSystem() : SystemBase("WeatherSystem") {}

WeatherSystem::WeatherSystem(EntityManager& entityManager)
    : SystemBase("WeatherSystem")
    , _entityManager(&entityManager)
{
}

void WeatherSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void WeatherSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto components = _entityManager->GetAllComponents<WeatherComponent>();
    for (auto* wc : components) {
        for (auto& event : wc->_events) {
            if (event.IsFinished()) continue;

            event.elapsedTime += deltaTime;

            // Transition phases
            if (event.elapsedTime < event.approachDuration) {
                event.phase = WeatherPhase::Approaching;
            } else if (event.elapsedTime >= event.totalDuration - event.dissipationDuration) {
                event.phase = WeatherPhase::Dissipating;
            } else {
                event.phase = WeatherPhase::Active;
            }
        }

        // Remove finished events
        wc->ClearFinished();
    }
}

WeatherEvent WeatherSystem::GenerateWeather(uint64_t sectorId, uint32_t seed) {
    WeatherEvent event;

    // Deterministic type selection from seed
    int typeIndex = static_cast<int>(seed % 7) + 1; // 1-7, skip Clear
    event.type = static_cast<SpaceWeatherType>(typeIndex);

    // Severity from seed bits
    int sevIndex = static_cast<int>((seed >> 4) % 4);
    event.severity = static_cast<WeatherSeverity>(sevIndex);

    // Duration 30-120 seconds based on seed
    event.totalDuration = 30.0f + static_cast<float>((seed >> 8) % 91);
    event.approachDuration = event.totalDuration * 0.15f;
    event.dissipationDuration = event.totalDuration * 0.15f;
    event.elapsedTime = 0.0f;
    event.phase = WeatherPhase::Approaching;
    event.sectorId = sectorId;
    event.modifiers = WeatherModifiers::GetDefaults(event.type, event.severity);

    return event;
}

} // namespace subspace
