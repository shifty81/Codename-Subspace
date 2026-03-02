#include "ships/ShipDamage.h"
#include "ships/BlockPlacement.h"
#include "ships/ShipStats.h"
#include "ships/StructuralIntegrity.h"

#include <algorithm>
#include <cstdlib>
#include <unordered_set>

namespace subspace {

// ---------------------------------------------------------------------------
// Existing methods
// ---------------------------------------------------------------------------

bool ShipDamage::ApplyDamage(Ship& ship, std::shared_ptr<Block> block, float damage) {
    if (!block) return false;

    float prevHP = block->currentHP;
    block->currentHP -= damage;

    if (block->currentHP <= 0.0f) {
        RemoveBlock(ship, block);
        return true;
    }

    // Only adjust totalHP by the delta; no full recalculation needed
    ship.totalHP -= (prevHP - block->currentHP);
    return false;
}

void ShipDamage::RemoveBlock(Ship& ship, std::shared_ptr<Block> block) {
    BlockPlacement::Remove(ship, block);
}

// ---------------------------------------------------------------------------
// Splash (area) damage
// ---------------------------------------------------------------------------

static int ManhattanDistance(const Vector3Int& a, const Vector3Int& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y) + std::abs(a.z - b.z);
}

int ShipDamage::ApplySplashDamage(Ship& ship, const Vector3Int& center, float damage, int radius) {
    // Collect unique blocks within radius first to avoid iterator invalidation
    std::vector<std::pair<std::shared_ptr<Block>, int>> targets;
    std::unordered_set<Block*> seen;

    for (const auto& [pos, block] : ship.occupiedCells) {
        int dist = ManhattanDistance(pos, center);
        if (dist <= radius && seen.insert(block.get()).second) {
            targets.push_back({block, dist});
        }
    }

    int blocksHit = 0;
    for (const auto& [block, dist] : targets) {
        float scaled = damage * (1.0f - static_cast<float>(dist) / static_cast<float>(radius + 1));
        if (scaled > 0.0f) {
            ApplyDamage(ship, block, scaled);
            ++blocksHit;
        }
    }
    return blocksHit;
}

// ---------------------------------------------------------------------------
// Penetrating damage
// ---------------------------------------------------------------------------

int ShipDamage::ApplyPenetratingDamage(Ship& ship, const Vector3Int& start,
                                       const Vector3Int& direction, float damage, int maxDepth) {
    int blocksHit = 0;
    Vector3Int pos = start;
    std::unordered_set<Block*> hitBlocks;
    float scaled = damage;

    for (int depth = 0; depth < maxDepth; ++depth) {
        auto it = ship.occupiedCells.find(pos);
        if (it != ship.occupiedCells.end()) {
            auto& block = it->second;
            // Only count each multi-cell block once
            if (hitBlocks.insert(block.get()).second) {
                ApplyDamage(ship, block, scaled);
                scaled *= 0.7f;
                ++blocksHit;
            }
        }
        pos = pos + direction;
    }
    return blocksHit;
}

// ---------------------------------------------------------------------------
// Repair
// ---------------------------------------------------------------------------

float ShipDamage::RepairBlock(Ship& ship, std::shared_ptr<Block> block, float repairAmount) {
    if (!block || repairAmount <= 0.0f) return 0.0f;

    float missing = block->maxHP - block->currentHP;
    if (missing <= 0.0f) return 0.0f;

    float actual = std::min(repairAmount, missing);
    block->currentHP += actual;
    ship.totalHP += actual;
    return actual;
}

float ShipDamage::RepairAll(Ship& ship, float totalRepairBudget) {
    if (totalRepairBudget <= 0.0f || ship.blocks.empty()) return 0.0f;

    // Sort blocks by HP percentage ascending (most damaged first)
    std::vector<std::shared_ptr<Block>> damaged;
    for (const auto& b : ship.blocks) {
        if (b->currentHP < b->maxHP) {
            damaged.push_back(b);
        }
    }
    std::sort(damaged.begin(), damaged.end(), [](const auto& a, const auto& b) {
        return (a->currentHP / a->maxHP) < (b->currentHP / b->maxHP);
    });

    float remaining = totalRepairBudget;
    float totalRepaired = 0.0f;

    for (const auto& block : damaged) {
        if (remaining <= 0.0f) break;
        float repaired = RepairBlock(ship, block, remaining);
        remaining -= repaired;
        totalRepaired += repaired;
    }
    return totalRepaired;
}

// ---------------------------------------------------------------------------
// Structural integrity
// ---------------------------------------------------------------------------

std::vector<std::vector<std::shared_ptr<Block>>> ShipDamage::CheckAndSeparateFragments(Ship& ship) {
    auto groups = StructuralIntegrity::FindDisconnectedGroups(ship);

    // First group is the largest (main hull). Remove all others.
    std::vector<std::vector<std::shared_ptr<Block>>> fragments;
    for (size_t i = 1; i < groups.size(); ++i) {
        for (const auto& block : groups[i]) {
            RemoveBlock(ship, block);
        }
        fragments.push_back(std::move(groups[i]));
    }
    return fragments;
}

// ---------------------------------------------------------------------------
// Query helpers
// ---------------------------------------------------------------------------

float ShipDamage::GetDamagePercentage(const Ship& ship) {
    if (ship.maxHP <= 0.0f) return 0.0f;
    return 1.0f - (ship.totalHP / ship.maxHP);
}

std::vector<std::shared_ptr<Block>> ShipDamage::GetDamagedBlocks(const Ship& ship,
                                                                  float belowPercentHP) {
    std::vector<std::shared_ptr<Block>> result;
    for (const auto& b : ship.blocks) {
        if (b->maxHP > 0.0f && (b->currentHP / b->maxHP) < belowPercentHP) {
            result.push_back(b);
        }
    }
    return result;
}

int ShipDamage::GetBlocksInRadius(const Ship& ship, const Vector3Int& center, int radius) {
    std::unordered_set<Block*> counted;
    for (const auto& [pos, block] : ship.occupiedCells) {
        if (ManhattanDistance(pos, center) <= radius) {
            counted.insert(block.get());
        }
    }
    return static_cast<int>(counted.size());
}

} // namespace subspace
