#include "content/KitbashIntakeSystem.h"

#include <algorithm>

namespace subspace {

namespace {
bool HasDomain(const UniversalKitbashProfile& profile, ConstructionDomain domain) {
    return std::any_of(profile.domainRoles.begin(), profile.domainRoles.end(),
        [domain](const KitbashDomainRole& role) { return role.domain == domain && !role.role.empty(); });
}
}

KitbashIntakeRecord KitbashIntakeSystem::Classify(
    std::string sourceId,
    std::string sourceRevision,
    const ShipyardModuleRecord& module,
    KitbashMaterialCertification materialCertification) {

    KitbashIntakeRecord out;
    out.sourceId = std::move(sourceId);
    out.sourceRevision = std::move(sourceRevision);
    out.profile = UniversalKitbashAuthority::BuildProfile(module, materialCertification);

    if (materialCertification == KitbashMaterialCertification::BrokenDependency) {
        out.state = KitbashCertificationState::Quarantined;
        out.reviewReasons.push_back("broken material dependency");
        return out;
    }

    if (materialCertification == KitbashMaterialCertification::ReviewRequired) {
        out.state = KitbashCertificationState::ReviewRequired;
        out.reviewReasons.push_back("material review required");
    }

    if (out.profile.domainRoles.empty()) {
        out.state = KitbashCertificationState::ReviewRequired;
        out.reviewReasons.push_back("no certified construction-domain role");
    }

    if (!module.functional && module.moduleClass != ShipyardModuleClass::Detail) {
        out.state = KitbashCertificationState::ManualPlacementOnly;
        out.reviewReasons.push_back("non-functional module classification");
    }

    if (module.sockets.empty() && module.moduleClass != ShipyardModuleClass::Detail) {
        out.state = KitbashCertificationState::ManualPlacementOnly;
        out.reviewReasons.push_back("no certified attachment socket");
    }

    if (!module.generatorEligible) {
        out.state = KitbashCertificationState::ManualPlacementOnly;
        out.reviewReasons.push_back("generator eligibility disabled");
    }

    if (out.reviewReasons.empty() && UniversalKitbashAuthority::IsPcgEligible(out.profile)) {
        out.state = KitbashCertificationState::Certified;
    } else if (out.state == KitbashCertificationState::ReviewRequired && UniversalKitbashAuthority::IsPcgEligible(out.profile)) {
        // Keep explicit review state when the asset can technically render but
        // still needs semantic/material authoring review.
    } else if (out.state != KitbashCertificationState::Quarantined && out.state != KitbashCertificationState::ReviewRequired) {
        out.state = KitbashCertificationState::ManualPlacementOnly;
    }

    return out;
}

std::vector<KitbashIntakeRecord> KitbashIntakeSystem::ReviewQueue(const std::vector<KitbashIntakeRecord>& records) {
    std::vector<KitbashIntakeRecord> out;
    for (const auto& record : records) {
        if (record.state == KitbashCertificationState::ReviewRequired ||
            record.state == KitbashCertificationState::ManualPlacementOnly) {
            out.push_back(record);
        }
    }
    return out;
}

std::vector<KitbashIntakeRecord> KitbashIntakeSystem::PcgCatalog(const std::vector<KitbashIntakeRecord>& records) {
    std::vector<KitbashIntakeRecord> out;
    for (const auto& record : records) {
        if (record.state == KitbashCertificationState::Certified && UniversalKitbashAuthority::IsPcgEligible(record.profile)) out.push_back(record);
    }
    return out;
}

std::vector<KitbashIntakeRecord> KitbashIntakeSystem::ProjectDomain(
    const std::vector<KitbashIntakeRecord>& records,
    ConstructionDomain domain) {
    std::vector<KitbashIntakeRecord> out;
    for (const auto& record : records) if (HasDomain(record.profile, domain)) out.push_back(record);
    return out;
}

const char* KitbashIntakeSystem::ToString(KitbashCertificationState state) {
    switch (state) {
        case KitbashCertificationState::Certified: return "CERTIFIED";
        case KitbashCertificationState::ManualPlacementOnly: return "MANUAL_PLACEMENT_ONLY";
        case KitbashCertificationState::ReviewRequired: return "REVIEW_REQUIRED";
        case KitbashCertificationState::Quarantined: return "QUARANTINED";
    }
    return "UNKNOWN";
}

} // namespace subspace
