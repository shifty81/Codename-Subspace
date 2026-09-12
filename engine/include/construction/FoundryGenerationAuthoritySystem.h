#pragma once
#include "content/UniversalKitbashAuthority.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace subspace {
struct GenerationDoctrine {
    std::string doctrineId;
    float exposedMachinery=0.5f,automation=0.5f,armorIntegration=0.5f,symmetry=0.7f,modularity=0.7f,cleanliness=0.5f;
    std::vector<std::string> preferredSemanticTags;
    std::vector<std::string> discouragedSemanticTags;
};
struct FoundryGenerationRequest {
    ConstructionDomain domain=ConstructionDomain::Ship;
    std::string classId,roleId,factionId,familyId,doctrineId;
    std::uint32_t seed=1;
    std::array<float,5> componentTierWeights{{.2f,.4f,.3f,.1f,0}};
};
class FoundryGenerationAuthoritySystem {
public:
    static bool Valid(const FoundryGenerationRequest& request);
    static float TierWeight(const FoundryGenerationRequest& request,UniversalSizeClass tier);
    static std::string StableIdentity(const FoundryGenerationRequest& request);
};
} // namespace subspace
