#include "ships/ShipArticulationSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kPi=3.14159265358979323846f;
float Rad(float d){return d*kPi/180.0f;}
Vector3 RotateAxisAligned(const Vector3& p,const Vector3& axis,float degrees){
    const float a=Rad(degrees),c=std::cos(a),s=std::sin(a);
    const float ax=std::fabs(axis.x),ay=std::fabs(axis.y),az=std::fabs(axis.z);
    if(az>=ax&&az>=ay)return {p.x*c-p.y*s,p.x*s+p.y*c,p.z};
    if(ay>=ax&&ay>=az)return {p.x*c+p.z*s,p.y,-p.x*s+p.z*c};
    return {p.x,p.y*c-p.z*s,p.y*s+p.z*c};
}
}

const char* ShipArticulationSystem::ModeName(ShipArticulationMode mode){
    switch(mode){case ShipArticulationMode::Fixed:return "FIXED";case ShipArticulationMode::Manual:return "MANUAL";case ShipArticulationMode::ScanSweep:return "SCAN SWEEP";case ShipArticulationMode::TrackTarget:return "TRACK TARGET";}return "FIXED";
}
ShipVisualArticulation* ShipArticulationSystem::Find(ProceduralShipVisualRecipe& recipe,std::size_t moduleIndex){for(auto& a:recipe.articulations)if(a.moduleIndex==moduleIndex)return &a;return nullptr;}
const ShipVisualArticulation* ShipArticulationSystem::Find(const ProceduralShipVisualRecipe& recipe,std::size_t moduleIndex){for(const auto& a:recipe.articulations)if(a.moduleIndex==moduleIndex)return &a;return nullptr;}
ShipVisualArticulation& ShipArticulationSystem::Ensure(ProceduralShipVisualRecipe& recipe,std::size_t moduleIndex){if(auto*a=Find(recipe,moduleIndex))return *a;recipe.articulations.push_back({});auto& a=recipe.articulations.back();a.moduleIndex=moduleIndex;return a;}
bool ShipArticulationSystem::Remove(ProceduralShipVisualRecipe& recipe,std::size_t moduleIndex){const auto before=recipe.articulations.size();recipe.articulations.erase(std::remove_if(recipe.articulations.begin(),recipe.articulations.end(),[&](const auto&a){return a.moduleIndex==moduleIndex;}),recipe.articulations.end());return recipe.articulations.size()!=before;}
void ShipArticulationSystem::ReindexAfterModuleRemoval(ProceduralShipVisualRecipe& recipe,const std::vector<std::size_t>& oldToNew){
    recipe.articulations.erase(std::remove_if(recipe.articulations.begin(),recipe.articulations.end(),[&](auto&a){if(a.moduleIndex>=oldToNew.size()||oldToNew[a.moduleIndex]==static_cast<std::size_t>(-1))return true;a.moduleIndex=oldToNew[a.moduleIndex];return false;}),recipe.articulations.end());
}
float ShipArticulationSystem::EvaluateDegrees(const ShipVisualArticulation& a,double seconds){
    if(!a.enabled)return a.restDegrees;
    if(a.mode==ShipArticulationMode::Manual)return std::clamp(a.currentDegrees,a.minDegrees,a.maxDegrees);
    if(a.mode==ShipArticulationMode::ScanSweep){const float lo=std::min(a.minDegrees,a.maxDegrees),hi=std::max(a.minDegrees,a.maxDegrees);const float span=std::max(1.0f,hi-lo);const float phase=static_cast<float>(seconds)*std::max(1.0f,a.speedDegreesPerSecond)/span;const float wave=.5f+.5f*std::sin(phase*2.0f*kPi);return lo+span*wave;}
    return std::clamp(a.currentDegrees,a.minDegrees,a.maxDegrees);
}
VisualModulePlacement ShipArticulationSystem::Apply(const VisualModulePlacement& placement,const ShipVisualArticulation& a,double seconds){
    VisualModulePlacement out=placement;const float angle=EvaluateDegrees(a,seconds)-a.restDegrees;if(std::fabs(angle)<.0001f)return out;
    const Vector3 pivot=a.pivotLocal;const Vector3 rotated=RotateAxisAligned(pivot,a.axisLocal,angle);out.x+=pivot.x-rotated.x;out.y+=pivot.y-rotated.y;out.z+=pivot.z-rotated.z;
    const float ax=std::fabs(a.axisLocal.x),ay=std::fabs(a.axisLocal.y),az=std::fabs(a.axisLocal.z);if(az>=ax&&az>=ay)out.yawDegrees+=angle;else if(ay>=ax&&ay>=az)out.pitchDegrees+=angle;else out.rollDegrees+=angle;return out;
}

} // namespace subspace
