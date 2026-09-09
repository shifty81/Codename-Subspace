#include "trade_route/LogisticsAutomationSystem.h"
#include <algorithm>
namespace subspace {
bool LogisticsAutomationSystem::RegisterNode(const AutomatedLogisticsNode& n){if(n.id.empty()||n.throughputPerHour<0)return false;nodes_[n.id]=n;return true;}
bool LogisticsAutomationSystem::RegisterRoute(const AutomatedRoute& r){if(r.id.empty()||r.from.empty()||r.to.empty()||r.commodity.empty()||r.from==r.to||r.maxPerRun<=0||nodes_.find(r.from)==nodes_.end()||nodes_.find(r.to)==nodes_.end())return false;routes_[r.id]=r;return true;}
double LogisticsAutomationSystem::AdvanceRoute(const std::string& id,double h){auto it=routes_.find(id);if(it==routes_.end()||!it->second.enabled||h<=0)return 0;auto& r=it->second;auto& a=nodes_.at(r.from);auto& b=nodes_.at(r.to);double available=a.inventory[r.commodity]-a.minimum[r.commodity];double need=std::max(0.0,b.target[r.commodity]-b.inventory[r.commodity]);double throughput=std::min(a.throughputPerHour,b.throughputPerHour)*h;double moved=std::max(0.0,std::min({available,need,r.maxPerRun,throughput}));a.inventory[r.commodity]-=moved;b.inventory[r.commodity]+=moved;r.movedTotal+=moved;return moved;}
const AutomatedLogisticsNode* LogisticsAutomationSystem::Node(const std::string& id) const {auto it=nodes_.find(id);return it==nodes_.end()?nullptr:&it->second;}
const AutomatedRoute* LogisticsAutomationSystem::Route(const std::string& id) const {auto it=routes_.find(id);return it==routes_.end()?nullptr:&it->second;}
}
