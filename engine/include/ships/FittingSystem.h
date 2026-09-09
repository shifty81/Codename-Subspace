#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class EquipmentCategory { Kinetic, Energy, Missile, PointDefense, Mining, Salvage, Tractor, CombatDrone, MiningDrone, SalvageDrone, RepairDrone, CargoDrone, EWar, FleetSupport, Shield, Armor, Sensor };
enum class FittingHardpointSize { Utility, Small, Medium, Large, Capital };

struct EquipmentDefinitionNative { std::string id; std::string name; EquipmentCategory category=EquipmentCategory::Kinetic; FittingHardpointSize size=FittingHardpointSize::Small; double power=0; double heat=0; double mass=0; double range=0; double tracking=0; };
struct FittingCapacity { double power=100; double heat=100; double mass=1000; int droneBandwidth=5; std::unordered_map<int,int> hardpoints; };
struct FittingResult { bool installed=false; std::string reason; };

class FittingSystem {
public:
    FittingSystem();
    const EquipmentDefinitionNative* Get(const std::string& id) const;
    std::vector<EquipmentDefinitionNative> Catalog() const;
    FittingResult CanInstall(const FittingCapacity& capacity,const std::vector<std::string>& fitted,const std::string& equipmentId) const;
private:
    std::unordered_map<std::string,EquipmentDefinitionNative> equipment_;
};

} // namespace subspace
