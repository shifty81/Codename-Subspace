#include "economy/LocalEconomySystem.h"

#include <algorithm>
namespace subspace {
void LocalEconomySystem::Register(LocalEconomyState&s,const std::string&c,double stock,double desired,double base) const {s.commodities[c]={std::max(0.0,stock),std::max(1.0,desired),std::max(.01,base),std::max(.01,base),{std::max(.01,base)}};}
void LocalEconomySystem::ApplySupplyShock(LocalEconomyState&s,const std::string&c,double delta) const {auto it=s.commodities.find(c);if(it==s.commodities.end())return;it->second.stock=std::max(0.0,it->second.stock+delta);}
void LocalEconomySystem::Tick(LocalEconomyState&s,double hours) const {hours=std::max(0.0,hours);for(auto&kv:s.commodities){auto&c=kv.second;const double ratio=c.desiredStock/std::max(1.0,c.stock);const double pressure=std::clamp(ratio,.35,4.0);c.currentPrice=c.basePrice*(.65+.35*pressure)*(.9+.1*s.industrialDemand);c.stock=std::max(0.0,c.stock-hours*c.desiredStock*.01*s.industrialDemand);c.priceHistory.push_back(c.currentPrice);if(c.priceHistory.size()>64)c.priceHistory.erase(c.priceHistory.begin());}}
SandboxContract LocalEconomySystem::GenerateContract(const LocalEconomyState&s,const std::string&commodity) const {SandboxContract c;c.career=ContractCareer::Logistics;c.faction=s.id;auto it=s.commodities.find(commodity);if(it==s.commodities.end()){c.title="Unknown procurement";return c;}const auto&x=it->second;const bool shortage=x.stock<x.desiredStock*.6;c.title=(shortage?"Emergency supply: ":"Procurement: ")+commodity;c.rewardCredits=x.currentPrice*std::max(50.0,x.desiredStock-x.stock)*1.2;c.minimumStanding=shortage?-.5:-.1;return c;}
} // namespace subspace
