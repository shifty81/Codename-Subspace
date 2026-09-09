#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class NormalizationStatus {
    NeedsReview,
    ActiveCpp,
    SourceToPort,
    PortedToCpp,
    ReplacedByCppDesign,
    ReferenceOnly,
    Deferred,
    Rejected
};

struct NormalizationLedgerEntry {
    std::string path;
    std::string category;
    NormalizationStatus status = NormalizationStatus::NeedsReview;
    std::string targetPath;
    std::string note;
};

struct ProjectNormalizationLedger {
    std::vector<NormalizationLedgerEntry> entries;
};

ProjectNormalizationLedger CreateDefaultSubspaceNormalizationLedger();
std::vector<NormalizationLedgerEntry> FilterNormalizationEntries(const ProjectNormalizationLedger& ledger,
                                                                 NormalizationStatus status);
bool HasUnreviewedNormalizationEntries(const ProjectNormalizationLedger& ledger);
std::string NormalizationStatusName(NormalizationStatus status);
std::string ProjectNormalizationLedgerSummary(const ProjectNormalizationLedger& ledger);

} // namespace subspace
