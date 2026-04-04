#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <cstdint>

namespace subspace {

/// Types of space weather phenomena.
enum class SpaceWeatherType {
    Clear,               // No weather effects
    IonStorm,            // Disrupts shields and sensors
    SolarFlare,          // Radiation damage, overcharges energy weapons
    NebulaDrift,         // Reduces sensor range, provides stealth
    MeteorShower,        // Random kinetic damage, mining bonus
    RadiationBurst,      // Sustained hull/crew damage
    GravityAnomaly,      // Alters ship speed and maneuverability
    ElectromagneticPulse // Temporarily disables electronics
};

/// Current phase of a weather event lifecycle.
enum class WeatherPhase {
    Inactive,    // No weather active
    Approaching, // Warning phase before full intensity
    Active,      // Full-intensity weather
    Dissipating  // Fading out
};

/// Severity level of a weather event.
enum class WeatherSeverity {
    Mild,
    Moderate,
    Severe,
    Extreme
};

/// Modifiers that weather applies to entities in the affected area.
struct WeatherModifiers {
    float speedMultiplier = 1.0f;        // Affects ship speed
    float shieldMultiplier = 1.0f;       // Affects shield effectiveness
    float sensorRangeMultiplier = 1.0f;  // Affects sensor/scan range
    float damagePerSecond = 0.0f;        // Environmental damage
    float weaponDamageMultiplier = 1.0f; // Affects weapon damage
    float miningYieldMultiplier = 1.0f;  // Affects mining output
    float stealthBonus = 0.0f;           // Added stealth (0–1)
    float accuracyMultiplier = 1.0f;     // Affects targeting accuracy

    /// Get default modifiers for a weather type and severity.
    static WeatherModifiers GetDefaults(SpaceWeatherType type, WeatherSeverity severity);

    /// Get the display name for a weather type.
    static std::string GetWeatherTypeName(SpaceWeatherType type);

    /// Get the display name for a weather phase.
    static std::string GetPhaseName(WeatherPhase phase);

    /// Get the display name for a severity level.
    static std::string GetSeverityName(WeatherSeverity severity);

    /// Get the intensity multiplier for a severity level (Mild 0.5 … Extreme 2.0).
    static float GetSeverityIntensity(WeatherSeverity severity);
};

/// Represents one active weather event in a sector.
struct WeatherEvent {
    SpaceWeatherType type = SpaceWeatherType::Clear;
    WeatherPhase phase = WeatherPhase::Inactive;
    WeatherSeverity severity = WeatherSeverity::Mild;
    float totalDuration = 60.0f;      // Total event duration (seconds)
    float elapsedTime = 0.0f;         // Time since event started
    float approachDuration = 10.0f;   // Warning phase length
    float dissipationDuration = 10.0f;// Fade-out phase length
    uint64_t sectorId = 0;            // Which sector this affects
    WeatherModifiers modifiers;       // Current effective modifiers

    /// Get current intensity (0–1), ramping during approach/dissipation.
    float GetCurrentIntensity() const;

    /// Is the event finished?
    bool IsFinished() const;
};

/// ECS component that exposes weather state for an entity's current sector.
class WeatherComponent : public IComponent {
public:
    WeatherComponent() = default;
    explicit WeatherComponent(uint64_t sectorId);

    uint64_t GetSectorId() const;
    void SetSectorId(uint64_t sectorId);

    /// Get the active weather events in this sector.
    const std::vector<WeatherEvent>& GetActiveWeather() const;

    /// Add a new weather event.
    bool AddWeatherEvent(const WeatherEvent& event);

    /// Get the number of active events.
    size_t GetActiveEventCount() const;

    /// Remove all finished events.
    void ClearFinished();

    /// Get the combined modifiers from all active weather events.
    WeatherModifiers GetCombinedModifiers() const;

    /// Check if any weather of a specific type is active.
    bool HasWeatherType(SpaceWeatherType type) const;

    /// Get the most severe weather event currently active (or nullptr).
    const WeatherEvent* GetMostSevereEvent() const;

    /// Maximum number of simultaneous weather events in a sector.
    static constexpr size_t kMaxConcurrentEvents = 3;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    uint64_t _sectorId = 0;
    std::vector<WeatherEvent> _events;

    friend class WeatherSystem;
};

/// System that manages space weather: generation, progression, and removal.
class WeatherSystem : public SystemBase {
public:
    WeatherSystem();
    explicit WeatherSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    void SetEntityManager(EntityManager* em);

    /// Generate a random weather event for a sector using a deterministic seed.
    static WeatherEvent GenerateWeather(uint64_t sectorId, uint32_t seed);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
