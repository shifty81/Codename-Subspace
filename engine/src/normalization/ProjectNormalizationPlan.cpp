#include "normalization/ProjectNormalizationPlan.h"

#include <sstream>

namespace subspace {

const char* NormalizationStatusName(NormalizationStatus status) {
    switch (status) {
        case NormalizationStatus::Active: return "ACTIVE";
        case NormalizationStatus::NeedsMove: return "NEEDS_MOVE";
        case NormalizationStatus::NeedsLedger: return "NEEDS_LEDGER";
        case NormalizationStatus::ReferenceOnly: return "REFERENCE_ONLY";
        case NormalizationStatus::Blocked: return "BLOCKED";
        default: return "UNKNOWN";
    }
}

ProjectNormalizationPlan BuildDefaultSubspaceNormalizationPlan() {
    ProjectNormalizationPlan plan;
    plan.rootItems.push_back({"engine/", "engine/", "active C++ CMake source root", NormalizationStatus::Active});
    plan.rootItems.push_back({"SubspaceTools.ps1", "SubspaceTools.ps1", "root project control entry point", NormalizationStatus::Active});
    plan.rootItems.push_back({"AvorionLike/", "reference/csharp-to-cpp-source/AvorionLike/", "source-to-port backlog; not active build target", NormalizationStatus::NeedsLedger});
    plan.rootItems.push_back({"AvorionLike.sln", "reference/csharp-to-cpp-source/AvorionLike.sln", "legacy solution retained for conversion reference", NormalizationStatus::ReferenceOnly});
    plan.contentItems.push_back({"Assets/", "content/assets/legacy/", "legacy assets require active-path audit", NormalizationStatus::NeedsMove});
    plan.contentItems.push_back({"assets/", "content/assets/", "lowercase asset tree should become canonical after audit", NormalizationStatus::NeedsMove});
    plan.contentItems.push_back({"GameData/", "content/data/", "legacy data needs path audit before move", NormalizationStatus::NeedsMove});
    plan.migrationItems.push_back({"docs/migration/", "docs/migration/", "conversion ledger authority", NormalizationStatus::Active});
    plan.migrationItems.push_back({"reference/third_party/pixel_planets/", "reference/third_party/pixel_planets/", "third-party visual reference/provenance", NormalizationStatus::ReferenceOnly});
    return plan;
}

std::vector<NormalizationItem> CollectBlockedNormalizationItems(const ProjectNormalizationPlan& plan) {
    std::vector<NormalizationItem> blocked;
    auto scan = [&blocked](const std::vector<NormalizationItem>& items) {
        for (const auto& item : items) {
            if (item.status == NormalizationStatus::Blocked || item.status == NormalizationStatus::NeedsLedger) {
                blocked.push_back(item);
            }
        }
    };
    scan(plan.rootItems); scan(plan.contentItems); scan(plan.migrationItems);
    return blocked;
}

std::string ProjectNormalizationSummary(const ProjectNormalizationPlan& plan) {
    const auto blocked = CollectBlockedNormalizationItems(plan);
    std::ostringstream ss;
    ss << "root=" << plan.rootItems.size()
       << " content=" << plan.contentItems.size()
       << " migration=" << plan.migrationItems.size()
       << " blockedOrLedger=" << blocked.size();
    return ss.str();
}

} // namespace subspace
