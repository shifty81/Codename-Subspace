#pragma once

#include "ships/Ship.h"
#include "ships/Block.h"

#include <string>
#include <vector>

namespace subspace {

/// Aggregated ship capabilities derived from block composition.
/// Used by Fleet AI to make damage-aware tactical decisions.
struct ShipCapabilities {
    float mobility   = 0.0f; ///< Derived from Engine blocks — thrust potential.
    float firepower  = 0.0f; ///< Derived from WeaponMount blocks.
    float power      = 0.0f; ///< Derived from Generator blocks — energy generation.
    float command    = 0.0f; ///< Derived from Gyro blocks — maneuverability / bridge.
    float defense    = 0.0f; ///< Derived from Armor blocks — raw armor mass.
    float cargo      = 0.0f; ///< Derived from Cargo blocks — storage capacity.
    float totalMass  = 0.0f; ///< Sum of all block masses.
    int   blockCount = 0;    ///< Total number of blocks.
    int   aliveCount = 0;    ///< Blocks with currentHP > 0.

    /// Get capability as a fraction of the ship's potential (alive / total).
    float GetHealthFraction() const;

    /// Get a named capability value by string key.
    float GetCapability(const std::string& name) const;

    /// Get a short summary string for debug display.
    std::string GetSummary() const;
};

/// Evaluates ship capabilities from block composition.
/// This is a stateless utility — call Evaluate() whenever you need updated stats.
class CapabilitySystem {
public:
    /// Evaluate capabilities from a ship's block list.
    static ShipCapabilities Evaluate(const Ship& ship);

    /// Evaluate capabilities from a raw block list (for editor previews, etc.).
    static ShipCapabilities EvaluateBlocks(const std::vector<std::shared_ptr<Block>>& blocks);

    /// Get per-block-type contribution weight for a capability.
    static float GetBlockCapabilityWeight(BlockType type, const std::string& capability);

    /// Get the display name for a block type.
    static std::string GetBlockTypeName(BlockType type);
};

} // namespace subspace
