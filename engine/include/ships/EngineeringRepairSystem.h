#pragma once
#include <string>
#include <unordered_map>
namespace subspace {
struct RepairResourcePool { double spareParts=0; double repairGel=0; int droneKits=0; };
struct EngineeringRepairJob { std::string id; std::string subsystemId; double remainingIntegrity=0; double integrityPerSecond=1; double partsPerIntegrity=0.1; bool active=false; bool complete=false; };
class EngineeringRepairSystem {
public:
 void SetResources(const RepairResourcePool& resources){resources_=resources;}
 const RepairResourcePool& Resources() const{return resources_;}
 bool Start(const EngineeringRepairJob& job);
 double Advance(const std::string& jobId,double seconds);
 bool Cancel(const std::string& jobId);
 const EngineeringRepairJob* Get(const std::string& jobId) const;
private: RepairResourcePool resources_;std::unordered_map<std::string,EngineeringRepairJob> jobs_;
};
}
