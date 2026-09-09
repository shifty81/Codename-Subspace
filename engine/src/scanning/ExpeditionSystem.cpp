#include "scanning/ExpeditionSystem.h"

#include <algorithm>
#include <cmath>
namespace subspace {
std::vector<DirectionalScanContact> ExpeditionSystem::DirectionalScan(const std::vector<ExplorationSignature>&sigs,double center,double cone,double strength) const {std::vector<DirectionalScanContact>out;cone=std::clamp(cone,5.0,360.0);for(const auto&s:sigs){double bearing=std::fmod(static_cast<double>(s.id)*137.507764+31.0,360.0);double delta=std::fabs(bearing-center);delta=std::min(delta,360.0-delta);if(delta<=cone*.5){double signal=std::clamp(strength/std::max(.05,s.difficulty)*(1.0-delta/std::max(1.0,cone)),0.0,1.0);out.push_back({s.id,bearing,signal,s.resolved});}}return out;}
bool ExpeditionSystem::AdvanceChain(ExpeditionChain&c,std::uint64_t resolved) const {if(c.completed||c.currentStage>=c.stages.size()||c.stages[c.currentStage]!=resolved)return false;++c.currentStage;if(c.currentStage>=c.stages.size())c.completed=true;return true;}
double ExpeditionSystem::ProbeTimeSeconds(double difficulty,double strength) const {return std::clamp(18.0*std::max(.05,difficulty)/std::max(.05,strength),2.0,180.0);}
} // namespace subspace
