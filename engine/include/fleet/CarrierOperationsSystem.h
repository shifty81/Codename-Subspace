#pragma once
#include <string>
#include <unordered_map>
namespace subspace {
enum class NestedCraftRole { Fighter, Interceptor, Shuttle, MiningCraft, SalvageCraft, Scout, Utility };
enum class NestedCraftState { Stored, Launching, Deployed, Recovering, Repairing, Lost };
struct NestedCraft { std::string id; NestedCraftRole role=NestedCraftRole::Fighter; NestedCraftState state=NestedCraftState::Stored; double hull=100; double ammo=100; double fuel=100; std::string pilotId; };
class CarrierOperationsSystem {
public:
 explicit CarrierOperationsSystem(int capacity=4):capacity_(capacity){}
 bool Register(const NestedCraft& craft);
 bool Launch(const std::string& id);
 bool Recover(const std::string& id);
 bool Service(const std::string& id,double hull,double ammo,double fuel);
 int DeployedCount() const;
 const NestedCraft* Get(const std::string& id) const;
private:int capacity_=4;std::unordered_map<std::string,NestedCraft> craft_;
};
}
