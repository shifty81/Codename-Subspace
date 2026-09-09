#include "home/HomeHabitableWorldRules.h"

namespace subspace {

HomeHabitableWorldRuleReport EvaluateHomeHabitableWorldRules(const HomeWorldSafetyOptions& options) {
    HomeHabitableWorldRuleReport report;
    report.constructionAllowed = true;
    report.combatAllowed = !options.safeHomeSystem || options.homeRaidsEnabled;
    report.structureDamageAllowed = report.combatAllowed && options.homeStructureDamageEnabled;
    report.ruleNotes.push_back(options.safeHomeSystem ? "Home system is safe by default." : "Home safety disabled by world config.");
    report.ruleNotes.push_back(options.offlineProgressEnabled ? "Automation can progress while away." : "Offline automation disabled.");
    if (!report.structureDamageAllowed) report.ruleNotes.push_back("Home structures persist unless world config allows damage.");
    return report;
}

} // namespace subspace
