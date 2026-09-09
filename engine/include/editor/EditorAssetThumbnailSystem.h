#pragma once

#include "content/ShipyardModuleSystem.h"
#include "content/UniversalKitbashAuthority.h"
#include "ships/PropulsionRoleSystem.h"

#include <string>

namespace subspace {

enum class EditorThumbnailViewPreset {
    ThreeQuarter = 0,
    Axial,
    Vertical
};

enum class EditorThumbnailReviewState {
    Certified = 0,
    Review,
    ManualOnly
};

struct EditorAssetThumbnailRecord {
    std::string assetId;
    std::string previewKey;
    std::string cacheKey;
    EditorThumbnailViewPreset viewPreset = EditorThumbnailViewPreset::ThreeQuarter;
    EditorThumbnailReviewState reviewState = EditorThumbnailReviewState::Review;
    std::string sizeBadge;
    std::string certificationBadge;
    bool mirrorSupported = false;
    bool morphSupported = false;
    bool propulsion = false;
    Vector3 localThrustAxis{0.0f,1.0f,0.0f};
    Vector3 localExhaustAxis{0.0f,-1.0f,0.0f};
    std::string thrustLabel;
    std::string exhaustLabel;
};

/// Project-wide thumbnail metadata authority. Preview geometry remains the real
/// canonical/imported mesh; this record normalizes framing, cache identity,
/// certification badges and functional direction overlays across construction
/// browsers.
class EditorAssetThumbnailSystem {
public:
    static EditorAssetThumbnailRecord Build(const ShipyardModuleRecord& record,
                                            int materialRevision = 1,
                                            int previewRecipeVersion = 2);
    static std::string AxisLabel(const Vector3& axis);
    static const char* ReviewName(EditorThumbnailReviewState state);
};

} // namespace subspace
