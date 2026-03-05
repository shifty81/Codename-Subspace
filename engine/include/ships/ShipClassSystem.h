#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace subspace {

/// Classification of ship hull types.
enum class ShipClass {
    Fighter,
    Corvette,
    Frigate,
    Destroyer,
    Cruiser,
    Battleship,
    Carrier,
    Freighter,
    Miner,
    Explorer
};

/// High-level role a ship fulfils.
enum class ShipRole {
    Combat,
    Trade,
    Mining,
    Exploration,
    Support,
    MultiRole
};

/// Multipliers applied to base stats depending on ship class.
struct ClassBonus {
    float speedMultiplier  = 1.0f;
    float damageMultiplier = 1.0f;
    float shieldMultiplier = 1.0f;
    float cargoMultiplier  = 1.0f;
    float miningMultiplier = 1.0f;
    float sensorMultiplier = 1.0f;
};

/// Full definition of a ship class including stats and crew requirements.
struct ShipClassDefinition {
    ShipClass shipClass = ShipClass::Fighter;
    std::string displayName;
    std::string description;
    ShipRole role = ShipRole::Combat;
    ClassBonus bonus;
    int minCrew   = 1;
    int maxCrew   = 4;
    float baseMass = 50.0f;
    float baseHull = 100.0f;
    int techLevel  = 1;  // 1-10

    /// Get the display name for a ship class.
    static std::string GetClassName(ShipClass shipClass);

    /// Get the display name for a ship role.
    static std::string GetRoleName(ShipRole role);

    /// Return sensible default definition for the given class.
    static ShipClassDefinition GetDefaultDefinition(ShipClass shipClass);
};

/// ECS component that assigns a class to a ship entity.
class ShipClassComponent : public IComponent {
public:
    ShipClassComponent();
    explicit ShipClassComponent(ShipClass shipClass);

    ShipClass GetShipClass() const;
    void SetShipClass(ShipClass shipClass);

    ShipRole GetRole() const;
    const ClassBonus& GetClassBonus() const;
    const ShipClassDefinition& GetDefinition() const;

    float GetEffectiveSpeed(float baseSpeed) const;
    float GetEffectiveDamage(float baseDamage) const;
    float GetEffectiveShield(float baseShield) const;
    float GetEffectiveCargo(float baseCargo) const;
    float GetEffectiveMining(float baseMining) const;
    float GetEffectiveSensor(float baseSensor) const;

    std::string GetDisplayName() const;
    std::string GetDescription() const;
    int GetTechLevel() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    ShipClassDefinition _definition;

    friend class ShipClassSystem;
};

/// System that manages ship class components.
class ShipClassSystem : public SystemBase {
public:
    ShipClassSystem();
    explicit ShipClassSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    /// Check whether upgrading from current to target class is allowed.
    /// Target tech level must be at most current tech level + 2.
    bool CanUpgradeClass(ShipClass current, ShipClass target) const;

    /// Return every class that the given class may upgrade to.
    std::vector<ShipClass> GetAvailableUpgrades(ShipClass current) const;

    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
