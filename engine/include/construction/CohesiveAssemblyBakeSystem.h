#pragma once

#include "content/ShipyardModuleSystem.h"
#include "editor/AuthoringStandardsSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

struct CohesiveBakeElement {
    std::size_t moduleIndex=0;
    std::string moduleId;
    bool exteriorUnion=true;
    bool interiorVolumeCandidate=false;
    bool removeOccludedFaces=true;
};

struct CohesiveBakePortalCut {
    std::size_t parentModuleIndex=0;
    std::size_t childModuleIndex=0;
    std::string parentSocket;
    std::string childSocket;
    float allowedPenetrationMeters=0.0f;
};

struct CohesiveAssemblyBakePlan {
    std::string sourceRecipeId;
    std::string outputAssetId;
    std::string outputObjName;
    CohesiveAssemblyBakePolicy policy{};
    std::vector<CohesiveBakeElement> elements;
    std::vector<CohesiveBakePortalCut> portalCuts;
    bool characterScaleCompatible=false;
    bool readyForBooleanBake=false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/// Builds the deterministic mesh-bake contract for a saved assembly. The
/// editable module assembly remains source authority; derived output is one
/// welded exterior shell with occluded/internal exterior faces removed and a
/// character-compatible carved interior/portal volume.
class CohesiveAssemblyBakeSystem {
public:
    static CohesiveAssemblyBakePlan BuildPlan(const ProceduralShipVisualRecipe& recipe,
                                              const std::vector<ShipyardModuleRecord>& catalog,
                                              const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
    static bool ValidatePenetration(const VisualAssemblyAttachment& attachment,
                                    const ShipyardModuleRecord& parent,
                                    const ShipyardModuleRecord& child,
                                    const CohesiveAssemblyBakePolicy& policy,
                                    float* allowedMeters=nullptr);
};

} // namespace subspace
