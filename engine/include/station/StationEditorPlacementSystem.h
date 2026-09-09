#pragma once

#include "core/Math.h"
#include "editor/EditorAssetBrowser.h"
#include "station/StationKitbashCatalogSystem.h"
#include "station/StationKitbashVisualSystem.h"

namespace subspace {

class StationEditorPlacementSystem {
public:
    static EditorPlacementResolution Resolve(const StationKitbashVisualRecipe& recipe,
                                             const std::vector<ShipyardModuleRecord>& catalog,
                                             const StationKitbashPiece& draggedPiece,
                                             const Vector3& pointerLocal);
};

} // namespace subspace
