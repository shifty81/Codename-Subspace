#include "integration/CombatShipIntegration.h"

#include <algorithm>
#include <cmath>

namespace subspace {

ShipSubsystemType CombatShipIntegration::SubsystemFor(EquipmentCategory c) {
    switch (c) {
        case EquipmentCategory::Kinetic:
        case EquipmentCategory::Energy:
        case EquipmentCategory::Missile:
        case EquipmentCategory::PointDefense:
        case EquipmentCategory::Mining:
        case EquipmentCategory::Salvage:
        case EquipmentCategory::Tractor:
        case EquipmentCategory::EWar:
        case EquipmentCategory::FleetSupport: return ShipSubsystemType::Weapon;
        case EquipmentCategory::CombatDrone:
        case EquipmentCategory::MiningDrone:
        case EquipmentCategory::SalvageDrone:
        case EquipmentCategory::RepairDrone:
        case EquipmentCategory::CargoDrone: return ShipSubsystemType::DroneBay;
        case EquipmentCategory::Shield:
        case EquipmentCategory::Armor: return ShipSubsystemType::Shield;
        case EquipmentCategory::Sensor: return ShipSubsystemType::Sensor;
    }
    return ShipSubsystemType::Weapon;
}

bool CombatShipIntegration::IsWeapon(EquipmentCategory c) {
    return c==EquipmentCategory::Kinetic || c==EquipmentCategory::Energy ||
           c==EquipmentCategory::Missile || c==EquipmentCategory::PointDefense ||
           c==EquipmentCategory::Mining || c==EquipmentCategory::Salvage ||
           c==EquipmentCategory::Tractor;
}

bool CombatShipIntegration::IsDroneBay(EquipmentCategory c) {
    return c==EquipmentCategory::CombatDrone || c==EquipmentCategory::MiningDrone ||
           c==EquipmentCategory::SalvageDrone || c==EquipmentCategory::RepairDrone ||
           c==EquipmentCategory::CargoDrone;
}

DroneRole CombatShipIntegration::DroneRoleFor(EquipmentCategory c) {
    switch(c) {
        case EquipmentCategory::MiningDrone:return DroneRole::Mining;
        case EquipmentCategory::SalvageDrone:return DroneRole::Salvage;
        case EquipmentCategory::RepairDrone:return DroneRole::Repair;
        case EquipmentCategory::CargoDrone:return DroneRole::Cargo;
        default:return DroneRole::Combat;
    }
}

CombatWeaponSpec CombatShipIntegration::WeaponSpecFor(const EquipmentDefinitionNative& e,
                                                       const std::string& runtimeId) {
    CombatWeaponSpec s;
    s.id=runtimeId;
    if(e.id.find("rail")!=std::string::npos)s.family=AdvancedWeaponFamily::Rail;
    else if(e.id.find("beam")!=std::string::npos)s.family=AdvancedWeaponFamily::Beam;
    else if(e.id.find("torpedo")!=std::string::npos)s.family=AdvancedWeaponFamily::Torpedo;
    else if(e.category==EquipmentCategory::Missile)s.family=AdvancedWeaponFamily::Missile;
    else if(e.category==EquipmentCategory::PointDefense)s.family=AdvancedWeaponFamily::PointDefense;
    else s.family=AdvancedWeaponFamily::Kinetic;
    s.damage=std::max(1.0, e.mass*0.35 + e.power*0.65);
    s.heatPerShot=std::max(0.25,e.heat*0.18);
    s.powerPerShot=std::max(0.5,e.power*0.30);
    s.cooldownSeconds=e.tracking>0?std::clamp(1.0/e.tracking,0.08,5.0):1.0;
    s.ammoPerShot=(e.category==EquipmentCategory::Energy || e.category==EquipmentCategory::Mining || e.category==EquipmentCategory::Salvage || e.category==EquipmentCategory::Tractor)?0:1;
    s.magazineCapacity=s.ammoPerShot==0?0:std::max(4,static_cast<int>(80.0/(1.0+e.mass*0.04)));
    return s;
}

std::string CombatShipIntegration::AmmoCommodityFor(const EquipmentDefinitionNative& e) {
    if(e.category==EquipmentCategory::Missile)return e.id.find("torpedo")!=std::string::npos?"torpedo_round":"guided_missile";
    if(e.category==EquipmentCategory::Kinetic)return "kinetic_round";
    if(e.category==EquipmentCategory::PointDefense)return "pd_round";
    return {};
}

bool CombatShipIntegration::Install(ProductionCombatShip& ship,const std::string& equipmentId,
                                    const std::string& instanceId,const FittingSystem& fitting,
                                    AdvancedCombatSystem& combat,ShipFailureSystem& failures,
                                    DroneOperationsSystem& drones) const {
    if(instanceId.empty()||ship.installed.count(instanceId))return false;
    const auto* e=fitting.Get(equipmentId); if(!e)return false;
    auto fit=fitting.CanInstall(ship.capacity,ship.fittedEquipment,equipmentId);if(!fit.installed)return false;
    InstalledEquipmentRuntime link;link.instanceId=instanceId;link.equipmentId=equipmentId;link.subsystemId="fit:"+instanceId;link.weapon=IsWeapon(e->category);link.droneBay=IsDroneBay(e->category);link.ammoCommodity=AmmoCommodityFor(*e);
    ShipSubsystemState ss;ss.id=link.subsystemId;ss.type=SubsystemFor(e->category);ss.maxIntegrity=std::max(25.0,50.0+e->mass);ss.integrity=ss.maxIntegrity;ss.critical=e->category==EquipmentCategory::Sensor||e->category==EquipmentCategory::Shield;
    if(!failures.Register(ss))return false;
    if(link.weapon){link.weaponRuntimeId="weapon:"+instanceId;auto spec=WeaponSpecFor(*e,link.weaponRuntimeId);if(!combat.Register(spec,spec.magazineCapacity))return false;}
    if(link.droneBay){DroneUnit d;d.id="drone:"+instanceId;d.role=DroneRoleFor(e->category);d.bandwidth=1;d.cargoCapacity=(d.role==DroneRole::Mining||d.role==DroneRole::Salvage||d.role==DroneRole::Cargo)?20.0:0.0;if(!drones.Register(d))return false;}
    ship.fittedEquipment.push_back(equipmentId);ship.installed[instanceId]=link;return true;
}

CombatFireResult CombatShipIntegration::Fire(ProductionCombatShip& ship,const std::string& instanceId,
                                             AdvancedCombatSystem& combat,const ShipFailureSystem& failures) const {
    CombatFireResult out;auto it=ship.installed.find(instanceId);if(it==ship.installed.end()||!it->second.weapon){out.reason="not installed weapon";return out;}
    const auto* subsystem=failures.Get(it->second.subsystemId);if(!subsystem||subsystem->state==FailureState::Disabled||subsystem->state==FailureState::Destroyed){out.reason="weapon subsystem disabled";return out;}
    out=combat.Fire(it->second.weaponRuntimeId,ship.availablePower);if(out.fired){ship.availablePower=std::max(0.0,ship.availablePower-out.powerConsumed);ship.generatedHeat+=out.heatGenerated;}return out;
}

bool CombatShipIntegration::ApplyModuleDamage(const ProductionCombatShip& ship,const std::string& instanceId,double damage,ShipFailureSystem& failures) const {auto it=ship.installed.find(instanceId);return it!=ship.installed.end()&&failures.ApplyDamage(it->second.subsystemId,damage);}

int CombatShipIntegration::ReloadFromInventory(const ProductionCombatShip& ship,const std::string& instanceId,int requested,std::unordered_map<std::string,std::uint64_t>& inventory,AdvancedCombatSystem& combat) const {
    if(requested<=0)return 0;auto it=ship.installed.find(instanceId);if(it==ship.installed.end()||it->second.ammoCommodity.empty())return 0;auto inv=inventory.find(it->second.ammoCommodity);if(inv==inventory.end()||inv->second==0)return 0;const auto* w=combat.Get(it->second.weaponRuntimeId);if(!w)return 0;int room=std::max(0,w->spec.magazineCapacity-w->ammo);int rounds=std::min({requested,room,static_cast<int>(inv->second)});if(rounds<=0)return 0;if(!combat.Reload(it->second.weaponRuntimeId,rounds))return 0;inv->second-=static_cast<std::uint64_t>(rounds);return rounds;
}

bool CombatShipIntegration::BeginRepair(const ProductionCombatShip& ship,const std::string& instanceId,const ShipFailureSystem& failures,EngineeringRepairSystem& repairs,double rate) const {
    auto it=ship.installed.find(instanceId);if(it==ship.installed.end())return false;const auto* ss=failures.Get(it->second.subsystemId);if(!ss||ss->integrity>=ss->maxIntegrity)return false;EngineeringRepairJob j;j.id="repair:"+instanceId;j.subsystemId=ss->id;j.remainingIntegrity=ss->maxIntegrity-ss->integrity;j.integrityPerSecond=std::max(0.1,rate);j.partsPerIntegrity=0.08;return repairs.Start(j);
}

double CombatShipIntegration::AdvanceRepair(const ProductionCombatShip& ship,const std::string& instanceId,double seconds,EngineeringRepairSystem& repairs,ShipFailureSystem& failures) const {auto it=ship.installed.find(instanceId);if(it==ship.installed.end())return 0;double restored=repairs.Advance("repair:"+instanceId,seconds);if(restored>0)failures.Restore(it->second.subsystemId,restored);return restored;}

SensorIntegrationReport CombatShipIntegration::EvaluateSensors(const ProductionCombatShip& ship,const ElectronicWarfareSystem& ewar,const EnvironmentalExposureReport& environment,const ShipFailureSystem& failures) const {
    SensorIntegrationReport r;double physical=failures.OperationalFraction(ShipSubsystemType::Sensor);double env=std::clamp(1.0-environment.sensorPenalty,0.0,1.0);r.sensorMultiplier=physical*env*ewar.CapabilityMultiplier(EWarEffectType::SensorJam);r.missileMultiplier=ewar.CapabilityMultiplier(EWarEffectType::MissileDisrupt);r.propulsionMultiplier=std::clamp((1.0-environment.propulsionPenalty)*ewar.CapabilityMultiplier(EWarEffectType::PropulsionSuppress),0.0,1.0);r.droneMultiplier=ewar.CapabilityMultiplier(EWarEffectType::DroneDisrupt);r.range=ship.baseSensorRange*r.sensorMultiplier;return r;
}

} // namespace subspace
