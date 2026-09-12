#include "construction/FoundryGenerationAuthoritySystem.h"
#include <cstdint>
#include <iomanip>
#include <sstream>
namespace subspace {
bool FoundryGenerationAuthoritySystem::Valid(const FoundryGenerationRequest&r){
    if(r.classId.empty()||r.roleId.empty())return false;float sum=0;for(float w:r.componentTierWeights){if(w<0)return false;sum+=w;}return sum>0;
}
float FoundryGenerationAuthoritySystem::TierWeight(const FoundryGenerationRequest&r,UniversalSizeClass t){const int i=static_cast<int>(t);return i>=0&&i<5?r.componentTierWeights[static_cast<std::size_t>(i)]:0.0f;}
std::string FoundryGenerationAuthoritySystem::StableIdentity(const FoundryGenerationRequest&r){
    std::uint64_t h=1469598103934665603ull;auto feed=[&](const std::string&s){for(unsigned char c:s){h^=c;h*=1099511628211ull;}};
    feed(std::to_string(static_cast<int>(r.domain)));feed(r.classId);feed(r.roleId);feed(r.factionId);feed(r.familyId);feed(r.doctrineId);feed(std::to_string(r.seed));
    std::ostringstream o;o<<std::hex<<std::setw(16)<<std::setfill('0')<<h;return o.str();
}
} // namespace subspace
