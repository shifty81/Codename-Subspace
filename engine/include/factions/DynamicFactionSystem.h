#pragma once
#include <string>
#include <unordered_map>
namespace subspace {
enum class FactionStrategicAction { Mine, Trade, Patrol, Raid, Expand, Fortify, Research, Recover };
struct DynamicFactionState { std::string id; double resources=100; double military=50; double industry=50; double expansion=0.5; double aggression=0.3; double stability=0.8; int stations=1; int fleets=1; };
class DynamicFactionSystem {
public:
 bool Register(const DynamicFactionState& state);
 FactionStrategicAction ChooseAction(const std::string& id,double regionalThreat,double scarcity) const;
 void Advance(const std::string& id,FactionStrategicAction action,double hours);
 const DynamicFactionState* Get(const std::string& id) const;
private:std::unordered_map<std::string,DynamicFactionState> states_;
};
}
