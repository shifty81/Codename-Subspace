#include "rendering/RenderFidelityPolicySystem.h"
namespace subspace {
AssemblyRenderFidelity RenderFidelityPolicySystem::Resolve(const RenderFidelityBudget&b){
    if(b.interiorVisible)return AssemblyRenderFidelity::Interior;
    if(b.combat||b.selected||b.distanceMeters<5000.0)return AssemblyRenderFidelity::Modules;
    if(b.distanceMeters<50000.0)return AssemblyRenderFidelity::Hlod;
    if(b.distanceMeters<500000.0)return AssemblyRenderFidelity::Impostor;
    return AssemblyRenderFidelity::StrategicIcon;
}
bool RenderFidelityPolicySystem::NeedsModuleInstances(AssemblyRenderFidelity f){return f==AssemblyRenderFidelity::Modules||f==AssemblyRenderFidelity::Interior;}
bool RenderFidelityPolicySystem::NeedsInterior(AssemblyRenderFidelity f){return f==AssemblyRenderFidelity::Interior;}
} // namespace subspace
