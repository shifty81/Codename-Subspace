#pragma once

#include "inventory/ItemizationSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "content/ShipyardModuleSystem.h"
#include "core/Math.h"

#include <cstdint>
#include <vector>

namespace subspace {

struct ScoopableItemDrop {
    std::uint64_t dropId = 0;
    GeneratedItem item{};
    Vector3 position{};
    Vector3 velocity{};
    float spinRadians = 0.0f;
    float compactScale = 0.18f;
    float pickupRadius = 0.55f;
    float attractionRange = 4.5f;
    bool collected = false;
};

/// Compact item drops carry the resolved item instance directly. Their world
/// representation is intentionally small regardless of the source module's
/// construction size so engines/hulls can be safely scooped into cargo.
class ScoopableLootSystem {
public:
    static ScoopableItemDrop CreateDrop(const GeneratedItem& item,
                                        const Vector3& position,
                                        const Vector3& impulse,
                                        std::uint64_t dropId);

    static std::vector<ScoopableItemDrop> BuildDestroyedShipDrops(
        const ProceduralShipVisualRecipe& ship,
        const std::vector<ShipyardModuleRecord>& catalog,
        const Vector3& destructionPosition,
        std::uint32_t seed,
        float moduleRecoveryChance = 0.32f,
        float blueprintFragmentChance = 0.28f);

    static void Update(std::vector<ScoopableItemDrop>& drops,
                       const Vector3& collectorPosition,
                       float collectorRange,
                       float deltaSeconds,
                       std::vector<GeneratedItem>* collectedItems = nullptr);
};

} // namespace subspace
