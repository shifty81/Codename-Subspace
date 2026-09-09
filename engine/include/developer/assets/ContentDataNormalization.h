#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ContentNormalizationActionKind {
    Keep,
    MoveToContentAssets,
    MoveToContentData,
    MoveToReference,
    ReviewManually
};

struct ContentNormalizationAction {
    ContentNormalizationActionKind kind = ContentNormalizationActionKind::ReviewManually;
    std::string source;
    std::string target;
    std::string reason;
};

struct ContentNormalizationPlan {
    std::vector<ContentNormalizationAction> actions;
};

ContentNormalizationPlan BuildDefaultContentNormalizationPlan();
std::string ContentNormalizationActionKindName(ContentNormalizationActionKind kind);
std::string ContentNormalizationPlanSummary(const ContentNormalizationPlan& plan);

} // namespace subspace
