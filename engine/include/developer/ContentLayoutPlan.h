#pragma once

#include <string>
#include <vector>

namespace subspace {

struct ContentLayoutRule {
    std::string legacyRoot;
    std::string canonicalRoot;
    std::string purpose;
};

class ContentLayoutPlan {
public:
    static ContentLayoutPlan CreateDefault();
    void AddRule(ContentLayoutRule rule);
    const std::vector<ContentLayoutRule>& GetRules() const { return _rules; }
    std::string ResolveCanonicalRoot(const std::string& legacyRoot) const;

private:
    std::vector<ContentLayoutRule> _rules;
};

} // namespace subspace
