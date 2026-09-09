#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class NormalizationStatus {
    Active,
    NeedsMove,
    NeedsLedger,
    ReferenceOnly,
    Blocked
};

struct NormalizationItem {
    std::string path;
    std::string targetPath;
    std::string reason;
    NormalizationStatus status = NormalizationStatus::Active;
};

struct ProjectNormalizationPlan {
    std::vector<NormalizationItem> rootItems;
    std::vector<NormalizationItem> contentItems;
    std::vector<NormalizationItem> migrationItems;
};

const char* NormalizationStatusName(NormalizationStatus status);
ProjectNormalizationPlan BuildDefaultSubspaceNormalizationPlan();
std::vector<NormalizationItem> CollectBlockedNormalizationItems(const ProjectNormalizationPlan& plan);
std::string ProjectNormalizationSummary(const ProjectNormalizationPlan& plan);

} // namespace subspace
