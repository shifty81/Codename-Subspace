#include "runtime/PersistentUniverseSystem.h"
#include <algorithm>
#include <sstream>
namespace subspace {
std::uint64_t PersistentUniverseSystem::Record(PersistentEventType t,const std::string& r,const std::string& e,double v){if(r.empty()||e.empty())return 0;auto seq=next_++;events_.push_back({seq,t,r,e,v});return seq;}
double PersistentUniverseSystem::ResourceRemaining(const std::string& entity,double initial) const {double remaining=initial;for(auto& e:events_)if(e.entityId==entity&&e.type==PersistentEventType::ResourceDepleted)remaining-=e.value;return std::max(0.0,remaining);}
bool PersistentUniverseSystem::IsDestroyed(const std::string& entity) const {for(auto it=events_.rbegin();it!=events_.rend();++it)if(it->entityId==entity){if(it->type==PersistentEventType::ShipDestroyed)return true;if(it->type==PersistentEventType::StationRepaired)return false;}return false;}
std::vector<PersistentUniverseEvent> PersistentUniverseSystem::EventsForRegion(const std::string& region) const {std::vector<PersistentUniverseEvent> out;for(auto& e:events_)if(e.regionId==region)out.push_back(e);return out;}
std::string PersistentUniverseSystem::Serialize() const {std::ostringstream out;out<<next_<<"\n";for(auto& e:events_)out<<e.sequence<<"\t"<<static_cast<int>(e.type)<<"\t"<<e.regionId<<"\t"<<e.entityId<<"\t"<<e.value<<"\n";return out.str();}
bool PersistentUniverseSystem::Deserialize(const std::string& text){std::istringstream in(text);std::uint64_t next=0;if(!(in>>next))return false;std::string rest;std::getline(in,rest);std::vector<PersistentUniverseEvent> parsed;std::string line;while(std::getline(in,line)){if(line.empty())continue;std::istringstream row(line);PersistentUniverseEvent e;int type=0;if(!(row>>e.sequence))return false;if(row.get()!='\t')return false;if(!(row>>type))return false;if(row.get()!='\t')return false;if(!std::getline(row,e.regionId,'\t'))return false;if(!std::getline(row,e.entityId,'\t'))return false;if(!(row>>e.value))return false;e.type=static_cast<PersistentEventType>(type);parsed.push_back(e);}events_=std::move(parsed);next_=next;return true;}
}
