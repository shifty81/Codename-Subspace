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

/// Rarity tiers for loot items.
enum class LootRarity { Common, Uncommon, Rare, Epic, Legendary };

/// A single entry in a loot table.
struct LootTableEntry {
    std::string itemName;
    LootRarity rarity = LootRarity::Common;
    float dropChance = 1.0f;   // 0.0 to 1.0
    int minQuantity = 1;
    int maxQuantity = 1;

    /// Get the display name for a rarity tier.
    static std::string GetRarityName(LootRarity rarity);

    /// Get the drop chance weight multiplier for a rarity tier (used in weighted selection).
    static float GetRarityWeight(LootRarity rarity);
};

/// A generated loot drop result.
struct LootDrop {
    std::string itemName;
    LootRarity rarity = LootRarity::Common;
    int quantity = 1;
};

/// A loot table containing possible drops.
struct LootTable {
    std::string tableName;
    std::vector<LootTableEntry> entries;

    /// Roll the loot table with a given seed. Returns generated drops.
    std::vector<LootDrop> Roll(uint32_t seed) const;

    /// Roll with a luck multiplier (higher = better rarity chance). Seed-based.
    std::vector<LootDrop> RollWithLuck(uint32_t seed, float luckMultiplier) const;

    /// Create a predefined "standard enemy" loot table.
    static LootTable CreateStandardEnemyTable();

    /// Create a predefined "boss" loot table (higher rarity chances).
    static LootTable CreateBossTable();

    /// Create a predefined "asteroid" loot table (resource-focused).
    static LootTable CreateAsteroidTable();
};

/// ECS component that assigns a loot table to an entity.
struct LootComponent : public IComponent {
    LootTable lootTable;
    float luckModifier = 1.0f;       // entity-specific luck
    bool hasBeenLooted = false;
    uint32_t lootSeed = 0;           // deterministic seed

    /// Generate drops from this entity's loot table.
    std::vector<LootDrop> GenerateDrops() const;

    /// Mark as looted.
    void MarkLooted();

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);
};

/// System that manages loot generation (processes loot on entity destruction).
class LootSystem : public SystemBase {
public:
    LootSystem();
    explicit LootSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
