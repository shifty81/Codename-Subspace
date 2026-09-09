#pragma once

#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace subspace {

enum class ShipSpatialRegion {
    Bow,
    ForwardHull,
    Command,
    MidHull,
    AftHull,
    Port,
    Starboard,
    Dorsal,
    Ventral,
    Propulsion
};

struct ShipOccupancyBox {
    std::size_t moduleIndex = 0;
    Vector3 center{};
    Vector3 halfExtents{.5f,.5f,.5f};
    bool detail = false;
    bool command = false;
    bool propulsion = false;
};

struct ShipSpatialAssemblyReport {
    bool valid = true;
    int unintendedOverlapPairs = 0;
    int detailClusterCells = 0;
    int recursiveDetailAttachments = 0;
    bool commandBuried = false;
    std::vector<std::pair<std::size_t,std::size_t>> overlapPairs;
    std::vector<std::string> warnings;
};

class ShipSpatialAssemblySystem {
public:
    static std::vector<ShipOccupancyBox> BuildOccupancy(const std::vector<ShipyardModuleRecord>& catalog,
                                                         const ProceduralShipVisualRecipe& recipe);
    static ShipSpatialRegion ClassifyRegion(const VisualModulePlacement& placement,
                                            const ProceduralShipVisualRecipe& recipe);
    static ShipSpatialAssemblyReport Validate(const std::vector<ShipyardModuleRecord>& catalog,
                                              const ProceduralShipVisualRecipe& recipe,
                                              float allowedParentOverlapRatio = 0.35f);
};

} // namespace subspace
