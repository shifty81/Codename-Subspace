#pragma once

#include "content/KitbashIntakeSystem.h"
#include "content/UniversalKitbashAuthority.h"
#include "construction/UniversalConstructionSystem.h"
#include "ships/ShipPcgRuntimeClosureSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class KitbashReviewKind {
    Classification,
    Socket,
    Orientation,
    Propulsion,
    Material,
    SurfaceSegmentation,
    Morph,
    PcgCertification,
    DuplicateCandidate
};

struct KitbashReviewItem {
    std::string assetId;
    KitbashReviewKind kind = KitbashReviewKind::Classification;
    std::string reason;
    float confidence = 0.0f;
    bool blockingPcg = true;
};

struct RuntimeKitbashCatalogs {
    std::vector<std::string> ship;
    std::vector<std::string> station;
    std::vector<std::string> planetary;
    std::vector<std::string> weapon;
    std::vector<std::string> manualOnly;
    std::vector<std::string> quarantined;
};

struct KitbashContentCertificationSummary {
    std::size_t total = 0;
    std::size_t certified = 0;
    std::size_t review = 0;
    std::size_t manualOnly = 0;
    std::size_t quarantined = 0;
    std::size_t materialFallback = 0;
    std::size_t propulsionReview = 0;
    RuntimeKitbashCatalogs catalogs;
    std::vector<KitbashReviewItem> queue;
    bool fullGateReady = false;
};

class KitbashReviewCatalogSystem {
public:
    // Pass652: central review queue feeds Authoring instead of silently making
    // uncertain inference authoritative.
    static std::vector<KitbashReviewItem> BuildReviewQueue(const std::vector<ShipyardModuleRecord>& catalog,
                                                            KitbashMaterialCertification defaultMaterialState = KitbashMaterialCertification::NormalizedFallback);

    // Pass653: runtime catalogs are materialized from universal domain roles and
    // certification rather than hand-maintained lists.
    static RuntimeKitbashCatalogs Materialize(const std::vector<ShipyardModuleRecord>& catalog,
                                              KitbashMaterialCertification defaultMaterialState = KitbashMaterialCertification::NormalizedFallback);

    // Pass654: Full Gate summary is deterministic, fail-closed and suitable for
    // both local UI/reporting and later CI enforcement.
    static KitbashContentCertificationSummary Certify(const std::vector<ShipyardModuleRecord>& catalog,
                                                       KitbashMaterialCertification defaultMaterialState = KitbashMaterialCertification::NormalizedFallback);
};

} // namespace subspace
