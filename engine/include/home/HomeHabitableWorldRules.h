#pragma once

#include <string>
#include <vector>

namespace subspace {

struct HomeWorldSafetyOptions {
    bool safeHomeSystem = true;
    bool homeRaidsEnabled = false;
    bool homeStructureDamageEnabled = false;
    bool offlineProgressEnabled = true;
};

struct HomeHabitableWorldRuleReport {
    bool constructionAllowed = true;
    bool combatAllowed = false;
    bool structureDamageAllowed = false;
    std::vector<std::string> ruleNotes;
};

HomeHabitableWorldRuleReport EvaluateHomeHabitableWorldRules(const HomeWorldSafetyOptions& options);

} // namespace subspace
