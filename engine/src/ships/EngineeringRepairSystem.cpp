#include "ships/EngineeringRepairSystem.h"
#include <algorithm>
namespace subspace {
bool EngineeringRepairSystem::Start(const EngineeringRepairJob& job){if(job.id.empty()||job.subsystemId.empty()||job.remainingIntegrity<=0||job.integrityPerSecond<=0||job.partsPerIntegrity<0)return false;auto j=job;j.active=true;j.complete=false;jobs_[j.id]=j;return true;}
double EngineeringRepairSystem::Advance(const std::string& jobId,double seconds){auto it=jobs_.find(jobId);if(it==jobs_.end()||!it->second.active||seconds<=0)return 0;auto& j=it->second;double requested=std::min(j.remainingIntegrity,j.integrityPerSecond*seconds);double affordable=j.partsPerIntegrity<=0?requested:resources_.spareParts/j.partsPerIntegrity;double done=std::min(requested,affordable);if(done<=0)return 0;resources_.spareParts=std::max(0.0,resources_.spareParts-done*j.partsPerIntegrity);j.remainingIntegrity-=done;if(j.remainingIntegrity<=1e-9){j.remainingIntegrity=0;j.complete=true;j.active=false;}return done;}
bool EngineeringRepairSystem::Cancel(const std::string& jobId){auto it=jobs_.find(jobId);if(it==jobs_.end()||!it->second.active)return false;it->second.active=false;return true;}
const EngineeringRepairJob* EngineeringRepairSystem::Get(const std::string& jobId) const {auto it=jobs_.find(jobId);return it==jobs_.end()?nullptr:&it->second;}
}
