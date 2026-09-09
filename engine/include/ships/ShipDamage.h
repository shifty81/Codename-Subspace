#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <memory>
#include <vector>

namespace subspace {

class ShipDamage {
public:
    // Apply damage to a specific block, returns true if block was destroyed
    static bool ApplyDamage(Ship& ship, std::shared_ptr<Block> block, float damage);

    // Remove a block and recalculate stats
    static void RemoveBlock(Ship& ship, std::shared_ptr<Block> block);

    // Area / splash damage using Manhattan distance fall-off.
    // Returns the number of blocks damaged.
    static int ApplySplashDamage(Ship& ship, const Vector3Int& center, float damage, int radius);

    // Directional penetrating damage along a cardinal axis.
    // Damage is reduced by 0.7x per block penetrated. Returns blocks hit.
    static int ApplyPenetratingDamage(Ship& ship, const Vector3Int& start,
                                      const Vector3Int& direction, float damage, int maxDepth);

    // Repair a single block. Returns actual HP restored.
    static float RepairBlock(Ship& ship, std::shared_ptr<Block> block, float repairAmount);

    // Distribute repair budget to the most damaged blocks first.
    // Returns total HP repaired.
    static float RepairAll(Ship& ship, float totalRepairBudget);

    // Run structural integrity check; remove fragments from the ship.
    // Returns the fragment groups (excluding the main hull).
    static std::vector<std::vector<std::shared_ptr<Block>>> CheckAndSeparateFragments(Ship& ship);

    // Ship-wide damage percentage (0.0 = pristine, 1.0 = fully destroyed).
    static float GetDamagePercentage(const Ship& ship);

    // Collect blocks whose HP is below the given fraction of their maxHP.
    static std::vector<std::shared_ptr<Block>> GetDamagedBlocks(const Ship& ship,
                                                                 float belowPercentHP = 0.5f);

    // Count blocks whose cells fall within Manhattan radius of center.
    static int GetBlocksInRadius(const Ship& ship, const Vector3Int& center, int radius);
};

} // namespace subspace
