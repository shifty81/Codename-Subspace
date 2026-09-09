#include "combat/LootSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// LootTableEntry
// ---------------------------------------------------------------------------

std::string LootTableEntry::GetRarityName(LootRarity rarity) {
    switch (rarity) {
        case LootRarity::Common:    return "Common";
        case LootRarity::Uncommon:  return "Uncommon";
        case LootRarity::Rare:      return "Rare";
        case LootRarity::Epic:      return "Epic";
        case LootRarity::Legendary: return "Legendary";
    }
    return "Common";
}

float LootTableEntry::GetRarityWeight(LootRarity rarity) {
    switch (rarity) {
        case LootRarity::Common:    return 1.0f;
        case LootRarity::Uncommon:  return 0.5f;
        case LootRarity::Rare:      return 0.2f;
        case LootRarity::Epic:      return 0.08f;
        case LootRarity::Legendary: return 0.02f;
    }
    return 1.0f;
}

// ---------------------------------------------------------------------------
// LootTable helpers
// ---------------------------------------------------------------------------

namespace {

// Simple LCG PRNG: advance seed and return a float in [0, 1].
float LcgNext(uint32_t& seed) {
    seed = seed * 1103515245 + 12345;
    return static_cast<float>((seed >> 16) & 0x7FFF) / 32767.0f;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// LootTable
// ---------------------------------------------------------------------------

std::vector<LootDrop> LootTable::Roll(uint32_t seed) const {
    std::vector<LootDrop> drops;
    for (const auto& entry : entries) {
        float roll = LcgNext(seed);
        if (roll < entry.dropChance) {
            LootDrop drop;
            drop.itemName = entry.itemName;
            drop.rarity   = entry.rarity;

            if (entry.maxQuantity > entry.minQuantity) {
                float qRoll = LcgNext(seed);
                int range = entry.maxQuantity - entry.minQuantity + 1;
                drop.quantity = entry.minQuantity +
                    static_cast<int>(qRoll * static_cast<float>(range));
                drop.quantity = std::min(drop.quantity, entry.maxQuantity);
            } else {
                drop.quantity = entry.minQuantity;
            }

            drops.push_back(drop);
        }
    }
    return drops;
}

std::vector<LootDrop> LootTable::RollWithLuck(uint32_t seed,
                                               float luckMultiplier) const {
    std::vector<LootDrop> drops;
    for (const auto& entry : entries) {
        float adjustedChance = entry.dropChance * luckMultiplier;
        if (adjustedChance > 1.0f) adjustedChance = 1.0f;

        float roll = LcgNext(seed);
        if (roll < adjustedChance) {
            LootDrop drop;
            drop.itemName = entry.itemName;
            drop.rarity   = entry.rarity;

            if (entry.maxQuantity > entry.minQuantity) {
                float qRoll = LcgNext(seed);
                int range = entry.maxQuantity - entry.minQuantity + 1;
                drop.quantity = entry.minQuantity +
                    static_cast<int>(qRoll * static_cast<float>(range));
                drop.quantity = std::min(drop.quantity, entry.maxQuantity);
            } else {
                drop.quantity = entry.minQuantity;
            }

            drops.push_back(drop);
        }
    }
    return drops;
}

LootTable LootTable::CreateStandardEnemyTable() {
    LootTable table;
    table.tableName = "StandardEnemy";
    table.entries = {
        {"Scrap Metal",       LootRarity::Common,   0.8f,  1, 5},
        {"Energy Cell",       LootRarity::Common,   0.6f,  1, 3},
        {"Weapon Part",       LootRarity::Uncommon, 0.3f,  1, 1},
        {"Shield Capacitor",  LootRarity::Rare,     0.1f,  1, 1},
        {"Advanced Module",   LootRarity::Epic,     0.03f, 1, 1},
    };
    return table;
}

LootTable LootTable::CreateBossTable() {
    LootTable table;
    table.tableName = "BossEnemy";
    table.entries = {
        {"Rare Alloy",        LootRarity::Uncommon,  0.9f,  2, 5},
        {"Prototype Weapon",  LootRarity::Rare,      0.5f,  1, 1},
        {"Shield Generator",  LootRarity::Rare,      0.4f,  1, 1},
        {"Faction Blueprint", LootRarity::Epic,      0.2f,  1, 1},
        {"Legendary Core",    LootRarity::Legendary, 0.05f, 1, 1},
    };
    return table;
}

LootTable LootTable::CreateAsteroidTable() {
    LootTable table;
    table.tableName = "Asteroid";
    table.entries = {
        {"Iron Ore",         LootRarity::Common,   0.9f,  3, 10},
        {"Titanium Ore",     LootRarity::Common,   0.6f,  2, 6},
        {"Naonite Crystal",  LootRarity::Uncommon, 0.3f,  1, 3},
        {"Trinium Shard",    LootRarity::Rare,     0.1f,  1, 2},
        {"Xanion Fragment",  LootRarity::Epic,     0.02f, 1, 1},
    };
    return table;
}

// ---------------------------------------------------------------------------
// LootComponent
// ---------------------------------------------------------------------------

std::vector<LootDrop> LootComponent::GenerateDrops() const {
    if (hasBeenLooted) return {};
    return lootTable.RollWithLuck(lootSeed, luckModifier);
}

void LootComponent::MarkLooted() {
    hasBeenLooted = true;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData LootComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "LootComponent";
    cd.data["luckModifier"]  = std::to_string(luckModifier);
    cd.data["hasBeenLooted"] = hasBeenLooted ? "1" : "0";
    cd.data["lootSeed"]      = std::to_string(lootSeed);
    cd.data["tableName"]     = lootTable.tableName;
    cd.data["entryCount"]    = std::to_string(lootTable.entries.size());

    for (size_t i = 0; i < lootTable.entries.size(); ++i) {
        std::string prefix = "entry_" + std::to_string(i) + "_";
        const auto& e = lootTable.entries[i];
        cd.data[prefix + "name"]    = e.itemName;
        cd.data[prefix + "rarity"]  = std::to_string(static_cast<int>(e.rarity));
        cd.data[prefix + "chance"]  = std::to_string(e.dropChance);
        cd.data[prefix + "minQty"]  = std::to_string(e.minQuantity);
        cd.data[prefix + "maxQty"]  = std::to_string(e.maxQuantity);
    }
    return cd;
}

void LootComponent::Deserialize(const ComponentData& data) {
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

    luckModifier  = getFloat("luckModifier", 1.0f);
    hasBeenLooted = getStr("hasBeenLooted") != "0";
    lootSeed      = static_cast<uint32_t>(getInt("lootSeed", 0));

    lootTable.tableName = getStr("tableName");
    int count = getInt("entryCount", 0);
    lootTable.entries.clear();
    lootTable.entries.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        std::string prefix = "entry_" + std::to_string(i) + "_";
        LootTableEntry entry;
        entry.itemName    = getStr(prefix + "name");

        constexpr int kMaxRarity = static_cast<int>(LootRarity::Legendary);
        int rarityVal = getInt(prefix + "rarity", 0);
        if (rarityVal >= 0 && rarityVal <= kMaxRarity) {
            entry.rarity = static_cast<LootRarity>(rarityVal);
        } else {
            entry.rarity = LootRarity::Common;
        }

        entry.dropChance  = getFloat(prefix + "chance", 1.0f);
        entry.minQuantity = getInt(prefix + "minQty", 1);
        entry.maxQuantity = getInt(prefix + "maxQty", 1);
        lootTable.entries.push_back(entry);
    }
}

// ---------------------------------------------------------------------------
// LootSystem
// ---------------------------------------------------------------------------

LootSystem::LootSystem() : SystemBase("LootSystem") {}

LootSystem::LootSystem(EntityManager& entityManager)
    : SystemBase("LootSystem")
    , _entityManager(&entityManager)
{
}

void LootSystem::Update(float /*deltaTime*/) {
    // Loot generation is on-demand via LootComponent::GenerateDrops().
    // This system exists for future event-driven loot processing.
}

} // namespace subspace
