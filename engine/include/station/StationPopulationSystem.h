#pragma once
#include <string>
#include <unordered_map>
namespace subspace {
struct StationPopulationState { std::string stationId; double population=0; double capacity=0; double jobs=0; double food=0; double supplies=0; double happiness=0.7; double serviceCapacity=0; };
class StationPopulationSystem {
public:
 void Advance(StationPopulationState& state,double hours) const;
 double Workforce(const StationPopulationState& state) const;
 double ConsumptionRate(const StationPopulationState& state) const;
};
}
