#include "developer/assets/ConversionNormalizationGate.h"

namespace subspace {

const char* ConversionGateStatusName(ConversionGateStatus status) {
    switch (status) {
    case ConversionGateStatus::Pending: return "PENDING";
    case ConversionGateStatus::Ported: return "PORTED_TO_CPP";
    case ConversionGateStatus::Replaced: return "REPLACED_BY_CPP_DESIGN";
    case ConversionGateStatus::ReferenceOnly: return "REFERENCE_ONLY";
    case ConversionGateStatus::Deferred: return "DEFERRED";
    case ConversionGateStatus::Rejected: return "REJECTED_WITH_REASON";
    }
    return "UNKNOWN";
}

ConversionGateReport BuildConversionGateReport(const std::vector<ConversionGateEntry>& entries) {
    ConversionGateReport report;
    for (const auto& entry : entries) {
        switch (entry.status) {
        case ConversionGateStatus::Pending: ++report.pending; report.warnings.push_back("Pending conversion: " + entry.sourcePath); break;
        case ConversionGateStatus::Ported: ++report.ported; break;
        case ConversionGateStatus::Replaced: ++report.replaced; break;
        case ConversionGateStatus::ReferenceOnly: ++report.referenceOnly; break;
        case ConversionGateStatus::Deferred: ++report.deferred; break;
        case ConversionGateStatus::Rejected: ++report.rejected; break;
        }
    }
    return report;
}

bool IsConversionGateCleanEnoughForArchive(const ConversionGateReport& report) {
    return report.pending == 0;
}

} // namespace subspace
