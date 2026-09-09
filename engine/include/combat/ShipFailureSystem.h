#pragma once
#include <string>
#include <unordered_map>
#include <vector>
namespace subspace {
enum class ShipSubsystemType { Reactor, Thruster, Sensor, Weapon, Shield, Cargo, LifeSupport, DroneBay, Fabricator };
enum class FailureState { Nominal, Degraded, Failing, Disabled, Destroyed };
struct ShipSubsystemState { std::string id; ShipSubsystemType type=ShipSubsystemType::Thruster; double integrity=100; double maxIntegrity=100; FailureState state=FailureState::Nominal; bool critical=false; };
class ShipFailureSystem {
public:
 bool Register(const ShipSubsystemState& subsystem);
 bool ApplyDamage(const std::string& id,double damage);
 bool Restore(const std::string& id,double integrity);
 double OperationalFraction(ShipSubsystemType type) const;
 std::vector<std::string> CriticalFailures() const;
 const ShipSubsystemState* Get(const std::string& id) const;
private: static FailureState Resolve(double ratio);std::unordered_map<std::string,ShipSubsystemState> systems_;
};
}
