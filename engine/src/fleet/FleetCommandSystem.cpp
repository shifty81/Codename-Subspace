#include "fleet/FleetCommandSystem.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace subspace {
namespace {
std::string JoinIds(const std::vector<std::uint64_t>& ids){
    std::ostringstream out;
    for(std::size_t i=0;i<ids.size();++i){if(i)out<<',';out<<ids[i];}
    return out.str();
}
std::vector<std::uint64_t> ParseIds(const std::string& text){
    std::vector<std::uint64_t> out;std::size_t start=0;
    while(start<text.size()){
        const auto end=text.find(',',start);const auto token=text.substr(start,end==std::string::npos?std::string::npos:end-start);
        try{if(!token.empty())out.push_back(std::stoull(token));}catch(...){}
        if(end==std::string::npos)break;start=end+1;
    }
    return out;
}
}

std::string FleetOrder::GetOrderTypeName(FleetOrderType type) {
    switch (type) {
        case FleetOrderType::Idle:return "Idle";case FleetOrderType::Patrol:return "Patrol";
        case FleetOrderType::Mine:return "Mine";case FleetOrderType::Trade:return "Trade";
        case FleetOrderType::Attack:return "Attack";case FleetOrderType::Escort:return "Escort";
        case FleetOrderType::Defend:return "Defend";case FleetOrderType::Scout:return "Scout";
        case FleetOrderType::Move:return "Move";case FleetOrderType::Approach:return "Approach";
        case FleetOrderType::Orbit:return "Orbit";case FleetOrderType::Hold:return "Hold";
        case FleetOrderType::Salvage:return "Salvage";case FleetOrderType::Dock:return "Dock";
        case FleetOrderType::Warp:return "Warp";case FleetOrderType::Repair:return "Repair";
        case FleetOrderType::Resupply:return "Resupply";
    } return "Unknown";
}
std::string FleetOrder::GetOrderStateName(FleetOrderState state) {
    switch(state){case FleetOrderState::Pending:return "Pending";case FleetOrderState::Active:return "Active";
        case FleetOrderState::Paused:return "Paused";case FleetOrderState::Completed:return "Completed";
        case FleetOrderState::Failed:return "Failed";case FleetOrderState::Blocked:return "Blocked";}return "Unknown";
}
std::string FleetOrder::GetRoleName(FleetRole role) {
    switch(role){case FleetRole::Flagship:return "Flagship";case FleetRole::Combat:return "Combat";
        case FleetRole::Mining:return "Mining";case FleetRole::Trading:return "Trading";
        case FleetRole::Support:return "Support";case FleetRole::Scout:return "Scout";}return "Unknown";
}

FleetCommandComponent::FleetCommandComponent(const std::string& fleetName):_fleetName(fleetName){}
const std::string& FleetCommandComponent::GetFleetName() const{return _fleetName;}
void FleetCommandComponent::SetFleetName(const std::string& name){_fleetName=name;}
int FleetCommandComponent::GetMaxMembers() const{return _maxMembers;}
void FleetCommandComponent::SetMaxMembers(int max){_maxMembers=std::max(1,max);}
void FleetCommandComponent::SetMaxOrders(int max){_maxOrders=std::max(1,max);}
int FleetCommandComponent::GetMemberCount() const{return static_cast<int>(_members.size());}
int FleetCommandComponent::GetActiveMemberCount() const{int count=0;for(const auto&m:_members)if(m.isActive)++count;return count;}
bool FleetCommandComponent::AddMember(std::uint64_t entityId,const std::string& shipName,FleetRole role){
    if(!entityId||static_cast<int>(_members.size())>=_maxMembers)return false;
    for(const auto&m:_members)if(m.entityId==entityId)return false;
    _members.push_back({entityId,shipName,role,1.0f,true});return true;
}
bool FleetCommandComponent::RemoveMember(std::uint64_t entityId){auto it=std::find_if(_members.begin(),_members.end(),[&](const FleetMember&m){return m.entityId==entityId;});if(it==_members.end())return false;_members.erase(it);return true;}
const FleetMember* FleetCommandComponent::GetMember(std::uint64_t entityId)const{for(const auto&m:_members)if(m.entityId==entityId)return &m;return nullptr;}
const std::vector<FleetMember>& FleetCommandComponent::GetAllMembers()const{return _members;}

bool FleetCommandComponent::IssueOrder(FleetOrderType type,float x,float y,float z,std::uint64_t target,int priority){
    FleetOrderRequest r;r.type=type;r.targetX=x;r.targetY=y;r.targetZ=z;r.targetEntityId=target;r.priority=priority;r.queue=true;return IssueOrder(r);
}
bool FleetCommandComponent::IssueOrder(const FleetOrderRequest& r){
    const int live=static_cast<int>(std::count_if(_orders.begin(),_orders.end(),[](const FleetOrder&o){return o.state==FleetOrderState::Pending||o.state==FleetOrderState::Active||o.state==FleetOrderState::Paused||o.state==FleetOrderState::Blocked;}));
    if(live>=_maxOrders)return false;
    if(!r.queue){
        for(auto&o:_orders){if(o.state==FleetOrderState::Pending||o.state==FleetOrderState::Active||o.state==FleetOrderState::Paused||o.state==FleetOrderState::Blocked){o.state=FleetOrderState::Failed;o.status="SUPERSEDED BY NEW ORDER";}}
    }
    FleetOrder o;o.orderId=_nextOrderId++;o.type=r.type;o.state=FleetOrderState::Pending;o.targetEntityId=r.targetEntityId;
    o.targetX=r.targetX;o.targetY=r.targetY;o.targetZ=r.targetZ;o.priority=r.priority;o.progress=0.0f;o.sequence=_nextSequence++;
    o.acceptanceRadius=std::max(0.1f,r.acceptanceRadius);o.orbitRadius=std::max(0.0f,r.orbitRadius);o.assignedMembers=r.assignedMembers;o.status="QUEUED";
    _orders.push_back(std::move(o));return true;
}
bool FleetCommandComponent::CancelOrder(int id){auto it=std::find_if(_orders.begin(),_orders.end(),[&](const FleetOrder&o){return o.orderId==id;});if(it==_orders.end())return false;_orders.erase(it);return true;}
FleetOrder* FleetCommandComponent::GetMutableOrder(int id){for(auto&o:_orders)if(o.orderId==id)return &o;return nullptr;}
const FleetOrder* FleetCommandComponent::GetOrder(int id)const{for(const auto&o:_orders)if(o.orderId==id)return &o;return nullptr;}
bool FleetCommandComponent::PauseOrder(int id){auto*o=GetMutableOrder(id);if(!o||o->state!=FleetOrderState::Active)return false;o->state=FleetOrderState::Paused;o->status="PAUSED";return true;}
bool FleetCommandComponent::ResumeOrder(int id){auto*o=GetMutableOrder(id);if(!o||(o->state!=FleetOrderState::Paused&&o->state!=FleetOrderState::Blocked))return false;o->state=FleetOrderState::Pending;o->status="QUEUED";return true;}
bool FleetCommandComponent::CompleteOrder(int id,const std::string&s){auto*o=GetMutableOrder(id);if(!o)return false;o->state=FleetOrderState::Completed;o->progress=1.0f;o->status=s;return true;}
bool FleetCommandComponent::FailOrder(int id,const std::string&s){auto*o=GetMutableOrder(id);if(!o)return false;o->state=FleetOrderState::Failed;o->status=s;return true;}
bool FleetCommandComponent::BlockOrder(int id,const std::string&s){auto*o=GetMutableOrder(id);if(!o)return false;o->state=FleetOrderState::Blocked;o->status=s;return true;}
const std::vector<FleetOrder>& FleetCommandComponent::GetAllOrders()const{return _orders;}
int FleetCommandComponent::GetActiveOrderCount()const{return static_cast<int>(std::count_if(_orders.begin(),_orders.end(),[](const FleetOrder&o){return o.state==FleetOrderState::Active;}));}
int FleetCommandComponent::GetPendingOrderCount()const{return static_cast<int>(std::count_if(_orders.begin(),_orders.end(),[](const FleetOrder&o){return o.state==FleetOrderState::Pending;}));}
float FleetCommandComponent::GetAverageMorale()const{if(_members.empty())return 0.0f;float total=0;for(const auto&m:_members)total+=m.morale;return total/static_cast<float>(_members.size());}
bool FleetCommandComponent::SetMemberMorale(std::uint64_t id,float morale){for(auto&m:_members)if(m.entityId==id){m.morale=std::clamp(morale,0.0f,1.0f);return true;}return false;}
bool FleetCommandComponent::SetMemberRole(std::uint64_t id,FleetRole role){for(auto&m:_members)if(m.entityId==id){m.role=role;return true;}return false;}

ComponentData FleetCommandComponent::Serialize()const{
    ComponentData cd;cd.componentType="FleetCommandComponent";cd.data["fleetName"]=_fleetName;cd.data["maxMembers"]=std::to_string(_maxMembers);cd.data["maxOrders"]=std::to_string(_maxOrders);cd.data["nextOrderId"]=std::to_string(_nextOrderId);cd.data["nextSequence"]=std::to_string(_nextSequence);
    cd.data["memberCount"]=std::to_string(_members.size());
    for(std::size_t i=0;i<_members.size();++i){const auto&m=_members[i];const auto p="member_"+std::to_string(i)+"_";cd.data[p+"entityId"]=std::to_string(m.entityId);cd.data[p+"shipName"]=m.shipName;cd.data[p+"role"]=std::to_string(static_cast<int>(m.role));cd.data[p+"morale"]=std::to_string(m.morale);cd.data[p+"isActive"]=m.isActive?"1":"0";}
    cd.data["orderCount"]=std::to_string(_orders.size());
    for(std::size_t i=0;i<_orders.size();++i){const auto&o=_orders[i];const auto p="order_"+std::to_string(i)+"_";cd.data[p+"orderId"]=std::to_string(o.orderId);cd.data[p+"type"]=std::to_string(static_cast<int>(o.type));cd.data[p+"state"]=std::to_string(static_cast<int>(o.state));cd.data[p+"targetEntityId"]=std::to_string(o.targetEntityId);cd.data[p+"targetX"]=std::to_string(o.targetX);cd.data[p+"targetY"]=std::to_string(o.targetY);cd.data[p+"targetZ"]=std::to_string(o.targetZ);cd.data[p+"priority"]=std::to_string(o.priority);cd.data[p+"progress"]=std::to_string(o.progress);cd.data[p+"sequence"]=std::to_string(o.sequence);cd.data[p+"acceptanceRadius"]=std::to_string(o.acceptanceRadius);cd.data[p+"orbitRadius"]=std::to_string(o.orbitRadius);cd.data[p+"assignedMembers"]=JoinIds(o.assignedMembers);cd.data[p+"status"]=o.status;}
    return cd;
}
void FleetCommandComponent::Deserialize(const ComponentData& data){
    auto s=[&](const std::string&k){auto it=data.data.find(k);return it==data.data.end()?std::string{}:it->second;};
    auto i=[&](const std::string&k,int d=0){try{auto v=s(k);return v.empty()?d:std::stoi(v);}catch(...){return d;}};
    auto f=[&](const std::string&k,float d=0.0f){try{auto v=s(k);return v.empty()?d:std::stof(v);}catch(...){return d;}};
    auto u=[&](const std::string&k,std::uint64_t d=0){try{auto v=s(k);return v.empty()?d:static_cast<std::uint64_t>(std::stoull(v));}catch(...){return d;}};
    _fleetName=s("fleetName");if(_fleetName.empty())_fleetName="Fleet";_maxMembers=i("maxMembers",10);_maxOrders=i("maxOrders",5);_nextOrderId=i("nextOrderId",1);_nextSequence=u("nextSequence",1);
    _members.clear();for(int n=0;n<i("memberCount",0);++n){const auto p="member_"+std::to_string(n)+"_";FleetMember m;m.entityId=u(p+"entityId");m.shipName=s(p+"shipName");const int rv=i(p+"role",1);m.role=rv>=0&&rv<=static_cast<int>(FleetRole::Scout)?static_cast<FleetRole>(rv):FleetRole::Combat;m.morale=f(p+"morale",1);m.isActive=i(p+"isActive",1)!=0;_members.push_back(m);}
    _orders.clear();for(int n=0;n<i("orderCount",0);++n){const auto p="order_"+std::to_string(n)+"_";FleetOrder o;o.orderId=i(p+"orderId");const int tv=i(p+"type");o.type=tv>=0&&tv<=static_cast<int>(FleetOrderType::Resupply)?static_cast<FleetOrderType>(tv):FleetOrderType::Idle;const int sv=i(p+"state");o.state=sv>=0&&sv<=static_cast<int>(FleetOrderState::Blocked)?static_cast<FleetOrderState>(sv):FleetOrderState::Pending;o.targetEntityId=u(p+"targetEntityId");o.targetX=f(p+"targetX");o.targetY=f(p+"targetY");o.targetZ=f(p+"targetZ");o.priority=i(p+"priority");o.progress=std::clamp(f(p+"progress"),0.0f,1.0f);o.sequence=u(p+"sequence",static_cast<std::uint64_t>(n+1));o.acceptanceRadius=std::max(0.1f,f(p+"acceptanceRadius",5.0f));o.orbitRadius=std::max(0.0f,f(p+"orbitRadius"));o.assignedMembers=ParseIds(s(p+"assignedMembers"));o.status=s(p+"status");_orders.push_back(std::move(o));}
}

FleetCommandSystem::FleetCommandSystem():SystemBase("FleetCommandSystem"){}
FleetCommandSystem::FleetCommandSystem(EntityManager& em):SystemBase("FleetCommandSystem"),_entityManager(&em){}
void FleetCommandSystem::SetEntityManager(EntityManager* em){_entityManager=em;}
void FleetCommandSystem::RegisterExecutor(FleetOrderType type,FleetOrderExecutor executor){if(executor)_executors[type]=std::move(executor);else _executors.erase(type);}
void FleetCommandSystem::ClearExecutor(FleetOrderType type){_executors.erase(type);}
bool FleetCommandSystem::HasExecutor(FleetOrderType type)const{return _executors.find(type)!=_executors.end();}
FleetOrder* FleetCommandSystem::SelectNextPending(FleetCommandComponent& fleet){
    FleetOrder* best=nullptr;for(auto&o:fleet._orders){if(o.state!=FleetOrderState::Pending)continue;if(!best||o.priority>best->priority||(o.priority==best->priority&&o.sequence<best->sequence))best=&o;}return best;
}
void FleetCommandSystem::Update(float dt){
    if(!_entityManager||dt<0.0f)return;
    for(auto*fleet:_entityManager->GetAllComponents<FleetCommandComponent>()){
        if(!fleet)continue;
        // One fleet-level order owns the active execution lane; queued orders wait.
        FleetOrder* active=nullptr;for(auto&o:fleet->_orders)if(o.state==FleetOrderState::Active){active=&o;break;}
        if(!active){active=SelectNextPending(*fleet);if(active){active->state=FleetOrderState::Active;active->status="DISPATCHING";}}
        if(!active)continue;
        const auto it=_executors.find(active->type);
        if(it==_executors.end()){
            active->state=FleetOrderState::Blocked;
            active->status="WAITING FOR DOMAIN EXECUTOR: "+FleetOrder::GetOrderTypeName(active->type);
            continue;
        }
        const auto result=it->second(*fleet,*active,dt);
        if(result.progress>=0.0f)active->progress=std::clamp(result.progress,0.0f,1.0f);
        if(!result.status.empty())active->status=result.status;
        switch(result.disposition){
            case FleetOrderExecutionDisposition::Running:break;
            case FleetOrderExecutionDisposition::Completed:active->state=FleetOrderState::Completed;active->progress=1.0f;if(active->status.empty())active->status="COMPLETED";break;
            case FleetOrderExecutionDisposition::Failed:active->state=FleetOrderState::Failed;if(active->status.empty())active->status="FAILED";break;
            case FleetOrderExecutionDisposition::Blocked:active->state=FleetOrderState::Blocked;if(active->status.empty())active->status="BLOCKED";break;
        }
    }
}

} // namespace subspace
