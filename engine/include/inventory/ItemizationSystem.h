#pragma once

#include "inventory/InventorySystem.h"
#include "content/ShipyardModuleSystem.h"
#include "core/Math.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class ItemKind {
    ShipModule,
    Equipment,
    Blueprint,
    BlueprintFragment,
    Resource,
    Salvage
};

enum class ItemStatKind {
    Integrity,
    Mass,
    Thrust,
    Torque,
    PowerDraw,
    PowerOutput,
    Heat,
    Cargo,
    SensorRange,
    MiningYield,
    SalvageYield,
    WeaponDamage,
    WeaponRange,
    Shield,
    Efficiency
};

struct ItemStatRoll {
    ItemStatKind stat = ItemStatKind::Integrity;
    float baseValue = 0.0f;
    float resolvedValue = 0.0f;
    bool higherIsBetter = true;
};

struct ItemAffix {
    std::string id;
    std::string displayName;
    ItemStatKind stat = ItemStatKind::Integrity;
    float multiplier = 1.0f;
    float additive = 0.0f;
};

struct ItemProvenance {
    std::string source = "UNKNOWN";
    std::string manufacturer = "UNKNOWN";
    std::string crafter = "";
    std::string originEntity;
    int manufacturingSkill = 0;
    int stationTier = 0;
    std::uint32_t seed = 1;
};

struct ItemDefinition {
    std::string definitionId;
    std::string displayName;
    ItemKind kind = ItemKind::ShipModule;
    std::string category;
    std::string sourceModuleId;
    ShipyardPartRole shipyardRole = ShipyardPartRole::Decoration;
    ShipyardModuleSize shipyardSize = ShipyardModuleSize::M;
    float baseMass = 1.0f;
    float baseValue = 100.0f;
    std::vector<ItemStatRoll> baseStats;
    std::vector<std::string> equipmentTags;
};

struct GeneratedItem {
    std::string instanceId;
    std::string definitionId;
    std::string displayName;
    ItemKind kind = ItemKind::ShipModule;
    ItemRarity rarity = ItemRarity::Common;
    float quality = 1.0f;              // craftsmanship/performance scalar
    float condition = 1.0f;            // 0..1 physical condition
    float mass = 1.0f;
    int value = 0;
    std::string category;
    std::string sourceModuleId;
    std::string iconKey;
    // Blueprint loot preserves the authoritative design identity rather than
    // becoming an anonymous generic inventory token. Fragment fields are zero
    // for ordinary items and complete blueprints.
    std::string blueprintId;
    int blueprintFragmentIndex = 0;
    int blueprintFragmentsRequired = 0;
    std::vector<ItemStatRoll> stats;
    std::vector<ItemAffix> affixes;
    ItemProvenance provenance{};
    bool stackable = false;
    int quantity = 1;

    float Stat(ItemStatKind kind, float fallback = 0.0f) const;
};

/// Canonical item generation authority used by loot, manufacturing, salvage,
/// inventory and Shipyard equipment. A resolved item instance is never
/// regenerated when moving between those systems: its instance id, rolls,
/// rarity, quality and provenance remain stable.
class ItemizationSystem {
public:
    static ItemDefinition BuildShipyardDefinition(const ShipyardModuleRecord& record);
    static GeneratedItem Generate(const ItemDefinition& definition,
                                  std::uint32_t seed,
                                  ItemRarity rarity,
                                  float quality = 1.0f,
                                  float condition = 1.0f,
                                  const ItemProvenance& provenance = {});

    static ItemRarity RollRarity(std::uint32_t seed,
                                 float qualityBias = 0.0f,
                                 ItemRarity maximum = ItemRarity::Legendary);

    static int RarityAffixBudget(ItemRarity rarity);
    static float RarityStatMultiplier(ItemRarity rarity);
    static const char* StatName(ItemStatKind stat);
    static std::string BuildIconKey(const GeneratedItem& item);
    static InventoryItem ToInventoryItem(const GeneratedItem& item);
};

} // namespace subspace
