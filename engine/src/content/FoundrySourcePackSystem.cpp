#include "content/FoundrySourcePackSystem.h"
namespace subspace {
bool FoundrySourcePackSystem::HasSufficientProvenance(const FoundrySourcePack&p){
    return !p.provider.empty()&&!p.packId.empty()&&!p.revision.empty()&&!p.sourceUrl.empty()&&!p.contentSha256.empty()&&
           !p.license.licenseId.empty()&&!p.license.licenseTextHash.empty()&&!p.license.acquisitionDate.empty();
}
bool FoundrySourcePackSystem::RuntimeEligible(const FoundrySourcePack&p){
    return p.certification==SourcePackCertification::Certified&&HasSufficientProvenance(p);
}
bool FoundrySourcePackSystem::RequiresLicenseReview(const FoundrySourcePack&p){
    return p.license.licenseId.empty()||p.license.licenseTextHash.empty()||p.license.acquisitionDate.empty();
}
} // namespace subspace
