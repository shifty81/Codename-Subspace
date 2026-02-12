#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <memory>

namespace subspace {

class ShipDamage {
public:
    // Apply damage to a specific block, returns true if block was destroyed
    static bool ApplyDamage(Ship& ship, std::shared_ptr<Block> block, float damage);

    // Remove a block and recalculate stats
    static void RemoveBlock(Ship& ship, std::shared_ptr<Block> block);
};

} // namespace subspace
