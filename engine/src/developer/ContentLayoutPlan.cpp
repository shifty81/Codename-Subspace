#include "developer/ContentLayoutPlan.h"

namespace subspace {

ContentLayoutPlan ContentLayoutPlan::CreateDefault()
{
    ContentLayoutPlan plan;
    plan.AddRule({"GameData", "GameData", "canonical authored gameplay/runtime data"});
    plan.AddRule({"content", "content", "canonical governed metadata/schemas/provenance"});
    plan.AddRule({"Assets", "content/assets", "legacy uppercase asset root if present"});
    plan.AddRule({"assets", "content/assets", "legacy lowercase asset root if present"});
    plan.AddRule({"engine/assets", "content/assets/engine", "engine-owned runtime assets when explicitly migrated"});
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
