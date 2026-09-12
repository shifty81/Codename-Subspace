#pragma once
#include <string>
#include <vector>

namespace subspace {
struct DerivedAssetIdentityInput {
    std::string sourceHash;
    std::string canonicalizerVersion;
    std::string meshCompilerVersion;
    std::string materialCompilerVersion;
    std::string spatialCompilerVersion;
    std::vector<std::string> settings;
};
class DerivedAssetIdentitySystem {
public:
    static std::string StableKey(const DerivedAssetIdentityInput& input);
};
} // namespace subspace
