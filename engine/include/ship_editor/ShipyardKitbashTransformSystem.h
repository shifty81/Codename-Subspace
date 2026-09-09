#pragma once

#include "content/UniversalKitbashAuthority.h"
#include "editor/ConstructionSymmetrySystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

struct DerivedKitbashVariant {
    std::string variantId;
    std::string sourceAssetId;
    UniversalSizeClass size = UniversalSizeClass::M;
    VisualModulePlacement placement{};
    bool valid = false;
    std::string message;
};

class ShipyardKitbashTransformSystem {
public:
    // Compatibility X helpers now delegate to the project-wide symmetry authority.
    static VisualModulePlacement MirrorPlacementX(const VisualModulePlacement& placement);
    static ShipyardAssemblySocket MirrorSocketX(const ShipyardAssemblySocket& socket);
    static std::string MirrorLateralName(const std::string& name);
    static bool MirrorRecipeSubtreeX(ProceduralShipVisualRecipe& recipe, std::size_t rootIndex);

    static VisualModulePlacement MirrorPlacement(const VisualModulePlacement& placement,
                                                 const ConstructionSymmetryFrame& frame);
    static ShipyardAssemblySocket MirrorSocket(const ShipyardAssemblySocket& socket,
                                               ConstructionSymmetryAxis axis);
    static bool MirrorRecipeSubtree(ProceduralShipVisualRecipe& recipe,std::size_t rootIndex,
                                    const ConstructionSymmetryFrame& frame);

    static DerivedKitbashVariant DeriveSizeVariant(const UniversalKitbashProfile& profile,
                                                   const VisualModulePlacement& sourcePlacement,
                                                   UniversalSizeClass targetSize);
};

} // namespace subspace
