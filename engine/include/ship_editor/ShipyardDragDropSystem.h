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
    // Mouse release enters a staged authoring state. The ghost remains editable
    // until explicit confirmation; releasing the palette drag is never the same
    // operation as attaching a module.
    bool staged = false;
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
    static bool Stage(ShipyardDragPreview& preview);
    static bool CycleCandidate(ShipyardDragPreview& preview, int delta);
    static bool TranslateStaged(ShipyardDragPreview& preview, const Vector3& delta, bool snap = true, float snapStep = 0.25f);
    static bool RotateStaged(ShipyardDragPreview& preview, const Vector3& deltaDegrees, bool snap = true, float rotationStepDegrees = 15.0f);
    static bool ScaleStaged(ShipyardDragPreview& preview, float delta, float minimum = 0.20f, float maximum = 4.0f);
    static bool ScaleStaged(ShipyardDragPreview& preview, const Vector3& delta, float minimum = 0.20f, float maximum = 4.0f);
};

} // namespace subspace
