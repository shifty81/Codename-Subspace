#pragma once
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
enum class DroneRole { Combat, Mining, Salvage, Cargo, Repair, Scout, PointDefense, Construction };
enum class DroneState { Stored, Launching, Active, Returning, Lost };
struct DroneUnit { std::string id; DroneRole role=DroneRole::Combat; DroneState state=DroneState::Stored; int bandwidth=1; double cargo=0; double cargoCapacity=0; std::string targetId; double taskProgress=0; };
class DroneOperationsSystem {
public:
 explicit DroneOperationsSystem(int bandwidth=5):bandwidthLimit_(bandwidth){}
 bool Register(const DroneUnit& drone);
 bool Launch(const std::string& id);
 bool Assign(const std::string& id,const std::string& target);
 bool Recall(const std::string& id);
 void Tick(double seconds);
 int BandwidthUsed() const;
 double CollectCargo(const std::string& id);
 const DroneUnit* Get(const std::string& id) const;
private:int bandwidthLimit_=5;std::unordered_map<std::string,DroneUnit> drones_;
};
}
