#include "interior/PortalGraphSystem.h"
#include <queue>
#include <unordered_set>
namespace subspace {
bool PortalGraphSystem::IsConnected(CompartmentId from,CompartmentId to,const std::vector<CompartmentPortal>&p,bool requireOpen){
    if(from==to)return true;std::queue<CompartmentId>q;std::unordered_set<CompartmentId>seen;seen.insert(from);q.push(from);
    while(!q.empty()){const auto n=q.front();q.pop();for(const auto&e:p){if(e.damaged){/* damaged boundary behaves open */}else if(requireOpen&&!e.open)continue;if(e.sealed&&!e.open&&!e.damaged)continue;CompartmentId next=ExteriorCompartmentId;if(e.a==n)next=e.b;else if(e.b==n)next=e.a;else continue;if(next==to)return true;if(seen.insert(next).second)q.push(next);}}return false;
}
bool PortalGraphSystem::HasExteriorLeak(CompartmentId from,const std::vector<CompartmentPortal>&p){return IsConnected(from,ExteriorCompartmentId,p,true);}
} // namespace subspace
