#pragma once

#include "content/ShipyardModuleSystem.h"
#include "ship_editor/ShipyardAuthoringAuthority.h"

#include <vector>

namespace subspace {

/// Compatibility bridge between the existing Shipyard recipe/catalog authority
/// and the additive Pass497-506 authoring authority. This bridge never rebuilds
/// a valid recipe: it projects current placements into stable slots and writes
/// module-choice changes back onto the same placement indices/transforms.
class ShipyardAuthoringBridge {
public:
    static void RegisterCurrentCatalog(ShipyardAuthoringAuthority& authority,
                                       const std::vector<ShipyardModuleRecord>& catalog);

    static ShipyardResolvedBlueprint BuildPreservedBlueprint(ShipyardAuthoringAuthority& authority,
                                                              const ProceduralShipVisualRecipe& recipe,
                                                              const std::vector<ShipyardModuleRecord>& catalog,
                                                              std::uint32_t generatorVersion = 497);

    /// Applies only module-definition choices to the existing recipe. Position,
    /// scale, yaw/pitch/roll, material, details, anchors and hardpoints remain
    /// untouched. Returns false if slot indices no longer match the recipe.
    static bool ApplyModuleChoicesPreservingPlacement(const ShipyardResolvedBlueprint& blueprint,
                                                       ProceduralShipVisualRecipe& recipe);
};

} // namespace subspace
