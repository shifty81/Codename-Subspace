#pragma once

#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "ship_editor/ShipyardEquipmentSystem.h"

#include <string>
#include <vector>

namespace subspace {

/// Reviewable generator-refinement export produced by the standalone Shipyard.
///
/// This is intentionally not an opaque "learn automatically" path.  Player
/// edits are preserved as explicit baseline -> edited deltas so they can be
/// reviewed, uploaded, promoted, and converted into deterministic generator
/// rules without silently teaching the runtime from every experimental draft.
class ShipyardAuthoringSampleSystem {
public:
    static std::string Serialize(const ProceduralShipVisualRecipe& baseline,
                                 const ProceduralShipVisualRecipe& edited,
                                 const ShipAppearanceState& appearance,
                                 const std::vector<ShipyardModuleRecord>& catalog,
                                 const std::vector<std::string>& validationErrors,
                                 const std::vector<std::string>& validationWarnings,
                                 const std::string& author = "PLAYER");

    static bool Save(const ProceduralShipVisualRecipe& baseline,
                     const ProceduralShipVisualRecipe& edited,
                     const ShipAppearanceState& appearance,
                     const std::vector<ShipyardModuleRecord>& catalog,
                     const std::vector<std::string>& validationErrors,
                     const std::vector<std::string>& validationWarnings,
                     const std::string& path,
                     std::string* error = nullptr,
                     const std::string& author = "PLAYER");
};

} // namespace subspace
