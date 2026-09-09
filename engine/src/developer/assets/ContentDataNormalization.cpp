#include "developer/assets/ContentDataNormalization.h"

#include <sstream>

namespace subspace {

ContentNormalizationPlan BuildDefaultContentNormalizationPlan()
{
    ContentNormalizationPlan plan;
    plan.actions.push_back({ContentNormalizationActionKind::MoveToContentAssets, "Assets/Models", "content/assets/models", "legacy art/source asset folder"});
    plan.actions.push_back({ContentNormalizationActionKind::MoveToContentData, "GameData", "content/data", "legacy gameplay data folder"});
    plan.actions.push_back({ContentNormalizationActionKind::MoveToContentAssets, "assets/module_packs", "content/assets/module_packs", "active module pack content"});
    plan.actions.push_back({ContentNormalizationActionKind::MoveToReference, "reference/third_party", "reference/third_party", "third-party provenance source"});
    plan.actions.push_back({ContentNormalizationActionKind::ReviewManually, "AvorionLike", "reference/csharp-to-cpp-source", "migration source until port ledger says complete"});
    return plan;
}

std::string ContentNormalizationActionKindName(ContentNormalizationActionKind kind)
{
    switch (kind) {
        case ContentNormalizationActionKind::Keep: return "Keep";
        case ContentNormalizationActionKind::MoveToContentAssets: return "MoveToContentAssets";
        case ContentNormalizationActionKind::MoveToContentData: return "MoveToContentData";
        case ContentNormalizationActionKind::MoveToReference: return "MoveToReference";
        case ContentNormalizationActionKind::ReviewManually: return "ReviewManually";
        default: return "Unknown";
    }
}

std::string ContentNormalizationPlanSummary(const ContentNormalizationPlan& plan)
{
    std::ostringstream stream;
    stream << "content-normalization actions=" << plan.actions.size();
    return stream.str();
}

} // namespace subspace
