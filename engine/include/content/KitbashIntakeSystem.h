#pragma once

#include "content/UniversalKitbashAuthority.h"

#include <string>
#include <vector>

namespace subspace {

enum class KitbashCertificationState {
    Certified,
    ManualPlacementOnly,
    ReviewRequired,
    Quarantined
};

struct KitbashIntakeRecord {
    std::string sourceId;
    std::string sourceRevision;
    UniversalKitbashProfile profile;
    KitbashCertificationState state{KitbashCertificationState::ReviewRequired};
    std::vector<std::string> reviewReasons;
};

class KitbashIntakeSystem {
public:
    static KitbashIntakeRecord Classify(
        std::string sourceId,
        std::string sourceRevision,
        const ShipyardModuleRecord& module,
        KitbashMaterialCertification materialCertification);

    static std::vector<KitbashIntakeRecord> ReviewQueue(const std::vector<KitbashIntakeRecord>& records);
    static std::vector<KitbashIntakeRecord> PcgCatalog(const std::vector<KitbashIntakeRecord>& records);
    static std::vector<KitbashIntakeRecord> ProjectDomain(
        const std::vector<KitbashIntakeRecord>& records,
        ConstructionDomain domain);

    static const char* ToString(KitbashCertificationState state);
};

} // namespace subspace
