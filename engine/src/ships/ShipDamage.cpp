#include "ships/ShipDamage.h"
#include "ships/BlockPlacement.h"
#include "ships/ShipStats.h"

namespace subspace {

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

} // namespace subspace
