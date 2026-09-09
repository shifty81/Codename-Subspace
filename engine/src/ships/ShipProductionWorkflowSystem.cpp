#include "ships/ShipProductionWorkflowSystem.h"

#include <algorithm>

namespace subspace {
ShipBillOfMaterials ShipProductionWorkflowSystem::ApplySubstitution(const ShipBillOfMaterials&src,const MaterialSubstitutionRule&r) const {auto b=src;auto from=static_cast<int>(r.from),to=static_cast<int>(r.to);auto it=b.quantities.find(from);if(it==b.quantities.end()||it->second<=0)return b;const double q=it->second;b.quantities.erase(it);b.quantities[to]+=q*std::max(.01,r.quantityMultiplier);b.totalMass*=std::max(.01,r.massMultiplier);b.constructionMinutes*=std::max(.01,r.timeMultiplier);return b;}
std::uint64_t ShipProductionWorkflowSystem::Queue(const ModularShipDesign&d,double me,double te){auto validation=construction_.Validate(d);if(!validation.valid)return 0;ShipyardJob j;j.id=nextId_++;j.blueprintName=d.name;j.bill=construction_.BuildBill(d);j.materialEfficiency=std::clamp(me,.5,1.25);j.timeEfficiency=std::clamp(te,.5,1.25);for(auto&kv:j.bill.quantities)kv.second*=j.materialEfficiency;j.bill.constructionMinutes*=j.timeEfficiency;jobs_[j.id]=j;return j.id;}
bool ShipProductionWorkflowSystem::Supply(std::uint64_t id,std::unordered_map<int,double>&inv){auto it=jobs_.find(id);if(it==jobs_.end()||it->second.state!=ShipyardJobState::WaitingMaterials)return false;for(const auto&req:it->second.bill.quantities){auto stock=inv.find(req.first);if(stock==inv.end()||stock->second+1e-6<req.second)return false;}for(const auto&req:it->second.bill.quantities)inv[req.first]-=req.second;it->second.state=ShipyardJobState::Queued;return true;}
void ShipProductionWorkflowSystem::Advance(std::uint64_t id,double minutes){auto it=jobs_.find(id);if(it==jobs_.end()||minutes<=0)return;auto&j=it->second;if(j.state==ShipyardJobState::Queued)j.state=ShipyardJobState::Building;if(j.state!=ShipyardJobState::Building)return;j.progress=std::clamp(j.progress+minutes/std::max(.01,j.bill.constructionMinutes),0.0,1.0);if(j.progress>=1)j.state=ShipyardJobState::Complete;}
const ShipyardJob* ShipProductionWorkflowSystem::Get(std::uint64_t id) const {auto it=jobs_.find(id);return it==jobs_.end()?nullptr:&it->second;}
std::vector<ShipyardJob> ShipProductionWorkflowSystem::Jobs() const {std::vector<ShipyardJob>v;for(const auto&kv:jobs_)v.push_back(kv.second);std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;});return v;}
} // namespace subspace
