#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class VisualAcceptanceSeverity {
    Info,
    Warning,
    Blocker
};

struct VisualAcceptanceItem {
    std::string id;
    std::string area;
    std::string requirement;
    VisualAcceptanceSeverity severity = VisualAcceptanceSeverity::Info;
    bool complete = false;
};

struct VisualAcceptanceReport {
    std::vector<VisualAcceptanceItem> items;
    int blockers = 0;
    int warnings = 0;
    int complete = 0;
};

VisualAcceptanceReport CreateSubspaceVisualAcceptanceChecklist();
std::string VisualAcceptanceSeverityName(VisualAcceptanceSeverity severity);
std::string VisualAcceptanceReportSummary(const VisualAcceptanceReport& report);

} // namespace subspace
