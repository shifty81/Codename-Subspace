#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
namespace subspace {
enum class AnomalousSpaceType { SubspaceTear, TemporalPocket, GraviticRift, IonMaw, AncientTransit }; 
struct AnomalousSpacePocket { std::uint64_t id=0; AnomalousSpaceType type=AnomalousSpaceType::SubspaceTear; std::string destination; double stability=1; double remainingHours=24; double massLimit=10000; double massUsed=0; double hazard=0.2; bool collapsed=false; };
class AnomalousSpaceSystem {
public:
 std::uint64_t Open(AnomalousSpaceType type,const std::string& destination,double stability,double hours,double massLimit,double hazard);
 bool Traverse(std::uint64_t id,double shipMass);
 void Tick(double hours);
 const AnomalousSpacePocket* Get(std::uint64_t id) const;
private:std::uint64_t nextId_=1;std::unordered_map<std::uint64_t,AnomalousSpacePocket> pockets_;
};
}
