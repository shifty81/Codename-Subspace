#include "client/VisualAcceptanceModel.h"

#include <sstream>

namespace subspace {

VisualAcceptanceReport CreateSubspaceVisualAcceptanceChecklist() {
    VisualAcceptanceReport report;
    report.items = {
        {"home-surface-primary", "Home", "H opens a large Factorio-like habitable home-world surface view", VisualAcceptanceSeverity::Blocker, false},
        {"home-overview-secondary", "Home", "Solar-system overview is secondary and shows outposts/routes", VisualAcceptanceSeverity::Warning, false},
        {"planet-interaction", "Sector", "Planets are selectable/scannable world objects, not decorative backdrop icons", VisualAcceptanceSeverity::Blocker, false},
        {"sun-light", "Sector", "Star light intensity affects HUD and future gameplay modifiers", VisualAcceptanceSeverity::Warning, true},
        {"thruster-fx", "Flight", "Main/RCS thrusters emit readable particles and support fuel-saving coast mode", VisualAcceptanceSeverity::Warning, true},
        {"ship-builder-home-only", "Shipyard", "Hot-swappable modular ship parts install at home before launch", VisualAcceptanceSeverity::Blocker, false},
        {"rail-travel", "Travel", "Interstellar travel is on-rails with route risk/reward/fitting checks", VisualAcceptanceSeverity::Warning, true}
    };
    for (const auto& item : report.items) {
        if (item.complete) ++report.complete;
        if (!item.complete && item.severity == VisualAcceptanceSeverity::Blocker) ++report.blockers;
        if (!item.complete && item.severity == VisualAcceptanceSeverity::Warning) ++report.warnings;
    }
    return report;
}

std::string VisualAcceptanceSeverityName(VisualAcceptanceSeverity severity) {
    switch (severity) {
        case VisualAcceptanceSeverity::Info: return "Info";
        case VisualAcceptanceSeverity::Warning: return "Warning";
        case VisualAcceptanceSeverity::Blocker: return "Blocker";
    }
    return "Unknown";
}

std::string VisualAcceptanceReportSummary(const VisualAcceptanceReport& report) {
    std::ostringstream out;
    out << "visualAcceptance items=" << report.items.size()
        << " complete=" << report.complete
        << " blockers=" << report.blockers
        << " warnings=" << report.warnings;
    return out.str();
}

} // namespace subspace
