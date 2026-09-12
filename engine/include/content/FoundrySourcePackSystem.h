#pragma once
#include "content/UniversalKitbashAuthority.h"
#include <string>
#include <vector>

namespace subspace {

enum class SourcePackCertification { Intake, Review, Certified, ReferenceOnly, Quarantined };

struct SourceLicenseEvidence {
    std::string licenseId;
    std::string licenseTextHash;
    std::string sourceUrl;
    std::string acquisitionDate;
    bool redistributionOfRawSourceAllowed = false;
};

struct FoundrySourcePack {
    std::string provider;
    std::string packId;
    std::string revision;
    std::string sourceUrl;
    std::string contentSha256;
    SourceLicenseEvidence license{};
    std::vector<ConstructionDomain> allowedDomains;
    SourcePackCertification certification = SourcePackCertification::Intake;
};

class FoundrySourcePackSystem {
public:
    static bool HasSufficientProvenance(const FoundrySourcePack& pack);
    static bool RuntimeEligible(const FoundrySourcePack& pack);
    static bool RequiresLicenseReview(const FoundrySourcePack& pack);
};

} // namespace subspace
