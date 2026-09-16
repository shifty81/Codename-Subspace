#pragma once

#include "rendering/ProceduralVisualVariantSystem.h"

#include <cstddef>
#include <string>

namespace subspace {

class ShipArticulationSystem {
public:
    static const char* ModeName(ShipArticulationMode mode);
    static ShipVisualArticulation* Find(ProceduralShipVisualRecipe& recipe, std::size_t moduleIndex);
    static const ShipVisualArticulation* Find(const ProceduralShipVisualRecipe& recipe, std::size_t moduleIndex);
    static ShipVisualArticulation& Ensure(ProceduralShipVisualRecipe& recipe, std::size_t moduleIndex);
    static bool Remove(ProceduralShipVisualRecipe& recipe, std::size_t moduleIndex);
    static void ReindexAfterModuleRemoval(ProceduralShipVisualRecipe& recipe,
                                          const std::vector<std::size_t>& oldToNew);
    static float EvaluateDegrees(const ShipVisualArticulation& articulation, double seconds);
    static VisualModulePlacement Apply(const VisualModulePlacement& placement,
                                       const ShipVisualArticulation& articulation,
                                       double seconds);
};

} // namespace subspace
