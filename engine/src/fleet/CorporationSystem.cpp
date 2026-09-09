#include "fleet/CorporationSystem.h"

#include <algorithm>
namespace subspace {
std::uint64_t CorporationSystem::Hire(const std::string&n,CrewSpecialty s,int skill,double salary){if(n.empty())return 0;auto id=nextId_++;members_[id]={id,n,s,std::clamp(skill,1,100),0,std::max(0.0,salary),0};return id;}
bool CorporationSystem::SetPermission(std::uint64_t id,CorporationPermission p,bool enabled){auto it=members_.find(id);if(it==members_.end())return false;auto bit=static_cast<std::uint64_t>(p);if(enabled)it->second.permissions|=bit;else it->second.permissions&=~bit;return true;}
bool CorporationSystem::HasPermission(std::uint64_t id,CorporationPermission p)const{auto it=members_.find(id);return it!=members_.end()&&(it->second.permissions&static_cast<std::uint64_t>(p))!=0;}
bool CorporationSystem::AssignShip(std::uint64_t id,std::uint64_t ship){auto it=members_.find(id);if(it==members_.end())return false;it->second.assignedShip=ship;return true;}
const CorporationMemberNative* CorporationSystem::Get(std::uint64_t id)const{auto it=members_.find(id);return it==members_.end()?nullptr:&it->second;}
double CorporationSystem::Payroll()const{double p=0;for(const auto&kv:members_)p+=kv.second.salary;return p;}
} // namespace subspace
