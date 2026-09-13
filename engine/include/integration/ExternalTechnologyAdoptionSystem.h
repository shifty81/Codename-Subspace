#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ExternalTechnologyDisposition {
    EvaluateForIntegration = 0,
    SelectiveAdoption,
    ArchitectureReference,
    Defer
};

struct ExternalTechnologyCandidate {
    std::string id;
    std::string project;
    std::string repository;
    std::string license;
    ExternalTechnologyDisposition disposition = ExternalTechnologyDisposition::ArchitectureReference;
    std::string subspaceUse;
    std::string blockerOrConstraint;
};

/// Project-level registry for external open-source engine technology. This is
/// intentionally an adoption assessment rather than an automatic dependency
/// list: Subspace keeps one engine/runtime authority and only integrates code
/// when a bounded interface and license/dependency review exists.
class ExternalTechnologyAdoptionSystem {
public:
    static std::vector<ExternalTechnologyCandidate> CarbonCandidates();
    static const char* DispositionName(ExternalTechnologyDisposition disposition);
};

} // namespace subspace
