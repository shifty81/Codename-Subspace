#include "content/DerivedAssetIdentitySystem.h"
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace subspace {
std::string DerivedAssetIdentitySystem::StableKey(const DerivedAssetIdentityInput&i){
    std::uint64_t h=1469598103934665603ull;
    auto feed=[&](const std::string&s){for(unsigned char c:s){h^=c;h*=1099511628211ull;}h^=0xffu;h*=1099511628211ull;};
    feed(i.sourceHash);feed(i.canonicalizerVersion);feed(i.meshCompilerVersion);feed(i.materialCompilerVersion);feed(i.spatialCompilerVersion);
    for(const auto&s:i.settings)feed(s);
    std::ostringstream out;out<<std::hex<<std::setfill('0')<<std::setw(16)<<h;return out.str();
}
} // namespace subspace
