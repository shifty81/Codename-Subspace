#pragma once
#include "runtime/AssemblyResidencySystem.h"
namespace subspace {
struct RenderFidelityBudget {double distanceMeters=0.0;bool interiorVisible=false;bool selected=false;bool combat=false;};
class RenderFidelityPolicySystem {
public:
    static AssemblyRenderFidelity Resolve(const RenderFidelityBudget& budget);
    static bool NeedsModuleInstances(AssemblyRenderFidelity fidelity);
    static bool NeedsInterior(AssemblyRenderFidelity fidelity);
};
} // namespace subspace
