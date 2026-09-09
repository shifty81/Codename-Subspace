#pragma once

#include "content/ShipyardModuleSystem.h"
#include "content/UniversalKitbashAuthority.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

struct ShipyardSnapCandidate {
    std::size_t parentModuleIndex = 0;
    std::string parentSocket;
    std::string childSocket;
    VisualModulePlacement placement{};
    float score = 0.0f;
    bool collisionRisk = false;
    float pointerDistance = 0.0f;
};

struct ShipyardDragPreview {
    bool active = false;
    std::string moduleId;
    VisualModulePlacement ghost{};
    std::vector<ShipyardSnapCandidate> candidates;
    int selectedCandidate = -1;
    bool valid = false;
    bool snapped = false;
    bool freePlacement = false;
    float snapRadius = 1.0f;
    UniversalSizeClass requestedSize = UniversalSizeClass::M;
    UniversalSizeClass resolvedSize = UniversalSizeClass::M;
    float resolvedUniformScale = 1.0f;
    bool sizeAdjusted = false;
    // Pass655-674 live symmetry preview: the cursor carries the real module and
    // optionally renders an exact reflected partner before commit.
    bool mirroredPreviewActive = false;
    bool mirroredValid = false;
    VisualModulePlacement mirroredGhost{};
    std::string status;
};

class ShipyardDragDropSystem {
public:
    static ShipyardDragPreview Begin(const ShipyardModuleRecord& child,
                                     const std::vector<ShipyardModuleRecord>& catalog,
                                     const ProceduralShipVisualRecipe& recipe);
    static ShipyardDragPreview Begin(const ShipyardModuleRecord& child,
                                     const std::vector<ShipyardModuleRecord>& catalog,
                                     const ProceduralShipVisualRecipe& recipe,
                                     UniversalSizeClass targetSize);
    static bool SelectBest(ShipyardDragPreview& preview);
};

} // namespace subspace
