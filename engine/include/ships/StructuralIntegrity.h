#pragma once

#include "ships/Block.h"
#include "ships/Ship.h"

#include <memory>
#include <vector>

namespace subspace {

/// Flood-fill based structural integrity checks for voxel ships.
class StructuralIntegrity {
public:
    /// Returns true if all blocks in the ship form a single connected group.
    static bool IsFullyConnected(const Ship& ship);

    /// Returns groups of connected blocks. The largest group is the "main" ship;
    /// any additional groups are disconnected fragments.
    static std::vector<std::vector<std::shared_ptr<Block>>> FindDisconnectedGroups(const Ship& ship);

    /// Checks whether removing the given block would split the ship into
    /// multiple disconnected pieces.
    static bool WouldDisconnect(const Ship& ship, std::shared_ptr<Block> block);
};

} // namespace subspace
