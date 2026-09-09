#include "fleet/CorporationOperationsSystem.h"

#include <algorithm>
namespace subspace {
std::uint64_t CorporationOperationsSystem::CreateWing(const std::string&n,FleetWingRole r){if(n.empty())return 0;auto id=nextWing_++;wings_[id]={id,n,r,{},FleetStandingOrder::Hold,0};return id;}
bool CorporationOperationsSystem::AssignShip(std::uint64_t id,std::uint64_t ship){auto it=wings_.find(id);if(it==wings_.end()||ship==0)return false;if(std::find(it->second.ships.begin(),it->second.ships.end(),ship)==it->second.ships.end())it->second.ships.push_back(ship);return true;}
bool CorporationOperationsSystem::IssueStandingOrder(std::uint64_t id,FleetStandingOrder o,std::uint64_t target){auto it=wings_.find(id);if(it==wings_.end())return false;it->second.order=o;it->second.target=target;return true;}
const FleetWingNative* CorporationOperationsSystem::GetWing(std::uint64_t id) const {auto it=wings_.find(id);return it==wings_.end()?nullptr:&it->second;}
double CorporationOperationsSystem::LogisticsReadiness(const CorporationAssetLedger&a) const {return std::clamp(a.ships.size()*.04+a.stations.size()*.16+a.planetaryOperations.size()*.22+a.credits/5e7,0.0,1.0);}
} // namespace subspace
