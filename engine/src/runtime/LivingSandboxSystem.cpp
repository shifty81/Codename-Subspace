#include "runtime/LivingSandboxSystem.h"

#include <algorithm>
namespace subspace {
void LivingSandboxSystem::Advance(LivingRegionState&r,double hours) const {hours=std::max(0.0,hours);double step=hours;if(r.lod==SimulationLod::Dormant)step*=.10;else if(r.lod==SimulationLod::Aggregate)step*=.55;else if(r.lod==SimulationLod::Regional)step*=.82;for(auto&npc:r.corporations){lod_.AdvanceEconomy(npc,NpcEconomicAction::Mine,step);lod_.AdvanceEconomy(npc,NpcEconomicAction::Refine,step);lod_.AdvanceEconomy(npc,NpcEconomicAction::Manufacture,step);lod_.AdvanceEconomy(npc,NpcEconomicAction::ReplaceLosses,step);}local_.Tick(r.economy,step);r.prosperity=std::clamp(r.prosperity+(r.corporations.empty()?-.002:.003)*step-r.threat*.001*step,0.0,1.0);r.simulatedHours+=static_cast<std::uint64_t>(hours);}
std::vector<LivingSandboxEvent> LivingSandboxSystem::EvaluateEvents(const LivingRegionState&r) const {std::vector<LivingSandboxEvent>e;if(r.threat>.72)e.push_back({r.id,"Sector crisis pressure rising",r.threat});for(const auto&kv:r.economy.commodities)if(kv.second.stock<kv.second.desiredStock*.35)e.push_back({r.id,"Shortage: "+kv.first,1.0-kv.second.stock/std::max(1.0,kv.second.desiredStock)});if(r.prosperity>.82)e.push_back({r.id,"Industrial expansion opportunity",r.prosperity});return e;}
bool LivingSandboxSystem::EconomyIsCausal(const LivingRegionState&r) const {for(const auto&kv:r.economy.commodities)if(kv.second.priceHistory.size()>1)return true;return false;}
} // namespace subspace
