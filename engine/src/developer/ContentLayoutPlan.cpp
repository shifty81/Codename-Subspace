#include "developer/ContentLayoutPlan.h"

namespace subspace {

ContentLayoutPlan ContentLayoutPlan::CreateDefault()
{
    ContentLayoutPlan plan;
    plan.AddRule({"Assets", "content/assets", "source and runtime art assets"});
    plan.AddRule({"assets", "content/assets", "lowercase asset mirror"});
    plan.AddRule({"GameData", "content/data", "gameplay data, definitions, and tuning"});
    plan.AddRule({"engine/assets", "content/assets/engine", "engine-owned runtime assets"});
    return plan;
}

void ContentLayoutPlan::AddRule(ContentLayoutRule rule)
{
    _rules.push_back(std::move(rule));
}

std::string ContentLayoutPlan::ResolveCanonicalRoot(const std::string& legacyRoot) const
{
    for (const auto& rule : _rules) {
        if (rule.legacyRoot == legacyRoot) {
            return rule.canonicalRoot;
        }
    }
    return legacyRoot;
}

} // namespace subspace
