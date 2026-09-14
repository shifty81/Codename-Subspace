#include "ship_editor/ShipyardJobSystem.h"

#include <algorithm>

namespace subspace {

ShipyardJobRecord* ShipyardJobSystem::FindMutable(std::uint64_t sequence){for(auto& j:jobs_)if(j.sequence==sequence)return &j;return nullptr;}
std::uint64_t ShipyardJobSystem::Queue(ShipyardJobDescriptor d){std::lock_guard<std::mutex> l(mutex_);const auto n=nextSequence_++;jobs_.push_back({n,std::move(d),ShipyardJobStatus::Queued,0,0.0f,"Queued",{}});return n;}
bool ShipyardJobSystem::Start(std::uint64_t n,std::string s){std::lock_guard<std::mutex> l(mutex_);auto* j=FindMutable(n);if(!j||j->status!=ShipyardJobStatus::Queued)return false;j->status=ShipyardJobStatus::Running;j->statusText=s.empty()?"Running":std::move(s);return true;}
bool ShipyardJobSystem::SetProgress(std::uint64_t n,float p,std::string s){std::lock_guard<std::mutex> l(mutex_);auto* j=FindMutable(n);if(!j||j->status!=ShipyardJobStatus::Running)return false;j->progress=std::clamp(p,0.0f,1.0f);if(!s.empty())j->statusText=std::move(s);return true;}
bool ShipyardJobSystem::AppendOutput(std::uint64_t n,std::string line){std::lock_guard<std::mutex> l(mutex_);auto* j=FindMutable(n);if(!j)return false;constexpr std::size_t maxLines=2048;if(j->output.size()>=maxLines)j->output.erase(j->output.begin(),j->output.begin()+256);j->output.push_back(std::move(line));return true;}
bool ShipyardJobSystem::Complete(std::uint64_t n,int code,std::string s){std::lock_guard<std::mutex> l(mutex_);auto* j=FindMutable(n);if(!j||(j->status!=ShipyardJobStatus::Running&&j->status!=ShipyardJobStatus::Queued))return false;j->exitCode=code;j->progress=1.0f;j->status=code==0?ShipyardJobStatus::Succeeded:ShipyardJobStatus::Failed;j->statusText=s.empty()?(code==0?"Succeeded":"Failed"):std::move(s);return true;}
bool ShipyardJobSystem::Cancel(std::uint64_t n,std::string s){std::lock_guard<std::mutex> l(mutex_);auto* j=FindMutable(n);if(!j||!j->descriptor.cancellable||j->status==ShipyardJobStatus::Succeeded||j->status==ShipyardJobStatus::Failed||j->status==ShipyardJobStatus::Cancelled)return false;j->status=ShipyardJobStatus::Cancelled;j->statusText=s.empty()?"Cancelled":std::move(s);return true;}
std::optional<ShipyardJobRecord> ShipyardJobSystem::Find(std::uint64_t n)const{std::lock_guard<std::mutex> l(mutex_);for(const auto& j:jobs_)if(j.sequence==n)return j;return std::nullopt;}
std::vector<ShipyardJobRecord> ShipyardJobSystem::Snapshot()const{std::lock_guard<std::mutex> l(mutex_);return jobs_;}
std::vector<ShipyardJobRecord> ShipyardJobSystem::Active()const{std::lock_guard<std::mutex> l(mutex_);std::vector<ShipyardJobRecord> out;for(const auto& j:jobs_)if(j.status==ShipyardJobStatus::Queued||j.status==ShipyardJobStatus::Running)out.push_back(j);return out;}

} // namespace subspace
