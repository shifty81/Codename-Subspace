#pragma once

#include <string>
#include <vector>

namespace subspace {

struct QualityGateCheck {
    std::string id;
    std::string label;
    bool pass = false;
    std::string detail;
};

struct QualityGateReport {
    std::string gateName;
    std::vector<QualityGateCheck> checks;
};

QualityGateReport CreateSubspaceBuildGateReport(bool patchesApplied, bool rootAuditPass, bool buildPass, bool testsPass, bool clientPresent);
bool QualityGatePassed(const QualityGateReport& report);
std::string QualityGateSummary(const QualityGateReport& report);

} // namespace subspace
