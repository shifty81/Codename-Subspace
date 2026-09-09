#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
namespace subspace {
enum class CapitalHullType { Battleship, Carrier, Dreadnought, IndustrialCapital, FactorySupport, MobileShipyard };
enum class CapitalBuildStage { Frame, PowerCore, Propulsion, Systems, Fitting, Commissioning, Complete };
struct CapitalBuildJob { std::uint64_t id=0; CapitalHullType hull=CapitalHullType::Battleship; CapitalBuildStage stage=CapitalBuildStage::Frame; std::unordered_map<std::string,double> required; std::unordered_map<std::string,double> delivered; double workRemaining=100; double totalWork=100; bool complete=false; };
class CapitalConstructionSystem {
public:
 std::uint64_t Start(CapitalHullType hull,const std::unordered_map<std::string,double>& materials,double work);
 double Deliver(std::uint64_t id,const std::string& material,double amount);
 double Advance(std::uint64_t id,double work);
 bool MaterialsReady(std::uint64_t id) const;
 const CapitalBuildJob* Get(std::uint64_t id) const;
private:std::uint64_t nextId_=1;std::unordered_map<std::uint64_t,CapitalBuildJob> jobs_;
};
}
