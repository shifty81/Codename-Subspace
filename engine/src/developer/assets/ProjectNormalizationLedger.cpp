#include "developer/assets/ProjectNormalizationLedger.h"

#include <map>
#include <sstream>

namespace subspace {

ProjectNormalizationLedger CreateDefaultSubspaceNormalizationLedger() {
    ProjectNormalizationLedger ledger;
    ledger.entries.push_back({"engine/", "active-source", NormalizationStatus::ActiveCpp, "engine/", "Current C++ engine/client authority."});
    ledger.entries.push_back({"AvorionLike/", "legacy-csharp", NormalizationStatus::SourceToPort, "reference/csharp-to-cpp-source/AvorionLike/", "Do not delete until each behavior is ported or rejected."});
    ledger.entries.push_back({"Assets/", "legacy-assets", NormalizationStatus::NeedsReview, "content/assets/", "Move only after path audit."});
    ledger.entries.push_back({"assets/", "legacy-assets", NormalizationStatus::NeedsReview, "content/assets/", "Normalize duplicate lower-case asset root."});
    ledger.entries.push_back({"GameData/", "legacy-data", NormalizationStatus::NeedsReview, "content/data/", "Port data contracts to C++ runtime."});
    ledger.entries.push_back({"reference/third_party/pixel_planets/", "third-party-reference", NormalizationStatus::ReferenceOnly, "reference/third_party/pixel_planets/", "MIT reference/provenance material; port concepts, not dependency."});
    return ledger;
}

std::vector<NormalizationLedgerEntry> FilterNormalizationEntries(const ProjectNormalizationLedger& ledger,
                                                                 NormalizationStatus status) {
    std::vector<NormalizationLedgerEntry> out;
    for (const auto& entry : ledger.entries) {
        if (entry.status == status) out.push_back(entry);
    }
    return out;
}

bool HasUnreviewedNormalizationEntries(const ProjectNormalizationLedger& ledger) {
    for (const auto& entry : ledger.entries) {
        if (entry.status == NormalizationStatus::NeedsReview) return true;
    }
    return false;
}

std::string NormalizationStatusName(NormalizationStatus status) {
    switch (status) {
        case NormalizationStatus::NeedsReview: return "NEEDS_REVIEW";
        case NormalizationStatus::ActiveCpp: return "ACTIVE_CPP";
        case NormalizationStatus::SourceToPort: return "SOURCE_TO_PORT";
        case NormalizationStatus::PortedToCpp: return "PORTED_TO_CPP";
        case NormalizationStatus::ReplacedByCppDesign: return "REPLACED_BY_CPP_DESIGN";
        case NormalizationStatus::ReferenceOnly: return "REFERENCE_ONLY";
        case NormalizationStatus::Deferred: return "DEFERRED";
        case NormalizationStatus::Rejected: return "REJECTED";
        default: return "UNKNOWN";
    }
}

std::string ProjectNormalizationLedgerSummary(const ProjectNormalizationLedger& ledger) {
    std::map<std::string, int> counts;
    for (const auto& entry : ledger.entries) {
        counts[NormalizationStatusName(entry.status)]++;
    }
    std::ostringstream stream;
    stream << "NormalizationLedger entries=" << ledger.entries.size();
    for (const auto& pair : counts) {
        stream << " " << pair.first << "=" << pair.second;
    }
    return stream.str();
}

} // namespace subspace
