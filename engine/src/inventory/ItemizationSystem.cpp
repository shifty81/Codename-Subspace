#include "inventory/ItemizationSystem.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace subspace {
namespace {

float Next01(std::uint32_t& seed) {
    seed = seed * 1664525u + 1013904223u;
    return static_cast<float>((seed >> 8) & 0x00FFFFFFu) / 16777215.0f;
}

float SizeScale(ShipyardModuleSize size) {
    switch (size) {
        case ShipyardModuleSize::XS: return 0.55f;
        case ShipyardModuleSize::S: return 0.80f;
        case ShipyardModuleSize::M: return 1.0f;
        case ShipyardModuleSize::L: return 1.55f;
        case ShipyardModuleSize::XL: return 2.35f;
    }
    return 1.0f;
}

void Add(std::vector<ItemStatRoll>& out, ItemStatKind stat, float value, bool higher = true) {
    out.push_back({stat, value, value, higher});
}

std::vector<ItemAffix> AffixPool(const ItemDefinition& d) {
    std::vector<ItemAffix> pool;
    pool.push_back({"reinforced","Reinforced",ItemStatKind::Integrity,1.10f,0.0f});
    pool.push_back({"lightweight","Lightweight",ItemStatKind::Mass,0.90f,0.0f});
    pool.push_back({"efficient","Efficient",ItemStatKind::PowerDraw,0.90f,0.0f});
    if (d.shipyardRole==ShipyardPartRole::MainEngine || d.shipyardRole==ShipyardPartRole::RcsThruster)
        pool.push_back({"high_output","High Output",ItemStatKind::Thrust,1.12f,0.0f});
    if (d.shipyardRole==ShipyardPartRole::SensorDish || d.shipyardRole==ShipyardPartRole::SensorMast)
        pool.push_back({"precision","Precision",ItemStatKind::SensorRange,1.12f,0.0f});
    if (d.shipyardRole==ShipyardPartRole::WeaponTurret || d.shipyardRole==ShipyardPartRole::MissileMount)
        pool.push_back({"calibrated","Calibrated",ItemStatKind::WeaponDamage,1.10f,0.0f});
    if (d.shipyardRole==ShipyardPartRole::Cargo)
        pool.push_back({"expanded","Expanded",ItemStatKind::Cargo,1.12f,0.0f});
    return pool;
}

} // namespace

float GeneratedItem::Stat(ItemStatKind kind, float fallback) const {
    for (const auto& s : stats) if (s.stat == kind) return s.resolvedValue;
    return fallback;
}

ItemDefinition ItemizationSystem::BuildShipyardDefinition(const ShipyardModuleRecord& record) {
    ItemDefinition d;
    d.definitionId = "shipyard:" + record.source.moduleId;
    d.displayName = ShipyardPartTaxonomySystem::DisplayName(record.source.moduleId);
    d.kind = ItemKind::ShipModule;
    d.category = ShipyardPartTaxonomySystem::CategoryName(record.builderCategory);
    d.sourceModuleId = record.source.moduleId;
    d.shipyardRole = record.partRole;
    d.shipyardSize = record.size;
    const float s = SizeScale(record.size);
    const float volumeProxy = std::max(0.15f, record.source.halfWidth * record.source.halfLength * record.source.halfHeight * 8.0f);
    d.baseMass = std::max(0.25f, volumeProxy * 1.8f * s);
    d.baseValue = 120.0f * s + volumeProxy * 34.0f;

    Add(d.baseStats, ItemStatKind::Integrity, 140.0f * s + volumeProxy * 18.0f);
    Add(d.baseStats, ItemStatKind::Mass, d.baseMass, false);

    switch (record.partRole) {
        case ShipyardPartRole::MainEngine:
            Add(d.baseStats, ItemStatKind::Thrust, 120.0f * s);
            Add(d.baseStats, ItemStatKind::PowerDraw, 32.0f * s, false);
            Add(d.baseStats, ItemStatKind::Heat, 24.0f * s, false);
            d.equipmentTags = {"PROPULSION","MAIN_ENGINE"};
            break;
        case ShipyardPartRole::RcsThruster:
            Add(d.baseStats, ItemStatKind::Torque, 48.0f * s);
            Add(d.baseStats, ItemStatKind::PowerDraw, 12.0f * s, false);
            d.equipmentTags = {"PROPULSION","RCS"};
            break;
        case ShipyardPartRole::EngineHousing:
        case ShipyardPartRole::EngineMount:
            Add(d.baseStats, ItemStatKind::Efficiency, 1.0f);
            d.equipmentTags = {"PROPULSION_MOUNT"};
            break;
        case ShipyardPartRole::Cargo:
            Add(d.baseStats, ItemStatKind::Cargo, 48.0f * s);
            d.equipmentTags = {"CARGO"};
            break;
        case ShipyardPartRole::Tank:
            Add(d.baseStats, ItemStatKind::Cargo, 26.0f * s);
            d.equipmentTags = {"FUEL","UTILITY"};
            break;
        case ShipyardPartRole::SensorDish:
        case ShipyardPartRole::SensorMast:
            Add(d.baseStats, ItemStatKind::SensorRange, 16.0f * s);
            Add(d.baseStats, ItemStatKind::PowerDraw, 7.0f * s, false);
            d.equipmentTags = {"SENSOR"};
            break;
        case ShipyardPartRole::WeaponTurret:
        case ShipyardPartRole::MissileMount:
        case ShipyardPartRole::HardpointBase:
            Add(d.baseStats, ItemStatKind::WeaponDamage, 36.0f * s);
            Add(d.baseStats, ItemStatKind::WeaponRange, 9.0f * s);
            d.equipmentTags = {"WEAPON"};
            break;
        case ShipyardPartRole::Cockpit:
        case ShipyardPartRole::Bridge:
            Add(d.baseStats, ItemStatKind::SensorRange, 8.0f * s);
            Add(d.baseStats, ItemStatKind::Efficiency, 1.0f);
            d.equipmentTags = {"COMMAND","SENSOR","UTILITY"};
            break;
        default:
            d.equipmentTags = record.functional ? std::vector<std::string>{"UTILITY"} : std::vector<std::string>{};
            break;
    }
    return d;
}

ItemRarity ItemizationSystem::RollRarity(std::uint32_t seed, float qualityBias, ItemRarity maximum) {
    float roll = Next01(seed) - std::clamp(qualityBias, -0.20f, 0.30f);
    ItemRarity r = ItemRarity::Common;
    if (roll < 0.015f) r = ItemRarity::Legendary;
    else if (roll < 0.065f) r = ItemRarity::Epic;
    else if (roll < 0.18f) r = ItemRarity::Rare;
    else if (roll < 0.46f) r = ItemRarity::Uncommon;
    return static_cast<int>(r) > static_cast<int>(maximum) ? maximum : r;
}

int ItemizationSystem::RarityAffixBudget(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::Common: return 0;
        case ItemRarity::Uncommon: return 1;
        case ItemRarity::Rare: return 2;
        case ItemRarity::Epic: return 3;
        case ItemRarity::Legendary: return 4;
    }
    return 0;
}

float ItemizationSystem::RarityStatMultiplier(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::Common: return 1.00f;
        case ItemRarity::Uncommon: return 1.035f;
        case ItemRarity::Rare: return 1.075f;
        case ItemRarity::Epic: return 1.125f;
        case ItemRarity::Legendary: return 1.19f;
    }
    return 1.0f;
}

GeneratedItem ItemizationSystem::Generate(const ItemDefinition& definition,
                                           std::uint32_t seed,
                                           ItemRarity rarity,
                                           float quality,
                                           float condition,
                                           const ItemProvenance& provenance) {
    GeneratedItem item;
    item.definitionId = definition.definitionId;
    item.displayName = definition.displayName;
    item.kind = definition.kind;
    item.rarity = rarity;
    item.quality = std::clamp(quality, 0.70f, 1.35f);
    item.condition = std::clamp(condition, 0.0f, 1.0f);
    item.category = definition.category;
    item.sourceModuleId = definition.sourceModuleId;
    item.stackable = definition.kind == ItemKind::Resource || definition.kind == ItemKind::Salvage;
    item.quantity = 1;
    item.provenance = provenance;
    item.provenance.seed = seed;

    std::ostringstream id;
    id << definition.definitionId << ':' << std::hex << seed << ':' << static_cast<int>(rarity);
    item.instanceId = id.str();

    const float rarityScale = RarityStatMultiplier(rarity);
    const float performanceScale = item.quality * rarityScale;
    item.stats = definition.baseStats;
    for (auto& stat : item.stats) {
        if (stat.stat == ItemStatKind::Mass) {
            // Better manufacturing can reduce excess structural mass but never
            // changes the actual visible module dimensions.
            stat.resolvedValue = stat.baseValue * std::clamp(1.08f - (item.quality - 0.80f) * 0.24f, 0.90f, 1.08f);
        } else if (!stat.higherIsBetter) {
            stat.resolvedValue = stat.baseValue / std::max(0.75f, performanceScale);
        } else {
            stat.resolvedValue = stat.baseValue * performanceScale;
        }
    }

    auto pool = AffixPool(definition);
    const int budget = std::min<int>(RarityAffixBudget(rarity), static_cast<int>(pool.size()));
    for (int i=0; i<budget && !pool.empty(); ++i) {
        const std::size_t index = static_cast<std::size_t>(Next01(seed) * static_cast<float>(pool.size())) % pool.size();
        const auto affix = pool[index];
        item.affixes.push_back(affix);
        for (auto& stat : item.stats) if (stat.stat == affix.stat) stat.resolvedValue = stat.resolvedValue * affix.multiplier + affix.additive;
        pool.erase(pool.begin() + static_cast<std::ptrdiff_t>(index));
    }

    item.mass = item.Stat(ItemStatKind::Mass, definition.baseMass);
    const float conditionValue = 0.25f + 0.75f * item.condition;
    item.value = static_cast<int>(std::round(definition.baseValue * rarityScale * item.quality * conditionValue * (1.0f + 0.12f * item.affixes.size())));
    item.iconKey = BuildIconKey(item);
    return item;
}

const char* ItemizationSystem::StatName(ItemStatKind stat) {
    switch (stat) {
        case ItemStatKind::Integrity:return "INTEGRITY";
        case ItemStatKind::Mass:return "MASS";
        case ItemStatKind::Thrust:return "THRUST";
        case ItemStatKind::Torque:return "TORQUE";
        case ItemStatKind::PowerDraw:return "POWER DRAW";
        case ItemStatKind::PowerOutput:return "POWER OUTPUT";
        case ItemStatKind::Heat:return "HEAT";
        case ItemStatKind::Cargo:return "CARGO";
        case ItemStatKind::SensorRange:return "SENSOR RANGE";
        case ItemStatKind::MiningYield:return "MINING";
        case ItemStatKind::SalvageYield:return "SALVAGE";
        case ItemStatKind::WeaponDamage:return "DAMAGE";
        case ItemStatKind::WeaponRange:return "RANGE";
        case ItemStatKind::Shield:return "SHIELD";
        case ItemStatKind::Efficiency:return "EFFICIENCY";
    }
    return "STAT";
}

std::string ItemizationSystem::BuildIconKey(const GeneratedItem& item) {
    std::ostringstream s;
    s << item.sourceModuleId << '|' << item.definitionId << '|' << static_cast<int>(item.rarity) << '|'
      << std::fixed << std::setprecision(3) << item.quality;
    for (const auto& a : item.affixes) s << '|' << a.id;
    return s.str();
}

InventoryItem ItemizationSystem::ToInventoryItem(const GeneratedItem& item) {
    InventoryItem inv;
    inv.itemId = item.definitionId;
    inv.instanceId = item.instanceId;
    inv.name = item.displayName;
    inv.description = InventoryItem::GetRarityName(item.rarity) + " " + item.category;
    inv.rarity = item.rarity;
    inv.weight = item.mass;
    inv.stackSize = item.quantity;
    inv.maxStackSize = item.stackable ? 99 : 1;
    inv.value = item.value;
    inv.category = item.category;
    inv.uniqueInstance = !item.stackable;
    inv.sourceModuleId = item.sourceModuleId;
    inv.iconKey = item.iconKey;
    inv.quality = item.quality;
    inv.condition = item.condition;
    inv.blueprintId = item.blueprintId;
    inv.blueprintFragmentIndex = item.blueprintFragmentIndex;
    inv.blueprintFragmentsRequired = item.blueprintFragmentsRequired;
    for (const auto& stat : item.stats) inv.stats.push_back({StatName(stat.stat), stat.resolvedValue});
    for (const auto& affix : item.affixes) inv.affixes.push_back(affix.displayName);
    return inv;
}

} // namespace subspace
