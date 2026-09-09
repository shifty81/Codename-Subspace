#include "developer/QualityGateReport.h"

#include <sstream>

namespace subspace {

QualityGateReport CreateSubspaceBuildGateReport(bool patchesApplied, bool rootAuditPass, bool buildPass, bool testsPass, bool clientPresent) {
    QualityGateReport report;
    report.gateName = "Subspace Full Gate";
    report.checks.push_back({"patches", "Patch handoff processed", patchesApplied, patchesApplied ? "updates applied or none queued" : "patch stage failed"});
    report.checks.push_back({"root", "Root audit", rootAuditPass, rootAuditPass ? "clean" : "root audit failed"});
    report.checks.push_back({"build", "C++ build", buildPass, buildPass ? "compiled" : "build failed"});
    report.checks.push_back({"tests", "Tests", testsPass, testsPass ? "passed" : "test failure"});
    report.checks.push_back({"client", "Playable client", clientPresent, clientPresent ? "present" : "missing executable"});
    return report;
}

bool QualityGatePassed(const QualityGateReport& report) {
    for (const auto& check : report.checks) if (!check.pass) return false;
    return true;
}

std::string QualityGateSummary(const QualityGateReport& report) {
    std::ostringstream out;
    out << report.gateName << ": " << (QualityGatePassed(report) ? "PASS" : "FAIL");
    for (const auto& check : report.checks) out << " | " << check.id << "=" << (check.pass ? "PASS" : "FAIL");
    return out.str();
}

} // namespace subspace
