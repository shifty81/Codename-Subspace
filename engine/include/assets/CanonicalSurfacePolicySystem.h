#pragma once

#include "assets/CanonicalAsset.h"

#include <string>
#include <vector>

namespace subspace::assets {

class CanonicalSurfacePolicySystem {
public:
    static SurfaceAuthoringPolicy Infer(AssetIndex materialIndex,
                                        const PbrMaterial& material,
                                        const std::string& sourceName = {});
    static std::vector<SurfaceAuthoringPolicy> Build(const CanonicalAsset& asset);
    static const char* SemanticName(SurfaceSemantic semantic);
};

} // namespace subspace::assets
