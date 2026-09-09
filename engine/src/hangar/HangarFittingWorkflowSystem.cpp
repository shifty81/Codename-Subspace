#include "hangar/HangarFittingWorkflowSystem.h"

#include <algorithm>
namespace subspace {
FittingResult HangarFittingWorkflowSystem::Install(HangarFitState&s,const std::string&id) const {auto r=fitting_.CanInstall(s.capacity,s.installed,id);if(r.installed)s.installed.push_back(id);return r;}
bool HangarFittingWorkflowSystem::Remove(HangarFitState&s,const std::string&id) const {auto it=std::find(s.installed.begin(),s.installed.end(),id);if(it==s.installed.end())return false;s.installed.erase(it);return true;}
HangarServiceQuote HangarFittingWorkflowSystem::Quote(const HangarFitState&s,double fuel,double ammo) const {HangarServiceQuote q;q.repairCost=(1.0-std::clamp(s.repairPercent,0.0,1.0))*3200.0;q.refuelCost=std::max(0.0,fuel-s.fuel)*2.5;q.ammoCost=std::max(0.0,ammo-s.ammunition)*4.0;q.total=q.repairCost+q.refuelCost+q.ammoCost;return q;}
void HangarFittingWorkflowSystem::Service(HangarFitState&s,double fuel,double ammo) const {s.repairPercent=1.0;s.fuel=std::max(s.fuel,fuel);s.ammunition=std::max(s.ammunition,ammo);}
} // namespace subspace
